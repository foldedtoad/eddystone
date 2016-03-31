/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_timer.h"

#include "boards.h"
#include "buzzer.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define TIMER_DELAY_ONE_MS APP_TIMER_TICKS( 1, APP_TIMER_PRESCALER )

static app_timer_id_t   m_buzzer_timer_id;

static volatile bool    m_playing = false;
static uint32_t         m_freq = 150;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_play_element(void)
{
    bool toggle = false;

    while (m_playing) {
        if (toggle) {
            toggle = false;
            nrf_gpio_pin_set(BUZZ2);
            nrf_gpio_pin_clear(BUZZ1);
        }
        else {
            toggle = true;
            nrf_gpio_pin_set(BUZZ1);
            nrf_gpio_pin_clear(BUZZ2);
        }
        nrf_delay_us(m_freq); 
    }

    nrf_gpio_pin_clear(BUZZ1);
    nrf_gpio_pin_clear(BUZZ2);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_play_execute(void * p_event_data, uint16_t event_size)
{
    m_playing = true;

    buzzer_play_element();
}

/*---------------------------------------------------------------------------*/
/*  This is a crappy way to generate a differencial square wave.             */
/*  Should investigate using gpiote to drive buzzer pins.                    */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
    switch (playlist->action) {

        case BUZZER_PLAY_TONE:
            app_timer_start(m_buzzer_timer_id,
                            (playlist->duration * TIMER_DELAY_ONE_MS),
                            &playlist[1]);

            app_sched_event_put(NULL, 0, buzzer_play_execute);
            break;

        case BUZZER_PLAY_QUIET:
            m_playing = false;
            app_timer_start(m_buzzer_timer_id, 
                            (playlist->duration * TIMER_DELAY_ONE_MS), 
                            &playlist[1]);
            break;

        case BUZZER_PLAY_DONE:
        default:
            m_playing = false;
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
uint32_t buzzer_play(buzzer_play_t * playlist)
{
    PUTS(__func__);

    buzzer_process_playlist(playlist);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    PUTS(__func__);

    m_playing = false;

    app_timer_stop(m_buzzer_timer_id);
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

    nrf_gpio_cfg_output(BUZZ1);
    nrf_gpio_cfg_output(BUZZ2);

    nrf_gpio_pin_clear(BUZZ1);
    nrf_gpio_pin_clear(BUZZ2);
}

