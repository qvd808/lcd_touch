#ifndef __MOD_LVGL_H__
#define __MOD_LVGL_H__

#include "lvgl.h"

void lvgl_task(void *arg);
void gesture_event_cb(lv_event_t *e);
void ui_init_gestures(lv_obj_t *scr);
void ui_load_home_screen(void);
void ui_load_music_screen(void);

#endif //__MOD_LVGL_H__
