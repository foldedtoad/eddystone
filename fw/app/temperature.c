/*---------------------------------------------------------------------------*/
/*  temperature.c                                                            */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"

#include "config.h"
#include "temperature.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint16_t temperature_data_get(void)
{
    int32_t temp;

    APP_ERROR_CHECK( sd_temp_get(&temp) );

    int8_t hi = (temp / 4);
    int8_t lo = (temp * 25) % 100;

    return (hi << 8) | lo;
}
