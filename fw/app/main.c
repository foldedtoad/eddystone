//
// Created by Alex Van Boxel on 09/08/15.
// Modified by Robin Callender on 11/25/15.
//
#include <stdbool.h>
#include <stdint.h>
#include <ble_advdata.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "ble_radio_notification.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"

#include "config.h"
#include "eddystone.h"
#include "dbglog.h"

#if defined(PROVISION_DBGLOG)
  #include "uart.h"
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define FICR_DEVICEADDR   ((uint8_t*) &NRF_FICR->DEVICEADDR[0])

#define SERVICE_DATA_OFFSET  0x07

#define URL_PREFIX__http_www     0x00
#define URL_PREFIX__https_www    0x01
#define URL_PREFIX__http         0x02
#define URL_PREFIX__https        0x03

static edstn_frame_t edstn_frames[3];

static uint32_t pdu_count = 0;

/* Parameters to be passed to the stack when starting advertising. */
static ble_gap_adv_params_t m_adv_params;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static uint32_t battery_adc_config = 
    (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos)                           |
    (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
    (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)                      |
    (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos)                     |
    (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);

/*---------------------------------------------------------------------------*/
/*  Battery Measurement parameters                                           */
/*---------------------------------------------------------------------------*/

/*  
 *  Reference voltage (in milli volts) used by ADC while doing conversion.
 */
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200

/* 
 *  The ADC is configured to use VDD with 1/3 prescaling as input. 
 *  And hence the result of conversion is to be multiplied by 3 to get 
 *  the actual value of the battery voltage.
 */
#define ADC_PRE_SCALING_COMPENSATION         3

/* 
 *  Resolutio of ADC conversion:  8-bits --> 255 values
 */
#define ADC_RESOLUTION                       255

/*---------------------------------------------------------------------------*/
/*  Macro to convert the result of ADC conversion in millivolts.             */
/*      value  = (adc_results * ADC_REF_VOLTAGE_IN_MILLIVOLTS);              */
/*      value /= ADC_RESOLUTION;                                             */
/*      value *= ADC_PRE_SCALING_COMPENSATION;                               */
/*---------------------------------------------------------------------------*/
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RESOLUTION) * \
            ADC_PRE_SCALING_COMPENSATION)


/*---------------------------------------------------------------------------*/
/*  Callback function for asserts in the SoftDevice.                         */
/*---------------------------------------------------------------------------*/
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t eddystone_set_adv_data(uint32_t frame_index)
{
    uint8_t * p_encoded_advdata = edstn_frames[frame_index].adv_frame;

    return sd_ble_gap_adv_data_set(p_encoded_advdata, 
                                   edstn_frames[frame_index].adv_len, 
                                   NULL, 0);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t big32cpy(uint8_t * dest, uint32_t val)
{
    dest[3] = (uint8_t) (val >> 0);
    dest[2] = (uint8_t) (val >> 8);
    dest[1] = (uint8_t) (val >> 16);
    dest[0] = (uint8_t) (val >> 24);
    return 4;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t big16cpy(uint8_t * dest, uint16_t val)
{
    dest[1] = (uint8_t) (val >> 0);
    dest[0] = (uint8_t) (val >> 8);
    return 4;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint16_t temperature_data_get(void)
{
    int32_t temp;

    APP_ERROR_CHECK( sd_temp_get(&temp) );

    int8_t hi = (temp / 4);
    int8_t lo = (temp * 25) % 100;

    return (hi << 8) | lo;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint16_t battery_level_get(void)
{
#if defined(PROVISION_BATTERY)

    /* Configure for ADC conversion */
    NRF_ADC->CONFIG = battery_adc_config;

    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;

    /* Stop any running conversions. */
    NRF_ADC->EVENTS_END = 0;    

    /* Start new conversion */
    NRF_ADC->TASKS_START = 1;

    while (!NRF_ADC->EVENTS_END) { /* spin: wait for conversion */ }

    uint16_t voltage_in_mv = ADC_RESULT_IN_MILLI_VOLTS(NRF_ADC->RESULT);

    /* Stop conversion task */
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->TASKS_STOP = 1;

    return voltage_in_mv;

#else
    return 0x0000;  /* "not supported" value */s
#endif
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_tlm_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_TLM].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_TLM].adv_len;

    eddystone_head_encode(encoded_advdata, 0x20, len_advdata);

    encoded_advdata[(*len_advdata)++] = 0x00; // Version

    /* Battery voltage, 1 mV/bit */
    eddystone_uint16(encoded_advdata, len_advdata, battery_level_get());

    /* Beacon temperature */
    eddystone_uint16(encoded_advdata, len_advdata, temperature_data_get());

    /* Advertising PDU count */
    eddystone_uint32(encoded_advdata, len_advdata, pdu_count);

    /* Time since power-on or reboot */
    *len_advdata += big32cpy(encoded_advdata + *len_advdata, pdu_count);

    /* Length   Service Data. Ibid. § 1.11 */
    encoded_advdata[SERVICE_DATA_OFFSET] = (*len_advdata) - 8;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_url_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_URL].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_URL].adv_len;

    eddystone_head_encode(encoded_advdata, 0x10, len_advdata);

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
static void build_uid_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_UID].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_UID].adv_len;

    eddystone_head_encode(encoded_advdata, 0x00, len_advdata);

    encoded_advdata[(*len_advdata)++] = APP_MEASURED_RSSI;

    /* Set Namespace */
    static const uint8_t namespace[] = UID_NAMESPACE;
    memcpy(&encoded_advdata[(*len_advdata)], &namespace, sizeof(namespace));
    *len_advdata += sizeof(namespace);

    /* Set Beacon Id (BID) */
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
/*  Function for initializing the Advertising functionality.                 */
/*---------------------------------------------------------------------------*/
static void advertising_init(void)
{
    build_uid_frame_buffer();
    build_url_frame_buffer();
    build_tlm_frame_buffer();

    eddystone_set_adv_data(EDDYSTONE_UID);

    /* Initialize advertising parameters (used when starting advertising). */
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;             // Undirected advertisement.
    m_adv_params.fp = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout = APP_CFG_NON_CONN_ADV_TIMEOUT;
}

/*---------------------------------------------------------------------------*/
/*  Function for starting advertising.                                       */
/*---------------------------------------------------------------------------*/
static void advertising_start(void)
{
    APP_ERROR_CHECK( sd_ble_gap_adv_start(&m_adv_params) );

    APP_ERROR_CHECK( bsp_indication_set(BSP_INDICATE_ADVERTISING) );
}

/*---------------------------------------------------------------------------*/
/*  Function for initializing the BLE stack.                                 */
/*---------------------------------------------------------------------------*/
static void ble_stack_init(void)
{
    /* Initialize the SoftDevice handler module. */
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

    /* Enable BLE stack */ 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;

    APP_ERROR_CHECK( sd_ble_enable(&ble_enable_params) );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void eddystone_interleave(bool radio_active)
{
    if (radio_active == false)
        return;

    if (pdu_count % 9 == 0) {
        build_tlm_frame_buffer();  // refresh TLM data
        eddystone_set_adv_data(EDDYSTONE_TLM);
    }
    else if (pdu_count % 5 == 0) {
        eddystone_set_adv_data(EDDYSTONE_URL);
    }
    else if (pdu_count % 3 == 0) {
        eddystone_set_adv_data(EDDYSTONE_UID);
    }

    pdu_count++;
}

/*---------------------------------------------------------------------------*/
/*  Function for doing power management.                                     */
/*---------------------------------------------------------------------------*/
static void power_manage(void)
{
    APP_ERROR_CHECK( sd_app_evt_wait() );
}

/*---------------------------------------------------------------------------*/
/*  Function for application main entry.                                     */
/*---------------------------------------------------------------------------*/
int main(void)
{
    uint32_t err_code;

    /* Initialize. */
    APP_TIMER_INIT(APP_TIMER_PRESCALER,
                   APP_TIMER_MAX_TIMERS,
                   APP_TIMER_OP_QUEUE_SIZE,
                   false);

    err_code = bsp_init(BSP_INIT_LED,
                        APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                        NULL);
    APP_ERROR_CHECK(err_code);

    ble_stack_init();

#if defined(PROVISION_DBGLOG)
    uart_init();
#endif

    PRINTF("\n*** Eddystone: %s %s ***\n\n", __DATE__, __TIME__);

    err_code = ble_radio_notification_init(NRF_APP_PRIORITY_LOW,
                                           NRF_RADIO_NOTIFICATION_DISTANCE_5500US,
                                           eddystone_interleave);

    advertising_init();

    APP_ERROR_CHECK(err_code);

    /* Start execution. */
    advertising_start();

    /* Enter main loop. */
    for (;;) {
        power_manage();
    }
}

