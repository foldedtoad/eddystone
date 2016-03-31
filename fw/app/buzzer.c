/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "app_timer.h"

#include "boards.h"
#include "buzzer.h"
#include "nrf_pwm.h"
#include "dbglog.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define BUZZER_PIN (BUZZ) 

#define TIMER_DELAY_ONE_MS APP_TIMER_TICKS( 1, APP_TIMER_PRESCALER )

static app_timer_id_t   m_buzzer_timer_id;  /* Timer for driving the buzzer */

static bool  in_play = false;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
    nrf_gpio_pin_clear(BUZZER_PIN);    // In sure buzzer GPIO is off.
    nrf_pwm_set_value(0, 0);           // Turn off PWM to buzzer

    /*
     *   Single play.
     */
    if (playlist == NULL) {
        return;
    }

    /*
     *   Play list.
     */
    switch (playlist->action) {

        case BUZZER_PLAY_TONE:

            // Enable while actively generating PWM signal.
            nrf_pwm_set_enabled(true);

            // Turn on PWM wave source.
            nrf_pwm_set_value(0, 62);  

            app_timer_start(m_buzzer_timer_id, 
                            (playlist->duration * TIMER_DELAY_ONE_MS), 
                            &playlist[1]);
            break;

        case BUZZER_PLAY_QUIET:

            // Disable while quiet: conserve power.
            nrf_pwm_set_enabled(false);

            app_timer_start(m_buzzer_timer_id, 
                            (playlist->duration * TIMER_DELAY_ONE_MS), 
                            &playlist[1]);
            break;

        case BUZZER_PLAY_DONE:
        default:

            // Disable when done: conserve power.
            nrf_pwm_set_enabled(false);

            // Done: set BUZZER pin to "output".
            in_play = false;
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
    if (in_play) return NRF_ERROR_BUSY;

    PUTS(__func__);

    in_play = true;

    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);

    buzzer_process_playlist(playlist);

    return NRF_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    PUTS(__func__);

    app_timer_stop(m_buzzer_timer_id);

    nrf_pwm_set_enabled(false);
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

    {
        nrf_pwm_config_t pwm_config = PWM_DEFAULT_CONFIG;
        
        pwm_config.mode             = PWM_MODE_BUZZER;
        pwm_config.num_channels     = 1;
        pwm_config.gpio_num[0]      = BUZZER_PIN;        
        
        nrf_gpio_cfg_output(BUZZER_PIN);

        nrf_pwm_init(&pwm_config);
    }

    nrf_pwm_set_enabled(false);

    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);
}

