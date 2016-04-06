/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"
#include "app_timer.h"

#include "boards.h"
#include "buzzer.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define GPIOTE_CHANNEL_NUMBER_0   0
#define GPIOTE_CHANNEL_NUMBER_1   1

#define TIMER_DELAY_ONE_MS APP_TIMER_TICKS( 1, APP_TIMER_PRESCALER )

static app_timer_id_t   m_buzzer_timer_id;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static __INLINE void nrf_gpio_pin_pulse(uint32_t pin_number)
{
    NRF_GPIO->OUTSET = (1 << pin_number);

    NRF_GPIO->OUTCLR = (1 << pin_number);
}

/*---------------------------------------------------------------------------*/
/* Configure a GPIO to toggle on a GPIOTE task.                              */
/*---------------------------------------------------------------------------*/
static void gpiote_init(void)
{
    /*
     * Configure GPIOTE_CHANNEL_NUMBERs to toggle the GPIO pins state.
     * NOTE: Only one GPIOTE task can be coupled to any output pin.
     */
    nrf_gpiote_task_config(GPIOTE_CHANNEL_NUMBER_0,
                           BUZZ1,
                           NRF_GPIOTE_POLARITY_TOGGLE,
                           NRF_GPIOTE_INITIAL_VALUE_LOW);

    nrf_gpiote_task_config(GPIOTE_CHANNEL_NUMBER_1,
                           BUZZ2,
                           NRF_GPIOTE_POLARITY_TOGGLE,
                           NRF_GPIOTE_INITIAL_VALUE_HIGH);
}

/*---------------------------------------------------------------------------*/
/* Use TIMER0 to generate events every 300 µs.                               */
/*---------------------------------------------------------------------------*/
static void timer0_init(void)
{
    /* Start 16 MHz crystal oscillator. */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start. */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { /* spin */ }

    /* Clear TIMER0 */
    NRF_TIMER0->TASKS_CLEAR = 1;

    /*
     * Configure TIMER0 for compare[0] event every 125µs.
     */
    NRF_TIMER0->PRESCALER = 2;
    NRF_TIMER0->CC[0]     = 500;
    NRF_TIMER0->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER0->BITMODE   = TIMER_BITMODE_BITMODE_24Bit;
    NRF_TIMER0->SHORTS    = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
}

/*---------------------------------------------------------------------------*/
/*  Use a PPI channel to connect the event to the task automatically.        */
/*---------------------------------------------------------------------------*/ 
static void ppi_init(void)
{
    /*  
     *  Configure PPI channel 0 to toggle GPIO_OUTPUT_PIN on 
     *  every TIMER0 COMPARE[0] match (300µs)
     */
    NRF_PPI->CH[0].EEP = (uint32_t) &NRF_TIMER0->EVENTS_COMPARE[0];
    NRF_PPI->CH[0].TEP = (uint32_t) &NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_NUMBER_0];

    NRF_PPI->CH[1].EEP = (uint32_t) &NRF_TIMER0->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t) &NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_NUMBER_1];

    /* Enable PPI channels */
    NRF_PPI->CHEN = (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos) |
                    (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos);
}

/*---------------------------------------------------------------------------*/
/*  This is a crappy way to generate a differencial square wave.             */
/*  Should investigate using gpiote to drive buzzer pins.                    */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
    switch (playlist->action) {

        case BUZZER_PLAY_TONE:

            gpiote_init();
            timer0_init();
            ppi_init();

            NRF_TIMER0->TASKS_START = 1;

            nrf_gpio_pin_pulse(TP1);

            app_timer_start(m_buzzer_timer_id,
                            (playlist->duration * TIMER_DELAY_ONE_MS),
                            &playlist[1]);
            break;

        case BUZZER_PLAY_QUIET:

            NRF_TIMER0->TASKS_STOP = 1;

            //nrf_gpio_pin_pulse(TP1);
            //nrf_gpio_pin_pulse(TP1);

            app_timer_start(m_buzzer_timer_id,
                            (playlist->duration * TIMER_DELAY_ONE_MS),
                            &playlist[1]);
            break;

        case BUZZER_PLAY_DONE:
        default:
            NRF_TIMER0->TASKS_STOP = 1;
            break;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_timeout_handler(void * context)
{
    buzzer_play_t * playlist = (buzzer_play_t*) context;

    nrf_gpio_pin_pulse(TP1);
    nrf_gpio_pin_pulse(TP1);

    buzzer_process_playlist(playlist);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint32_t buzzer_play(buzzer_play_t * playlist)
{
    buzzer_process_playlist(playlist);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    app_timer_stop(m_buzzer_timer_id);

    NRF_TIMER0->TASKS_STOP = 1;
}

/*---------------------------------------------------------------------------*/
/*  Must be initialized before SoftDevice initialization.                    */
/*---------------------------------------------------------------------------*/
void buzzer_init(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_buzzer_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                buzzer_timeout_handler);
    APP_ERROR_CHECK(err_code);

    *(uint32_t *)0x40008C0C = 1;  // PAN-73 workaround

    nrf_gpio_cfg_output(BUZZ1);
    nrf_gpio_cfg_output(BUZZ2);

    nrf_gpio_pin_clear(BUZZ1);
    nrf_gpio_pin_clear(BUZZ2);

    nrf_gpio_cfg_output(TP1);
    nrf_gpio_pin_clear(TP1);
}
