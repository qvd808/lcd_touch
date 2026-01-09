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
}

void lvgl_task(void *arg) {

  /* Initialize display + LVGL */
  display_handle_t display = display_init();

  lv_display_t *lv_disp = mod_lvgl_init(&display);

  /* Touch init is optional here, but safe */
  touch_controller_init(lv_disp);

  /* Get active screen */
  lv_obj_t *scr = lv_display_get_screen_active(lv_disp);

  /* Set blue background */
  lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  /* Create label */
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "Hello LVGL - Swipe me!");
  lv_obj_center(label);

  /* ADD GESTURE DETECTION HERE */
  lv_obj_add_event_cb(scr, gesture_event_cb, LV_EVENT_GESTURE, label);

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
