/* 
 *  Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#ifndef PILSY_BOARD_H
#define PILSY_BOARD_H

#include "nrf_gpio.h"

#define HARDWARE_REV_NAME   "HW 2.0"

#define LFCLKSRC_OPTION     NRF_CLOCK_LFCLKSRC_XTAL_20_PPM

#define PULLDOWN            NRF_GPIO_PIN_PULLDOWN
#define PULLUP              NRF_GPIO_PIN_PULLUP
#define NOPULL              NRF_GPIO_PIN_NOPULL

#define LED_1               1
#define LED_2               2

#define BUTTON_1            0

#define TX_PIN_NUMBER       14
#define RX_PIN_NUMBER       17

#define __breakpoint     __ASM volatile ( "bkpt \n" )

#endif /* PILSY_BOARD_H */
