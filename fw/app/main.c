/*---------------------------------------------------------------------------*/
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nrf_soc.h"
#include "ble_radio_notification.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "ble_advdata.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "app_scheduler.h"

#include "config.h"
#include "advert.h"
#include "connect.h"
#include "eddystone.h"
#include "dbglog.h"

#if defined(PROVISION_DBGLOG)
  #include "uart.h"
#endif

 /* 
  *  Number of central/peripheral links used by the application. 
  *  When changing this number remember to adjust the RAM settings.
  */
#define CENTRAL_LINK_COUNT                   0
#define PERIPHERAL_LINK_COUNT                1

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
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
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    error_info_t  * pError;
    assert_info_t * pAssert;
    char          * text = "??";

    (void) pError;
    (void) pAssert;
   
    (void) text;

    switch (id) {
        case NRF_FAULT_ID_SDK_ERROR:
            pError = (error_info_t*) info;
            for (int i=0; i < NRF_ERRORS_COUNT; i++) {
                if (pError->err_code == nrf_errors[i].error_code) {
                    text = nrf_errors[i].text;
                    break;
                }
            }
            PRINTF("[0x%08x] NRF_ERROR_%s 0x%x at %s(%d)\n",
                (unsigned) pc,
                text, (unsigned) pError->err_code,
                (char*) pError->p_file_name,
                (int) pError->line_num);
            break;

        case NRF_FAULT_ID_SDK_ASSERT:
            pAssert = (assert_info_t*) info;
            PRINTF("[0x%08x] assert at %s(%d)\n",
                (unsigned) pc,
                (char*) pAssert->p_file_name,
                (int) pAssert->line_num);
            break;

        default:
            PRINTF("unrecognized: id(%d)  pc(0x%x)\n",
                (unsigned)id, (unsigned)pc);
            break;
    }

    __disable_irq();

#if defined(DEBUG)
    __BKPT(0);
#endif

    NVIC_SystemReset();
}


/*---------------------------------------------------------------------------*/
/*  Callback function for asserts in the SoftDevice.                         */
/*---------------------------------------------------------------------------*/
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

//*---------------------------------------------------------------------------*/
/*  Function for initializing the BLE stack.                                 */
/*---------------------------------------------------------------------------*/
static void ble_stack_init(void)
{
    uint32_t            err_code;
    ble_gap_addr_t      addr;
    ble_enable_params_t ble_enable_params;

    /* Initialize the SoftDevice handler module. */
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);

    /* Enable BLE stack */ 
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

    APP_ERROR_CHECK( softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                          PERIPHERAL_LINK_COUNT,
                                                          &ble_enable_params) );

    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;

    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    APP_ERROR_CHECK( softdevice_enable(&ble_enable_params) );

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
    ble_stack_init();
    scheduler_init();

#if defined(PROVISION_DBGLOG)
    uart_init();
#endif

    PRINTF("\n*** Eddystone: %s %s ***\n\n", __DATE__, __TIME__);

    storage_init();
    timer_init();
    radio_init();

    gap_params_init();
    services_init();
    eddystone_init();
    conn_params_init();
    sec_params_init();

    device_manager_init();

    advertising_start_connectable();

    /* Enter main loop. */
    for (;;) {
        app_sched_execute();
        power_manage();
    }
}
