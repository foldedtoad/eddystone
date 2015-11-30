/*---------------------------------------------------------------------------*/
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <ble_advdata.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "ble_radio_notification.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "app_scheduler.h"

#include "config.h"
#include "connect.h"
#include "eddystone.h"
#include "dbglog.h"
#include "ble_dfu.h"
#include "ble_gap.h"
#include "dfu_app_handler.h"

#if defined(PROVISION_DBGLOG)
  #include "uart.h"
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define FICR_DEVICEADDR   ((uint8_t*) &NRF_FICR->DEVICEADDR[0])

#define EDDYSTONE_UID_TYPE    0x00
#define EDDYSTONE_URL_TYPE    0x10
#define EDDYSTONE_TLM_TYPE    0x20

#define SERVICE_DATA_OFFSET  0x07

#define URL_PREFIX__http_www     0x00
#define URL_PREFIX__https_www    0x01
#define URL_PREFIX__http         0x02
#define URL_PREFIX__https        0x03

#define TLM_VERSION   0x00

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static edstn_frame_t edstn_frames[3];

static uint32_t adv_cnt = 0;
static uint32_t sec_cnt = 0;

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


#if defined(PROVISION_DBGLOG)
static const struct {
    uint16_t  error_code;
    char    * text;
} nrf_errors [] = {
    { NRF_ERROR_SVC_HANDLER_MISSING,    "SVC_HANDLER_MISSING" },
    { NRF_ERROR_SOFTDEVICE_NOT_ENABLED, "SD_NOT_ENABLED"      },
    { NRF_ERROR_INTERNAL,               "INTERNAL"            },
    { NRF_ERROR_NO_MEM,                 "NO_MEM"              },
    { NRF_ERROR_NOT_FOUND,              "NOT_FOUND"           },
    { NRF_ERROR_NOT_SUPPORTED,          "NOT_SUPPORTED"       },
    { NRF_ERROR_INVALID_PARAM,          "INVALID_PARAMETER"   },
    { NRF_ERROR_INVALID_STATE,          "INVALID_STATE"       },
    { NRF_ERROR_INVALID_LENGTH,         "INVALID_LENGTH"      },
    { NRF_ERROR_INVALID_FLAGS,          "INVALID_FLAGS"       },
    { NRF_ERROR_INVALID_DATA,           "INVALID_DATA"        },
    { NRF_ERROR_DATA_SIZE,              "DATA_SIZE"           },
    { NRF_ERROR_TIMEOUT,                "TIMEOUT"             },
    { NRF_ERROR_NULL,                   "NULL"                },
    { NRF_ERROR_FORBIDDEN,              "FORBIDDEN"           },
    { NRF_ERROR_INVALID_ADDR,           "INVALID_ADDR"        },
    { NRF_ERROR_BUSY,                   "BUSY"                },
};
#define NRF_ERRORS_COUNT (sizeof(nrf_errors)/sizeof(nrf_errors[0]))
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
#if defined(PROVISION_DBGLOG)
    char * text = "??";
    (void) text;

    for (int i=0; i < NRF_ERRORS_COUNT; i++) {
        if (error_code == nrf_errors[i].error_code) {
            text = nrf_errors[i].text;
            break;
        }
    }
    PRINTF("%s: NRF_ERROR_%s 0x%x, at %s(%d)\n", __func__, text, 
           (unsigned)error_code, (char*)p_file_name, (int)line_num);

#endif

#if defined(DEBUG)
    __disable_irq();
    __BKPT(0);
    while (1) { /* spin */}

#else
    // On assert, the system can only recover with a reset.   
    NVIC_SystemReset();
#endif
}

/*---------------------------------------------------------------------------*/
/*  Callback function for asserts in the SoftDevice.                         */
/*---------------------------------------------------------------------------*/
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    __BKPT();
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
    /* Set "not supported" value */
    return 0x0000;
#endif
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void build_tlm_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_TLM].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_TLM].adv_len;

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
static void build_url_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_URL].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_URL].adv_len;

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
static void build_uid_frame_buffer()
{
    uint8_t * encoded_advdata = edstn_frames[EDDYSTONE_UID].adv_frame;
    uint8_t * len_advdata     = &edstn_frames[EDDYSTONE_UID].adv_len;

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

#if 0  
    /* Undirected advertisement with non-connectability. */
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT;
#else
    /* Advertisement with connectability */
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT;
#endif

}

/*---------------------------------------------------------------------------*/
/*  Function for starting advertising.                                       */
/*---------------------------------------------------------------------------*/
static void advertising_start(void)
{
    APP_ERROR_CHECK( sd_ble_gap_adv_start(&m_adv_params) );

    APP_ERROR_CHECK( bsp_indication_set(BSP_INDICATE_ADVERTISING) );

    PUTS(__func__);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void eddystone_scheduler(bool radio_is_active)
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
    }
    else if ((iterations % 5) == 0) {
        eddystone_set_adv_data(EDDYSTONE_URL);
        adv_cnt++;
    }
    else if ((iterations % 3) == 0) {
        eddystone_set_adv_data(EDDYSTONE_UID);
        adv_cnt++;
    }
}

/*---------------------------------------------------------------------------*/
/*  Function for initializing the BLE stack.                                 */
/*---------------------------------------------------------------------------*/
static void ble_stack_init(void)
{
    ble_gap_addr_t      addr;
    ble_enable_params_t ble_enable_params;

    /* Initialize the SoftDevice handler module. */
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);

    /* Enable BLE stack */ 
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    APP_ERROR_CHECK( sd_ble_enable(&ble_enable_params) );

    sd_ble_gap_address_get(&addr);
    sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &addr);

    /* Subscribe for BLE events. */
    APP_ERROR_CHECK( softdevice_ble_evt_handler_set(ble_evt_dispatch) );

    /* Register with the SoftDevice handler module for BLE events. */
    APP_ERROR_CHECK( softdevice_sys_evt_handler_set(sys_evt_dispatch) );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void timer_init(void)
{
    uint32_t err_code;

    APP_TIMER_INIT(APP_TIMER_PRESCALER,
                   APP_TIMER_MAX_TIMERS,
                   APP_TIMER_OP_QUEUE_SIZE,
                   false);

    err_code = bsp_init(BSP_INIT_LED,
                        APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                        NULL);
    APP_ERROR_CHECK(err_code);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void radio_init(void)
{
    uint32_t err_code;

    err_code = ble_radio_notification_init(NRF_APP_PRIORITY_LOW,
                                           NRF_RADIO_NOTIFICATION_DISTANCE_5500US,
                                           eddystone_scheduler);
    APP_ERROR_CHECK(err_code);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
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
    /* Initialize. */
    ble_stack_init();
    scheduler_init();

#if defined(PROVISION_DBGLOG)
    uart_init();
#endif

    PRINTF("\n*** Eddystone: %s %s ***\n\n", __DATE__, __TIME__);

    storage_init();
    timer_init();
    radio_init();

    /* Configure connection-side for DFU support */
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
    sec_params_init();

    device_manager_init();

    /* Start execution. */
    advertising_start();

    /* Enter main loop. */
    for (;;) {
        app_sched_execute();
        power_manage();
    }
}
