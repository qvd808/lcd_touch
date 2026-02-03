#include "screen/home.h"
#include "mod_lvgl.h"
#include "mod_state.h"
#include <inttypes.h>
#include <stdint.h>

static lv_obj_t *steps_label = NULL;

static void home_screen_delete_cb(lv_event_t *e) { steps_label = NULL; }

void home_screen(lv_obj_t *scr) {
  /* Disable scrolling and scrollbar on main screen */
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

  /* Set dark background */
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  /* Time display */
  lv_obj_t *time_label = lv_label_create(scr);
  lv_label_set_text(time_label, "10:30");
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -60);

  /* Date display */
  lv_obj_t *date_label = lv_label_create(scr);
  lv_label_set_text(date_label, "Monday, Jan 9");
  lv_obj_set_style_text_color(date_label, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, -10);

  /* Status icons container */
  lv_obj_t *status_container = lv_obj_create(scr);
  lv_obj_remove_flag(status_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(status_container, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(status_container, 200, 40);
  lv_obj_align(status_container, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_bg_opa(status_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(status_container, 0, 0);
  lv_obj_set_style_pad_all(status_container, 0, 0);
  lv_obj_set_flex_flow(status_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(status_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* Battery icon */
  lv_obj_t *battery_label = lv_label_create(status_container);
  lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(battery_label, lv_color_hex(0x00FF00), 0);

  /* Bluetooth icon */
  lv_obj_t *bt_label = lv_label_create(status_container);
  lv_label_set_text(bt_label, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_color(bt_label, lv_color_hex(0x2196F3), 0);

  /* WiFi icon */
  lv_obj_t *wifi_label = lv_label_create(status_container);
  lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(wifi_label, lv_color_white(), 0);

  /* Quick stats container */
  lv_obj_t *stats_container = lv_obj_create(scr);
  lv_obj_remove_flag(stats_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(stats_container, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(stats_container, 220, 80);
  lv_obj_align(stats_container, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(stats_container, lv_color_hex(0x1A1A1A), 0);
  lv_obj_set_style_bg_opa(stats_container, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(stats_container, 15, 0);
  lv_obj_set_style_border_width(stats_container, 0, 0);
  lv_obj_set_flex_flow(stats_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(stats_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* Steps stat */
  lv_obj_t *steps_container = lv_obj_create(stats_container);
  lv_obj_remove_flag(steps_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(steps_container, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(steps_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(steps_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(steps_container, 0, 0);
  lv_obj_set_flex_flow(steps_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(steps_container, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *steps_icon = lv_label_create(steps_container);
  lv_label_set_text(steps_icon, LV_SYMBOL_SHUFFLE);
  lv_obj_set_style_text_color(steps_icon, lv_color_hex(0xFF6B6B), 0);

  steps_label = lv_label_create(steps_container);
  lv_label_set_text_fmt(steps_label, "%" PRIu32, mod_state_get()->steps);
  lv_obj_set_style_text_color(steps_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(steps_label, &lv_font_montserrat_14, 0);

  /* Heart rate stat */
  lv_obj_t *heart_container = lv_obj_create(stats_container);
  lv_obj_remove_flag(heart_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(heart_container, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(heart_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(heart_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(heart_container, 0, 0);
  lv_obj_set_flex_flow(heart_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(heart_container, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *heart_icon = lv_label_create(heart_container);
  lv_label_set_text(heart_icon, LV_SYMBOL_CHARGE);
  lv_obj_set_style_text_color(heart_icon, lv_color_hex(0xFF4081), 0);

  lv_obj_t *heart_value = lv_label_create(heart_container);
  lv_label_set_text(heart_value, "72 bpm");
  lv_obj_set_style_text_color(heart_value, lv_color_white(), 0);
  lv_obj_set_style_text_font(heart_value, &lv_font_montserrat_14, 0);

  /* Calories stat */
  lv_obj_t *cal_container = lv_obj_create(stats_container);
  lv_obj_remove_flag(cal_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(cal_container, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(cal_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(cal_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cal_container, 0, 0);
  lv_obj_set_flex_flow(cal_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cal_container, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *cal_icon = lv_label_create(cal_container);
  lv_label_set_text(cal_icon, LV_SYMBOL_WARNING);
  lv_obj_set_style_text_color(cal_icon, lv_color_hex(0xFFA726), 0);

  lv_obj_t *cal_value = lv_label_create(cal_container);
  lv_label_set_text(cal_value, "542");
  lv_obj_set_style_text_color(cal_value, lv_color_white(), 0);
  lv_obj_set_style_text_font(cal_value, &lv_font_montserrat_14, 0);

  /* ADD GESTURE DETECTION */
  ui_init_gestures(scr);
  lv_obj_add_event_cb(scr, home_screen_delete_cb, LV_EVENT_DELETE, NULL);
}
void home_update_steps(uint32_t steps) {
  lvgl_lock();
  if (steps_label) {
    lv_label_set_text_fmt(steps_label, "%" PRIu32, steps);
  }
  lvgl_unlock();
}
