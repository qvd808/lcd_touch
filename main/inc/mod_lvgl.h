#ifndef __MOD_LVGL_H__
#define __MOD_LVGL_H__

#include "lvgl.h"
#include "mod_screen_controller.h"

void lvgl_task(void *arg);
void gesture_event_cb(lv_event_t *e);
void ui_init_gestures(lv_obj_t *scr);

#endif //__MOD_LVGL_H__
