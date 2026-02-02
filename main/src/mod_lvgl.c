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
#include <sys/param.h>
#include <sys/unistd.h>
#include "screen/music.h"
#include "screen/home.h"

// ################## PRIVATE VARIABLE ###############################
static const char *TAG = "MOD_LVGL";
// LVGL library is not thread-safe, this example will call LVGL APIs from
// different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

static lv_display_rotation_t current_rotation = LV_DISPLAY_ROTATION_90;

static void example_lvgl_port_update_callback(lv_display_t *disp) {
  esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
  lv_display_rotation_t rotation = lv_display_get_rotation(disp);

  switch (rotation) {
  case LV_DISPLAY_ROTATION_0:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, true, false);
    break;
  case LV_DISPLAY_ROTATION_90:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, true);
    break;
  case LV_DISPLAY_ROTATION_180:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, false, true);
    break;
  case LV_DISPLAY_ROTATION_270:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, false);
    break;
  }
}

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
  example_lvgl_port_update_callback(disp);
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

  lv_display_set_rotation(disp, current_rotation);

  return disp;
}


void ui_init_gestures(lv_obj_t *scr) {
  lv_obj_add_event_cb(scr, gesture_event_cb, LV_EVENT_GESTURE, NULL);
}

void gesture_event_cb(lv_event_t *e) {
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

  switch (dir) {
  case LV_DIR_LEFT:
    ui_controller_next();
    break;
  case LV_DIR_RIGHT:
    ui_controller_prev();
    break;
  default:
    break;
  }
#endif
}


void lvgl_task(void *arg) {

  /* Initialize display + LVGL */
  display_handle_t display = display_init();

  lv_display_t *lv_disp = mod_lvgl_init(&display);

  /* Touch init is optional here, but safe */
  esp_lcd_touch_handle_t touch_handle = touch_controller_init();

  mod_lv_init_input(lv_disp, touch_handle);

  /* Initialize screen controller and load initial screen */
  ui_controller_init();
  ui_controller_load_initial();

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
