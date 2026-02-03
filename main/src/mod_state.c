#include "mod_state.h"
#include <string.h>

static app_state_t global_state;

void mod_state_init(void) {
    global_state.steps = 0;
    global_state.music_progress = 0;
    global_state.music_duration = 0;
    strncpy(global_state.music_song, "No Song", sizeof(global_state.music_song));
    strncpy(global_state.music_artist, "---", sizeof(global_state.music_artist));
}

app_state_t* mod_state_get(void) {
    return &global_state;
}
