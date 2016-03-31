/* 
 *  Copyright (c) 2016 Robin Callender. All Rights Reserved.
 */
#ifndef BUZZER_H__
#define BUZZER_H__

typedef enum {
    BUZZER_PLAY_DONE = 0,
    BUZZER_PLAY_TONE,
    BUZZER_PLAY_QUIET,
} buzzer_play_action_t;

typedef struct {
    buzzer_play_action_t  action;    // see enum above
    uint16_t              duration;  // in milliseconds
} buzzer_play_t;

void     buzzer_init(void);
uint32_t buzzer_play(buzzer_play_t * playlist);
void     buzzer_stop(void);

#endif /* BUZZER_H__ */