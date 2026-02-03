#ifndef __MUSIC_H__
#define __MUSIC_H__

#include "lvgl.h"

void music_screen(lv_obj_t *scr);
void music_update_progress(uint32_t progress_sec);

#endif //__MUSIC_H__
