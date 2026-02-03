#ifndef __MOD_LVGL_H__
#define __MOD_LVGL_H__

#include "lvgl.h"
#include "mod_screen_controller.h"

void lvgl_task(void *arg);

/* Thread-safety helpers for LVGL */
void lvgl_lock(void);
void lvgl_unlock(void);

#endif //__MOD_LVGL_H__
