/*---------------------------------------------------------------------------*/
/*  advert.h                                                                 */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#include "nrf51.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_scheduler.h"
#include "ble_advdata.h"

#include "config.h"
#include "advert.h"
#include "connect.h"
#include "eddystone.h"
#include "dbglog.h"
#include "ble_dfu.h"
#include "ble_gap.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ble_gap_adv_params_t   m_adv_params;

/*---------------------------------------------------------------------------*/
/*  Function for initializing the Advertising functionality.                 */
/*---------------------------------------------------------------------------*/
void advertising_init(void)
{
    PUTS(__func__);

    /* Build (refresh) all Eddystone frames. */
    eddystone_init();

    /* Initialize advertising parameters (used when starting advertising). */
    memset(&m_adv_params, 0, sizeof(m_adv_params));

#if 0
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT;
#else
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT;
#endif
}

/*---------------------------------------------------------------------------*/
/*  Function for starting advertising.                                       */
/*---------------------------------------------------------------------------*/
void advertising_start(void)
{
    PUTS(__func__);

    APP_ERROR_CHECK( sd_ble_gap_adv_start(&m_adv_params) );

    APP_ERROR_CHECK( bsp_indication_set(BSP_INDICATE_ADVERTISING) );
}
