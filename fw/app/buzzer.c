/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#include "nrf.h"
#include "nrf_soc.h"
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

#define BUZZ_TIMER                NRF_TIMER2

#define GPIOTE_CHANNEL_NUMBER_0   0
#define GPIOTE_CHANNEL_NUMBER_1   1

#define TIMER_DELAY_ONE_MS APP_TIMER_TICKS( 1, APP_TIMER_PRESCALER )

static app_timer_id_t   m_buzzer_timer_id;

/*---------------------------------------------------------------------------*/
/* Configure the GPIOs used to toggle on a GPIOTE task.                      */
/*---------------------------------------------------------------------------*/
static void buzzer_gpiote_config(void)
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
/* Unconfigure the GPIOs used to toggle on a GPIOTE task.                    */
/* Insure GPIO lines reconfigured for output and are set low.                */
/*---------------------------------------------------------------------------*/
static void buzzer_gpiote_unconfig(void)
{
    nrf_gpiote_unconfig(GPIOTE_CHANNEL_NUMBER_0);
    nrf_gpiote_unconfig(GPIOTE_CHANNEL_NUMBER_1);

    nrf_gpio_cfg_output(BUZZ1);
    nrf_gpio_cfg_output(BUZZ2);

    nrf_gpio_pin_clear(BUZZ1);
    nrf_gpio_pin_clear(BUZZ2);
}

/*---------------------------------------------------------------------------*/
/* Use BUZZ_TIMER to generate events 'frequency' period.                     */
/*---------------------------------------------------------------------------*/
static void buzzer_timer_config(uint16_t frequency)
{
    /* Start 16 MHz crystal oscillator. */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start. */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { /* spin */ }

    /* Clear TIMER0 */
    BUZZ_TIMER->TASKS_CLEAR = 1;

    /*
     * Configure TIMER0 for compare[0] event every 125µs.
     */
    BUZZ_TIMER->PRESCALER = 2;
    BUZZ_TIMER->CC[0]     = frequency;
    BUZZ_TIMER->MODE      = TIMER_MODE_MODE_Timer;
    BUZZ_TIMER->BITMODE   = TIMER_BITMODE_BITMODE_24Bit;
    BUZZ_TIMER->SHORTS    = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
}

/*---------------------------------------------------------------------------*/
/*  Use a PPI channel to connect the event to the task automatically.        */
/*---------------------------------------------------------------------------*/ 
static void buzzer_ppi_config(void)
{
    /*  
     *  Configure PPI channel 0 to toggle GPIO_OUTPUT_PIN on 
     *  every TIMER0 COMPARE[0] match (300µs)
     */
    sd_ppi_channel_assign(GPIOTE_CHANNEL_NUMBER_0,
                          &BUZZ_TIMER->EVENTS_COMPARE[0],
                          &NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_NUMBER_0]);

    sd_ppi_channel_assign(GPIOTE_CHANNEL_NUMBER_1, 
                          &BUZZ_TIMER->EVENTS_COMPARE[0], 
                          &NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_NUMBER_1]);

    /* Enable PPI channels */
    sd_ppi_channel_enable_set((PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos) |
                              (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
    switch (playlist->action) {

        case BUZZER_PLAY_TONE:

            BUZZ_TIMER->TASKS_STOP = 1;

            buzzer_gpiote_config();
            buzzer_timer_config(playlist->frequency);
            buzzer_ppi_config();

            BUZZ_TIMER->TASKS_START = 1;

            app_timer_start(m_buzzer_timer_id,
                            (playlist->duration * TIMER_DELAY_ONE_MS),
                            &playlist[1]);
            break;

        case BUZZER_PLAY_QUIET:

            BUZZ_TIMER->TASKS_STOP = 1;
            buzzer_gpiote_unconfig();

            app_timer_start(m_buzzer_timer_id,
                            (playlist->duration * TIMER_DELAY_ONE_MS),
                            &playlist[1]);
            break;

        case BUZZER_PLAY_DONE:
        default:
            BUZZ_TIMER->TASKS_STOP = 1;
            buzzer_gpiote_unconfig();
            break;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_timeout_handler(void * context)
{
    buzzer_play_t * playlist = (buzzer_play_t*) context;

    buzzer_process_playlist(playlist);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_play_execute(void * p_data, uint16_t size)
{
    buzzer_play_t ** playlist = (buzzer_play_t**) p_data;

    buzzer_process_playlist(*playlist);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_play(buzzer_play_t * playlist)
{
    app_sched_event_put(&playlist, sizeof(playlist), buzzer_play_execute);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    app_timer_stop(m_buzzer_timer_id);

    BUZZ_TIMER->TASKS_STOP = 1;
    buzzer_gpiote_unconfig();
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
}
