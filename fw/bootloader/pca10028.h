/*
 *  Copyright (c) 2015 Moment Inc. All Rights Reserved.
 */
#ifndef PCA10028_H
#define PCA10028_H

#include "nrf_gpio.h"

#define HARDWARE_REV_NAME   "PCA10028"

#define LFCLKSRC_OPTION     NRF_CLOCK_LFCLKSRC_XTAL_20_PPM

#define PULLDOWN            NRF_GPIO_PIN_PULLDOWN
#define PULLUP              NRF_GPIO_PIN_PULLUP
#define NOPULL              NRF_GPIO_PIN_NOPULL

#define LED_1               21

#define BUTTON_1            17

#define RTS_PIN_NUMBER      8
#define TX_PIN_NUMBER       9
#define CTS_PIN_NUMBER      10
#define RX_PIN_NUMBER       11
#define HWFC                false

#endif // PCA10028_H
