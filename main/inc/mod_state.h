#ifndef __MOD_STATE_H__
#define __MOD_STATE_H__

#include <stdint.h>

typedef struct {
    uint32_t steps;
    uint32_t music_progress;
    char music_song[64];
    char music_artist[64];
    uint32_t music_duration;
} app_state_t;

/**
 * @brief Get the pointer to the global application state
 */
app_state_t* mod_state_get(void);

/**
 * @brief Initialize the global state with default values
 */
void mod_state_init(void);

#endif // __MOD_STATE_H__
