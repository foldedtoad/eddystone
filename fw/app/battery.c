/*---------------------------------------------------------------------------*/
/*  battery.c                                                                */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "app_scheduler.h"

#include "config.h"
#include "battery.h"
#include "dbglog.h"

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
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint16_t battery_level_get(void)
{
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
}
