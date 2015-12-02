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
 
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

typedef struct {
    uint8_t adv_frame [BLE_GAP_ADV_MAX_SIZE];
    uint8_t adv_len;
} eddystone_frame_t;

typedef struct {
    uint8_t  flags_len;      // Length: Flags. CSS v5, Part A, 1.3
    uint8_t  flags_type;     // Flags data type value
    uint8_t  flags_data;     // Flags data
    uint8_t  svc_uuid_len;   // Length: Complete list of 16-bit Service UUIDs.
    uint8_t  svc_uuid_type;  // Complete list of 16-bit Service UUIDs data type value
    uint16_t svc_uuid_list;  // 16-bit Eddystone UUID
    uint8_t  svc_data_len;   // Length: Service Data.
    uint8_t  svc_data_type;  // Service Data data type value
    uint16_t svc_data_uuid;  // 16-bit Eddystone UUID
    uint8_t  frame_type;     // eddystone frame type
} __attribute__ ((packed)) eddystone_header_t;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static eddystone_frame_t eddystone_frames [3];

static uint32_t adv_cnt = 0;
static uint32_t sec_cnt = 0;

static const eddystone_header_t  header = {
    .flags_len     = 0x02,
    .flags_type    = 0x01,
    .flags_data    = 0x06,
    .svc_uuid_len  = 0x03,
    .svc_uuid_type = 0x03,
    .svc_uuid_list = 0xFEAA,  // 0xAAFE  big-endian flip
    .svc_data_len  = 0x03,
    .svc_data_type = 0x16,
    .svc_data_uuid = 0xFEAA,  // 0xAAFE  big-endian flip
    .frame_type    = 0x00,
};

#define SVC_DATA_LEN_OFFSET  (offsetof(eddystone_header_t, svc_data_len) + 1)

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_header(uint8_t * data, uint8_t frame_type, uint8_t * len) 
{
    if ((*len) + sizeof(eddystone_header_t) > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    memcpy(data, &header, sizeof(header));

    *len = sizeof(eddystone_header_t);
    
    ((eddystone_header_t*)data)->frame_type = frame_type;

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_uint32(uint8_t * data, uint8_t * len, uint32_t val)
{
    if ((*len) + sizeof(uint32_t) > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    data[(*len)++] = (uint8_t) (val >> 24);
    data[(*len)++] = (uint8_t) (val >> 16);
    data[(*len)++] = (uint8_t) (val >>  8);
    data[(*len)++] = (uint8_t) (val >>  0);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_uint16(uint8_t * data, uint8_t * len, uint16_t val)
{
    if ((*len) + sizeof(uint16_t) > BLE_GAP_ADV_MAX_SIZE) {
        return NRF_ERROR_DATA_SIZE;
    }

    data[(*len)++] = (uint8_t) (val >> 8);
    data[(*len)++] = (uint8_t) (val >> 0);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void eddystone_set_adv_data(uint32_t frame_index)
{
    uint32_t err_code;

    uint8_t * encoded_advdata = eddystone_frames[frame_index].adv_frame;
    uint8_t   len_advdata     = eddystone_frames[frame_index].adv_len; 

    err_code = sd_ble_gap_adv_data_set(encoded_advdata, len_advdata, NULL, 0);
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

    eddystone_header(encoded_advdata, EDDYSTONE_TLM_TYPE, len_advdata);

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

    /* Update Service Data Length. */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - SVC_DATA_LEN_OFFSET;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_url_frame_buffer(void)
{
    uint8_t * encoded_advdata =  eddystone_frames[EDDYSTONE_URL].adv_frame;
    uint8_t * len_advdata     = &eddystone_frames[EDDYSTONE_URL].adv_len;

    *len_advdata = 0;

    eddystone_header(encoded_advdata, EDDYSTONE_URL_TYPE, len_advdata);

    encoded_advdata[(*len_advdata)++] = APP_MEASURED_RSSI;
    encoded_advdata[(*len_advdata)++] = URL_PREFIX__http;

    /* Set URL string */
    memcpy(&encoded_advdata[(*len_advdata)], URL_STRING, URL_LENGTH);
    *len_advdata += URL_LENGTH;

    /* Update Service Data Length. */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - SVC_DATA_LEN_OFFSET;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_uid_frame_buffer(void)
{
    uint8_t * encoded_advdata =  eddystone_frames[EDDYSTONE_UID].adv_frame;
    uint8_t * len_advdata     = &eddystone_frames[EDDYSTONE_UID].adv_len;

    *len_advdata = 0;

    eddystone_header(encoded_advdata, EDDYSTONE_UID_TYPE, len_advdata);

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

    /* Update Service Data Length. */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - SVC_DATA_LEN_OFFSET;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void eddystone_init(void)
{
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
