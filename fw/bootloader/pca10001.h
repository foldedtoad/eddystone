/*
 *  Copyright (c) 2015 Moment Inc. All Rights Reserved.
 */
#ifndef PCA10001_H
#define PCA10001_H

#define PULLDOWN            NRF_GPIO_PIN_PULLDOWN
#define PULLUP              NRF_GPIO_PIN_PULLUP
#define NOPULL              NRF_GPIO_PIN_NOPULL

#define LENS_IDENTIFY_MASK  0x000000F8
#define LENS_DETECT_MASK    0x00003000

#define HWFC                true

/*---------------------------------------------------------------------------*/

#define RTS_PIN_NUMBER      8
#define TX_PIN_NUMBER       9
#define CTS_PIN_NUMBER      10
#define RX_PIN_NUMBER       11

#define DETECT_RC           14
#define DETECT_DRV          15

#define BUTTON_START        16
#define BUTTON_0            16
#define SHUTTER_BUTTON      BUTTON_0
#define BUTTON_1            17
#define SHUTTER_HALF        BUTTON_1
#define BUTTON_STOP         17
#define BUTTON_PULL         PULLUP

#define LED_START           18
#define LED_0               18
#define LED                 LED_0
#define LED_1               19
#define LED_STOP            19

/*----------------------------------------------------------------------------*/

#define BUTTONS_NUMBER      2
#define BSP_BUTTON_0        BUTTON_0
#define BSP_BUTTON_1        BUTTON_1
#define BUTTONS_MASK        0x00030000
#define BSP_BUTTON_0_MASK   (1 << BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK   (1 << BSP_BUTTON_1)
#define BUTTONS_LIST        { BUTTON_0, BUTTON_1 }

#define LEDS_NUMBER         2
#define BSP_LED_0           LED_0
#define BSP_LED_1           LED_1
#define LEDS_MASK           0x000C0000
#define LEDS_INV_MASK       0
#define BSP_LED_0_MASK      (1 << LED_0)
#define BSP_LED_1_MASK      (1 << LED_1)
#define LEDS_LIST           { LED_0, LED_1 }


/*----------------------------------------------------------------------------*/

#endif
