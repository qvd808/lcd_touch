
#ifndef __UI_H__
#define __UI_H__

#include "lvgl.h"

void lv_screen(lv_disp_t *disp);
void increment_time();
void update_time_display();
void set_time(uint8_t h, uint8_t m, uint8_t s);

#endif //__UI_H__
