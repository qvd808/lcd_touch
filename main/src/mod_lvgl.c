#include "mod_lvgl.h"
#include "config.h"
#include "display/lv_display.h"
#include "draw/sw/lv_draw_sw.h"
#include "driver/display.h"
#include "driver/touch.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lv_init.h"
#include "misc/lv_event_private.h"
#include "misc/lv_types.h"
#include "widgets/label/lv_label.h"
#include <stdbool.h>
#include <sys/unistd.h>

// ################## PRIVATE VARIABLE ###############################
static const char *TAG = "MOD_LVGL";
// LVGL library is not thread-safe, this example will call LVGL APIs from
// different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

// ################## PRIVATE FUNCTION ###############################
/* -------------------- LVGL ↔ LCD callbacks -------------------- */

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                    esp_lcd_panel_io_event_data_t *edata,
                                    void *user_ctx) {
  lv_display_t *disp = (lv_display_t *)user_ctx;
  lv_display_flush_ready(disp);
  return false;
}

static void increase_lvgl_tick(void *arg) {
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                          uint8_t *px_map) {
  esp_lcd_panel_handle_t panel =
      (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

  int x1 = area->x1;
  int x2 = area->x2;
  int y1 = area->y1;
  int y2 = area->y2;

  lv_draw_sw_rgb565_swap(px_map, (x2 + 1 - x1) * (y2 + 1 - y1));

  esp_lcd_panel_draw_bitmap(panel, x1, y1, x2 + 1, y2 + 1, px_map);
}

static void example_lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data) {
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
  uint16_t touchpad_x[1] = {0};
  uint16_t touchpad_y[1] = {0};
  uint8_t touchpad_cnt = 0;

  esp_lcd_touch_handle_t touch_pad = lv_indev_get_user_data(indev);
  esp_lcd_touch_read_data(touch_pad);
  /* Get coordinates */
  bool touchpad_pressed = esp_lcd_touch_get_coordinates(
      touch_pad, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

  if (touchpad_pressed && touchpad_cnt > 0) {
    data->point.x = touchpad_x[0];
    data->point.y = touchpad_y[0];
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
#else
  (void)indev, (void)data;
#endif
}

static void mod_lv_init_input(lv_display_t *display,
                              esp_lcd_touch_handle_t tp) {
  if (tp == NULL) {
    ESP_LOGI(TAG, "TOUCH CONTROLLER NOT ENAVBLE - SKIP");
  }

  static lv_indev_t *indev;
  indev = lv_indev_create(); // Input device driver (Touch)
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_display(indev, display);
  lv_indev_set_user_data(indev, tp);
  lv_indev_set_read_cb(indev, example_lvgl_touch_cb);
}

/* -------------------- Public API -------------------- */

static lv_display_t *mod_lvgl_init(const display_handle_t *display) {
  ESP_LOGI(TAG, "Initialize LVGL");
  lv_init();

  lv_display_t *disp = lv_display_create(EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);

  /* Allocate draw buffers */
  size_t draw_buf_size =
      EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);

  void *buf1 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buf_size, 0);
  void *buf2 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buf_size, 0);
  assert(buf1 && buf2);

  lv_display_set_buffers(disp, buf1, buf2, draw_buf_size,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  /* Bind display hardware to LVGL */
  lv_display_set_user_data(disp, display->panel_handle);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);

  /* LVGL tick timer */
  const esp_timer_create_args_t tick_args = {.callback = increase_lvgl_tick,
                                             .name = "lvgl_tick"};

  esp_timer_handle_t tick_timer;
  ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

  /* Register flush-ready callback */
  const esp_lcd_panel_io_callbacks_t cbs = {
      .on_color_trans_done = notify_lvgl_flush_ready,
  };

  ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(display->io_handle,
                                                            &cbs, disp));

  return disp;
}

void gesture_event_cb(lv_event_t *e) {
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

  switch (dir) {
  case LV_DIR_LEFT:
    ESP_LOGI("GESTURE", "Swiped LEFT");
    lv_label_set_text(e->user_data, "Swiped LEFT");
    break;
  case LV_DIR_RIGHT:
    ESP_LOGI("GESTURE", "Swiped RIGHT");
    lv_label_set_text(e->user_data, "Swiped RIGHT");
    break;
  case LV_DIR_TOP:
    ESP_LOGI("GESTURE", "Swiped UP");
    lv_label_set_text(e->user_data, "Swiped UP");
    break;
  case LV_DIR_BOTTOM:
    ESP_LOGI("GESTURE", "Swiped DOWN");
    lv_label_set_text(e->user_data, "Swiped DOWN");
    break;
  case LV_DIR_NONE:
  case LV_DIR_HOR:
  case LV_DIR_VER:
  case LV_DIR_ALL:
    // Not specific directional gestures, ignore
    break;
  }
#endif
}

static void home_screen(lv_obj_t *scr) {
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
  lv_obj_set_size(steps_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(steps_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(steps_container, 0, 0);
  lv_obj_set_flex_flow(steps_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(steps_container, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *steps_icon = lv_label_create(steps_container);
  lv_label_set_text(steps_icon, LV_SYMBOL_SHUFFLE);
  lv_obj_set_style_text_color(steps_icon, lv_color_hex(0xFF6B6B), 0);

  lv_obj_t *steps_value = lv_label_create(steps_container);
  lv_label_set_text(steps_value, "8,432");
  lv_obj_set_style_text_color(steps_value, lv_color_white(), 0);
  lv_obj_set_style_text_font(steps_value, &lv_font_montserrat_14, 0);

  /* Heart rate stat */
  lv_obj_t *heart_container = lv_obj_create(stats_container);
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

  /* Swipe indicator */
  lv_obj_t *swipe_hint = lv_label_create(scr);
  lv_label_set_text(swipe_hint, "< Swipe >");
  lv_obj_set_style_text_color(swipe_hint, lv_color_hex(0x555555), 0);
  lv_obj_align(swipe_hint, LV_ALIGN_BOTTOM_MID, 0, -5);

  /* ADD GESTURE DETECTION */
  lv_obj_add_event_cb(scr, gesture_event_cb, LV_EVENT_GESTURE, time_label);
}

void lvgl_task(void *arg) {

  /* Initialize display + LVGL */
  display_handle_t display = display_init();

  lv_display_t *lv_disp = mod_lvgl_init(&display);

  /* Touch init is optional here, but safe */
  esp_lcd_touch_handle_t touch_handle = touch_controller_init();

  mod_lv_init_input(lv_disp, touch_handle);

  /* Get active screen */
  lv_obj_t *scr = lv_display_get_screen_active(lv_disp);
  home_screen(scr);

  ESP_LOGI(TAG, "Starting LVGL task");
  uint32_t time_till_next_ms = 0;
  while (1) {
    _lock_acquire(&lvgl_api_lock);
    time_till_next_ms = lv_timer_handler();
    _lock_release(&lvgl_api_lock);
    // in case of triggering a task watch dog time out
    time_till_next_ms = MAX(time_till_next_ms, LVGL_TASK_MIN_DELAY_MS);
    // in case of lvgl display not ready yet
    time_till_next_ms = MIN(time_till_next_ms, LVGL_TASK_MAX_DELAY_MS);
    usleep(1000 * time_till_next_ms);
  }
}
