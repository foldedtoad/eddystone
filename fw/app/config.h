/* 
 *  Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#ifndef _CONFIG_H__
#define _CONFIG_H__

#include <stdint.h>

#include "app_timer.h"
#include "app_scheduler.h"

#include "bsp.h"

/* 
 *  Include or not the service_changed characteristic. 
 *  if not enabled, the server's database cannot be changed for 
 *  the lifetime of the device
 */
#define IS_SRVC_CHANGED_CHARACT_PRESENT 1

/* 
 *  Time for which the device must be advertising in 
 *  non-connectable mode (in seconds). 
 *  A value of 0 disables timeout. 
 */
#define APP_CFG_NON_CONN_ADV_TIMEOUT    0

/*
 *  GAP parameters
 */
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY                   0 
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)

/* 
 *  Time from initiating event (connect or start of notification) to first 
 *  time sd_ble_gap_conn_param_update is called (15 seconds).
 */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER)

/* 
 *  Time between each call to sd_ble_gap_conn_param_update 
 *  after the first call (5 seconds).
 */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) 

/* 
 *  Number of attempts before giving up the connection parameter negotiation.
 */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

/*
 *  Security Parameters
 */
#define SEC_PARAM_TIMEOUT               30 
#define SEC_PARAM_BOND                  1 
#define SEC_PARAM_MITM                  0
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE
#define SEC_PARAM_OOB                   0 
#define SEC_PARAM_MIN_KEY_SIZE          7 
#define SEC_PARAM_MAX_KEY_SIZE          16

/* 
 *  The advertising interval for advertisement (100 ms).
 *  This value can vary between 100ms to 10.24s). 
 */
#define ADVERTISEMENT_INTERVAL          100
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(ADVERTISEMENT_INTERVAL, UNIT_0_625_MS)

#define DEAD_BEEF                       0xDEADBEEF 

#define APP_TIMER_PRESCALER             0
#define APP_TIMER_MAX_TIMERS            (2 + BSP_APP_TIMERS_NUMBER)
#define APP_TIMER_OP_QUEUE_SIZE         4

/* 
 *  Maximum size of scheduler events. 
 *  Note that scheduler BLE stack events do not contain any data, as the 
 *  events are being pulled from the stack in the event handler.
 */
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)

/* 
 *  Maximum number of events in the scheduler queue.
 */
#define SCHED_QUEUE_SIZE                10

/*
 *  The Beacon's measured RSSI at 1 meter distance in dBm.
 */
#define APP_MEASURED_RSSI               0xC3

#define VBAT_MAX_IN_MV                  3300

#define EDDYSTONE_UID                   0
#define EDDYSTONE_URL                   1
#define EDDYSTONE_TLM                   2

/* 
 *  Handle of first application specific service when when 
 *  service changed characteristic is present.
 */
#define APP_SERVICE_HANDLE_START        0x000C

/* 
 *  Max handle value in BLE.
 */
#define BLE_HANDLE_MAX                  0xFFFF

/* 
 *  Go to https://goo.gl for utility to shortening URL names.
 */
#define URL_STRING                      "goo.gl/jjurOU"
#define URL_LENGTH                      (sizeof(URL_STRING) - 1)

/*
 *  8C257BA1-E4F7-4026-A735-B6C01043EEA4  UUID  (generated with uuidgen)
 *  8C257BA1B6C01043EEA4                  Namespace
 */
#define UID_NAMESPACE                   {0x8C,0x25,0x7B,0xA1,0xB6,0xC0,0x10,0x43,0xEE,0xA4}

#endif /* _CONFIG_H__ */
