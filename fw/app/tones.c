/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdint.h>

#include "config.h"
#include "buzzer.h"

/*---------------------------------------------------------------------------*/
/* Collection of sounds and tones                                            */
/*---------------------------------------------------------------------------*/

buzzer_play_t startup_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=700},   // long buzz    700ms
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};

buzzer_play_t two_beeps_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};

buzzer_play_t three_beeps_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};
