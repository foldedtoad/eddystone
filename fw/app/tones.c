/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#include <stdint.h>

#include "config.h"
#include "buzzer.h"

#define note_A          564
#define note_A_sharp    532
#define note_B          564
#define note_C          482
#define note_C_sharp    450
#define note_D          427
#define note_D_sharp    397
#define note_E          380
#define note_F          345
#define note_F_sharp    335
#define note_G          322
#define note_G_sharp    300

#define REST1       60
#define REST2       160
#define REST4       600
#define NOTE1       200
#define NOTE2       300
#define NOTE4       600

#define QUARTER(note) \
    {.action = BUZZER_PLAY_TONE,  .duration=NOTE1,  .frequency=note}, \
    {.action = BUZZER_PLAY_QUIET, .duration=REST1,  .frequency=0}

#define HALF(note) \
    {.action = BUZZER_PLAY_TONE,  .duration=NOTE2,  .frequency=note}, \
    {.action = BUZZER_PLAY_QUIET, .duration=REST2,  .frequency=0}

#define WHOLE(note) \
    {.action = BUZZER_PLAY_TONE,  .duration=NOTE4,  .frequency=note}, \
    {.action = BUZZER_PLAY_QUIET, .duration=REST4,  .frequency=0}

#define FINI {.action = BUZZER_PLAY_DONE, .duration=0, .frequency=0}

/*---------------------------------------------------------------------------*/
/* Collection of sounds and tones                                            */
/*---------------------------------------------------------------------------*/

buzzer_play_t mary_had_a_little_lamb_sound [] = {

    QUARTER(  note_E  ),
    QUARTER(  note_D  ),
    QUARTER(  note_C  ),
    QUARTER(  note_D  ),
    QUARTER(  note_E  ),
    QUARTER(  note_E  ),
    HALF(     note_E  ),
    QUARTER(  note_D  ),
    QUARTER(  note_D  ),
    HALF(     note_D  ),
    QUARTER(  note_E  ),
    QUARTER(  note_G  ),
    HALF(     note_G  ),

    QUARTER(  note_E  ),
    QUARTER(  note_D  ),
    QUARTER(  note_C  ),
    QUARTER(  note_D  ),
    QUARTER(  note_E  ),
    QUARTER(  note_E  ),
    QUARTER(  note_E  ),
    QUARTER(  note_E  ),
    QUARTER(  note_D  ),
    QUARTER(  note_D  ),
    QUARTER(  note_E  ),
    QUARTER(  note_D  ),
    WHOLE(    note_C  ),
    FINI,
};

buzzer_play_t startup_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=700, .frequency=400},   // long buzz    700ms
    {.action = BUZZER_PLAY_DONE,  .duration=0,   .frequency=0},     // stop
};

buzzer_play_t two_beeps_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz
    {.action = BUZZER_PLAY_DONE,  .duration=0,   .frequency=0},     // stop
};

buzzer_play_t three_beeps_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200, .frequency=400},   // short buzz
    {.action = BUZZER_PLAY_DONE,  .duration=0,   .frequency=0},     // stop
};

buzzer_play_t test_sound [] = {

    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=564},   // short buzz   A

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=532},   // short buzz   A# 

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=505},   // short buzz   B  

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=482},   // short buzz   C
    
    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=450},   // short buzz   C#
    
    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=427},   // short buzz   D
    
    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=397},   // short buzz   D#
    
    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=380},   // short buzz   E
    
    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=345},   // short buzz   F

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=335},   // short buzz   F#

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=322},   // short buzz   G

    {.action = BUZZER_PLAY_QUIET, .duration=100, .frequency=0},     // short quiet 
    {.action = BUZZER_PLAY_TONE,  .duration=100, .frequency=300},   // short buzz   G#

    {.action = BUZZER_PLAY_DONE,  .duration=0,   .frequency=0},     // stop
};
