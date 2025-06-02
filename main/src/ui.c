#include "ui.h"
#include <stdio.h>

static uint8_t hours = 0;
static uint8_t minutes = 0;
static uint8_t seconds = 0;
static lv_obj_t *time_label = NULL; // Keep reference to the label

void lv_screen(lv_disp_t *disp) {
  lv_obj_t *active_scr = lv_display_get_screen_active(disp);
  lv_obj_set_style_bg_color(active_scr, lv_color_hex(0x003a57), LV_PART_MAIN);

  // Create label only once and store reference
  time_label = lv_label_create(active_scr);

  lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);

  // Set initial time display
  update_time_display();
}

void update_time_display() {
  if (time_label != NULL) {
	static char time_str[15]; // "HH:MM:SS" + null terminator
	sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
	lv_label_set_text(time_label, time_str);
  }
}

void increment_time() {
  seconds++;

  // Handle rollover
  if (seconds >= 60) {
	seconds = 0;
	minutes++;
	
	if (minutes >= 60) {
	  minutes = 0;
	  hours++;

	  if (hours >= 24) {
		hours = 0;
	  }
	}
  }

  // Update the display
  update_time_display();
}

// Optional: Function to set initial time
void set_time(uint8_t h, uint8_t m, uint8_t s) {
  hours = h;
  minutes = m;
  seconds = s;
  update_time_display();
}
