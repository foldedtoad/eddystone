/*---------------------------------------------------------------------------*/
/*  battery.c                                                                */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_soc.h"
#include "nrf52_bitfields.h"
#include "nrf_adc.h"
#include "nrf_drv_saadc.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "app_scheduler.h"
#include "ble_bas.h"

#include "config.h"
#include "battery.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*  Battery Measurement parameters                                           */
/*---------------------------------------------------------------------------*/

/*  
 *  Reference voltage (in milli volts) used by ADC while doing conversion.
 */
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        600

/* 
 *  The ADC is configured to use VDD with 1/6 prescaling as input. 
 *  And hence the result of conversion is to be multiplied by 3 to get 
 *  the actual value of the battery voltage.
 */
#define ADC_PRE_SCALING_COMPENSATION         6

/* 
 *  Resolutio of ADC conversion:  10-bits --> 1024 values
 */
#define ADC_RESOLUTION                       1024

 /*  
  *  Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that 
  *  is connected in series with the voltage supply. This is the voltage drop
  *  when the forward current is 1mA. Source: Data sheet of 
  *  'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com.
  */
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS       270

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
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)

static ble_bas_t           m_bas;
static nrf_saadc_value_t   adc_buf[2];
static app_timer_id_t      m_battery_timer_id;

//static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t  adc_result;

        uint16_t millivolts;
        uint8_t  percentage;
        uint32_t err_code;

        adc_result = p_event->data.done.p_buffer[0] - 100;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
        APP_ERROR_CHECK(err_code);

        millivolts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;

        percentage = battery_level_in_percent(millivolts);

        err_code = ble_bas_battery_level_update(&m_bas, percentage);
        if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
        {
            APP_ERROR_HANDLER(err_code);
        }
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void battery_level_meas_timeout_handler(void * p_context)
{
    APP_ERROR_CHECK(nrf_drv_saadc_sample());
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void adc_configure(void)
{
    APP_ERROR_CHECK( nrf_drv_saadc_init(NULL, saadc_event_handler) );

    nrf_saadc_channel_config_t config = 
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);

    APP_ERROR_CHECK( nrf_drv_saadc_channel_init(0, &config) );

    APP_ERROR_CHECK( nrf_drv_saadc_buffer_convert(&adc_buf[0], 1) );

    APP_ERROR_CHECK( nrf_drv_saadc_buffer_convert(&adc_buf[1], 1) );
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t *p_evt)
{
    uint32_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            // Start battery timer
            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            err_code = app_timer_stop(m_battery_timer_id);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void bas_init(void)
{
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = on_bas_evt;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_report_read_perm);

    APP_ERROR_CHECK( ble_bas_init(&m_bas, &bas_init_obj) );

        // Create battery timer.
    uint32_t err_code = app_timer_create(&m_battery_timer_id,
                                         APP_TIMER_MODE_REPEATED,
                                         battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint16_t battery_level_get(void)
{

    return 0;
}
