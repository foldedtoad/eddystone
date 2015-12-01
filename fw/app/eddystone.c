/*---------------------------------------------------------------------------*/
/*  eddystone.c                                                              */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "ble_gap.h"
#include "nrf_error.h"

#include "config.h"
#include "eddystone.h"
#include "battery.h"
#include "temperature.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define FICR_DEVICEADDR          ((uint8_t*) &NRF_FICR->DEVICEADDR[0])

#define EDDYSTONE_UID_TYPE       0x00
#define EDDYSTONE_URL_TYPE       0x10
#define EDDYSTONE_TLM_TYPE       0x20

#define SERVICE_DATA_OFFSET      0x07

#define URL_PREFIX__http_www     0x00
#define URL_PREFIX__https_www    0x01
#define URL_PREFIX__http         0x02
#define URL_PREFIX__https        0x03

#define TLM_VERSION              0x00
 
typedef struct {
    uint8_t adv_frame [BLE_GAP_ADV_MAX_SIZE];
    uint8_t adv_len;
} eddystone_frame_t;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static eddystone_frame_t eddystone_frames [3];

static uint32_t adv_cnt = 0;
static uint32_t sec_cnt = 0;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_head_encode(uint8_t * p_encoded_data,
                                      uint8_t   frame_type,
                                      uint8_t * p_len) 
{
    // Check for buffer overflow.
    if ((*p_len) + 12 > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    (*p_len) = 0;
    p_encoded_data[(*p_len)++] = 0x02; // Length; Flags. CSS v5, Part A, § 1.3
    p_encoded_data[(*p_len)++] = 0x01; // Flags data type value
    p_encoded_data[(*p_len)++] = 0x06; // Flags data
    p_encoded_data[(*p_len)++] = 0x03; // Length; Complete list of 16-bit Service UUIDs. Ibid. § 1.1
    p_encoded_data[(*p_len)++] = 0x03; // Complete list of 16-bit Service UUIDs data type value
    p_encoded_data[(*p_len)++] = 0xAA; // 16-bit Eddystone UUID
    p_encoded_data[(*p_len)++] = 0xFE;
    p_encoded_data[(*p_len)++] = 0x03; // Length; Service Data. Ibid. § 1.11
    p_encoded_data[(*p_len)++] = 0x16; // Service Data data type value
    p_encoded_data[(*p_len)++] = 0xAA; // 16-bit Eddystone UUID
    p_encoded_data[(*p_len)++] = 0xFE;
    p_encoded_data[(*p_len)++] = frame_type;

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_uint32(uint8_t * p_encoded_data,
                                 uint8_t * p_len,
                                 uint32_t  val)
{
    if ((*p_len) + 4 > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 24u);
    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 16u);
    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 8u);
    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 0u);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_uint16(uint8_t * p_encoded_data,
                                 uint8_t * p_len,
                                 uint16_t  val)
{
    if ((*p_len) + 2 > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 8u);
    p_encoded_data[(*p_len)++] = (uint8_t) (val >> 0u);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void eddystone_set_adv_data(uint32_t frame_index)
{
    uint32_t err_code;

    uint8_t * p_encoded_advdata = eddystone_frames[frame_index].adv_frame;
    uint8_t   len_advdata       = eddystone_frames[frame_index].adv_len; 

    err_code = sd_ble_gap_adv_data_set(p_encoded_advdata, 
                                       len_advdata,
                                       NULL, 0);
    APP_ERROR_CHECK( err_code );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_tlm_frame_buffer(void)
{
    uint8_t * encoded_advdata =  eddystone_frames[EDDYSTONE_TLM].adv_frame;
    uint8_t * len_advdata     = &eddystone_frames[EDDYSTONE_TLM].adv_len;

    *len_advdata = 0;

    eddystone_head_encode(encoded_advdata, EDDYSTONE_TLM_TYPE, len_advdata);

    encoded_advdata[(*len_advdata)++] = TLM_VERSION;

    /* Battery voltage, 1 mV/bit */
    eddystone_uint16(encoded_advdata, len_advdata, battery_level_get());

    /* Beacon temperature */
    eddystone_uint16(encoded_advdata, len_advdata, temperature_data_get());

    /* Advertising PDU count */
    eddystone_uint32(encoded_advdata, len_advdata, adv_cnt);

    /* Time since power-on or reboot */
    eddystone_uint32(encoded_advdata, len_advdata, sec_cnt);

    /* RFU field must be 0x00 */
    encoded_advdata[(*len_advdata)++] = 0x00;
    encoded_advdata[(*len_advdata)++] = 0x00;

    /* Length   Service Data. Ibid. § 1.11 */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - 8;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_url_frame_buffer(void)
{
    uint8_t * encoded_advdata =  eddystone_frames[EDDYSTONE_URL].adv_frame;
    uint8_t * len_advdata     = &eddystone_frames[EDDYSTONE_URL].adv_len;

    *len_advdata = 0;

    eddystone_head_encode(encoded_advdata, EDDYSTONE_URL_TYPE, len_advdata);

    encoded_advdata[(*len_advdata)++] = APP_MEASURED_RSSI;
    encoded_advdata[(*len_advdata)++] = URL_PREFIX__http;

    /* set URL string */
    memcpy(&encoded_advdata[(*len_advdata)], URL_STRING, URL_LENGTH);
    *len_advdata += URL_LENGTH;

    /* Length   Service Data. Ibid. § 1.11 */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - 8;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_uid_frame_buffer(void)
{
    uint8_t * encoded_advdata =  eddystone_frames[EDDYSTONE_UID].adv_frame;
    uint8_t * len_advdata     = &eddystone_frames[EDDYSTONE_UID].adv_len;

    *len_advdata = 0;

    eddystone_head_encode(encoded_advdata, EDDYSTONE_UID_TYPE, len_advdata);

    encoded_advdata[(*len_advdata)++] = APP_MEASURED_RSSI;

    /* Set Namespace */
    static const uint8_t namespace[] = UID_NAMESPACE;
    memcpy(&encoded_advdata[(*len_advdata)], &namespace, sizeof(namespace));
    *len_advdata += sizeof(namespace);

    /* Set Beacon Id (BID) to FICR Device Address */
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[5];
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[4];
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[3];
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[2];
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[1];
    encoded_advdata[(*len_advdata)++] = FICR_DEVICEADDR[0];

    /* RFU field must be 0x00 */
    encoded_advdata[(*len_advdata)++] = 0x00;
    encoded_advdata[(*len_advdata)++] = 0x00;

    /* Length   Service Data. Ibid. § 1.11 */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - 8;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void eddystone_init(void)
{
    PUTS(__func__);

    memset(eddystone_frames, 0, sizeof(eddystone_frames));

    build_uid_frame_buffer();
    build_url_frame_buffer();
    build_tlm_frame_buffer();

    eddystone_set_adv_data(EDDYSTONE_UID);
}

/*---------------------------------------------------------------------------*/
/*  Crappy scheduler -- will re-implement later (sigh)                       */
/*---------------------------------------------------------------------------*/
void eddystone_scheduler(bool radio_is_active)
{
    static uint32_t iterations = 0;

    if (radio_is_active == false)
        return;

    iterations++;
    sec_cnt++;

    if ((iterations % 9) == 0) {
        build_tlm_frame_buffer();
        eddystone_set_adv_data(EDDYSTONE_TLM);
        adv_cnt++;
        //PUTS("TLM");
    }
    else if ((iterations % 5) == 0) {
        eddystone_set_adv_data(EDDYSTONE_URL);
        adv_cnt++;
        //PUTS("URL");
    }
    else if ((iterations % 3) == 0) {
        eddystone_set_adv_data(EDDYSTONE_UID);
        adv_cnt++;
        //PUTS("UID");
    }
}
