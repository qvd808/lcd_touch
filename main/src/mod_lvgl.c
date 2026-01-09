#include "mod_lvgl.h"
#include "config.h"
#include "display/lv_display.h"
#include "draw/sw/lv_draw_sw.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lv_init.h"
#include "misc/lv_types.h"
#include <stdbool.h>

static const char *TAG = "MOD_LVGL";

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

lv_display_t *mod_lvgl_init(const display_handle_t *display) {
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
