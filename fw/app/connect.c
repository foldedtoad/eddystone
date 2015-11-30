/*---------------------------------------------------------------------------*/
/*  connect.h                                                                */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <ble_advdata.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "pstorage.h"

#include "config.h"
#include "dbglog.h"
#include "pstorage_platform.h"
#include "ble_dfu.h"
#include "dfu_app_handler.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* Connection handle */
static uint16_t                  m_conn_handle = BLE_CONN_HANDLE_INVALID;

/* DFU Service handle */
static ble_dfu_t                 m_dfus;

/* Application identifier allocated by device manager. */
static dm_application_instance_t m_app_handle;

/* Security requirements for this application. */
static ble_gap_sec_params_t      m_sec_params;              

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS) {

        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0) {

            err_code = sd_ble_gatts_service_changed(m_conn_handle, 
                                                    APP_SERVICE_HANDLE_START, 
                                                    BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {

                APP_ERROR_HANDLER(err_code);
            }
        }

        APP_ERROR_CHECK( dm_application_context_delete(p_handle) );
    }
    else if (err_code == DM_NO_APP_CONTEXT) {
        // No context available. Ignore.
    }
    else {
        APP_ERROR_HANDLER(err_code);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           uint32_t            event_result)
{
    APP_ERROR_CHECK(event_result);

    switch (p_event->event_id) {

        case DM_EVT_LINK_SECURED:
            puts("LINK SECURED event");

            app_context_load(p_handle);

            break;

        default:
            // No implementation needed.
            break;
    }

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void device_manager_init(void)
{
    dm_init_param_t         init_data;
    dm_application_param_t  register_param;

    memset(&init_data, 0, sizeof(init_data));

#if 0
    // Clear all bonded centrals if the Bonds Delete button is pushed.
    init_data.clear_persistent_data = (nrf_gpio_pin_read(TEMP_PAIR_BUTTON_PIN) == 0);
#endif

    APP_ERROR_CHECK( dm_init(&init_data) );

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    APP_ERROR_CHECK( dm_register(&m_app_handle, &register_param) );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void sec_params_init(void)
{
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint32_t service_changed_indicate(void)
{
    uint32_t  err_code;

    err_code = sd_ble_gatts_service_changed(m_conn_handle,
                                            APP_SERVICE_HANDLE_START,
                                            BLE_HANDLE_MAX);
    switch (err_code) {

        case NRF_SUCCESS:
            PUTS("Service Changed indicated"); 
            break;

        case BLE_ERROR_INVALID_CONN_HANDLE:
        case NRF_ERROR_INVALID_STATE:
        case BLE_ERROR_NO_TX_BUFFERS:
        case NRF_ERROR_BUSY:
        case BLE_ERROR_GATTS_SYS_ATTR_MISSING:
            PRINTF("service_changed minor error: %u\n", (unsigned)err_code); 
            break;

        case NRF_ERROR_NOT_SUPPORTED:
            PUTS("service_changed not supported");
            break;

        default:
            APP_ERROR_HANDLER(err_code);
            break;
    }

    return err_code;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {

        uint32_t err_code;

        err_code = sd_ble_gap_disconnect(m_conn_handle, 
                                         BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void conn_params_error_handler(uint32_t error)
{
    APP_ERROR_HANDLER( error );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void conn_params_init(void)
{
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    APP_ERROR_CHECK( ble_conn_params_init(&cp_init) );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void gap_params_init(void)
{
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    APP_ERROR_CHECK( sd_ble_gap_ppcp_set(&gap_conn_params) );
}

/*---------------------------------------------------------------------------*/
/*  On confirm of Service Changed, start bootloader.                         */
/*---------------------------------------------------------------------------*/
static void service_changed_evt(ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->header.evt_id == BLE_GATTS_EVT_SC_CONFIRM) {

        PUTS("Service Changed confirmed");

        /* Starting the bootloader - will cause reset. */
        bootloader_start(m_conn_handle);
    }
}
/*---------------------------------------------------------------------------*/
/*  DFU BLE Reset Prepare                                                    */
/*---------------------------------------------------------------------------*/
static void reset_prepare(void)
{
    PUTS(__func__);

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        /* Disconnect from peer. */
        uint8_t status = BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION;

        APP_ERROR_CHECK( sd_ble_gap_disconnect(m_conn_handle, status) );
    }
}

/*---------------------------------------------------------------------------*/
/*  Initializing the DFU Service.                                            */
/*---------------------------------------------------------------------------*/
static void dfu_init(void)
{
    ble_dfu_init_t   dfus_init;

    /* Initialize the Device Firmware Update Service. */
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;

    APP_ERROR_CHECK( ble_dfu_init(&m_dfus, &dfus_init) );

    dfu_app_reset_prepare_set(reset_prepare);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void services_init(void)
{    
    dfu_init();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void storage_init(void)
{    
    APP_ERROR_CHECK( pstorage_init() );
}
