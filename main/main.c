/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_display.h"
#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <unistd.h>

static const char *TAG = "example";

// LVGL library is not thread-safe, this example will call LVGL APIs from
// different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

extern void example_lvgl_demo_ui(lv_disp_t *disp);

static void example_lvgl_port_task(void *arg) {
  ESP_LOGI(TAG, "Starting LVGL task");
  uint32_t time_till_next_ms = 0;
  while (1) {
    _lock_acquire(&lvgl_api_lock);
    time_till_next_ms = lv_timer_handler();
    _lock_release(&lvgl_api_lock);
    // in case of triggering a task watch dog time out
    time_till_next_ms = MAX(time_till_next_ms, EXAMPLE_LVGL_TASK_MIN_DELAY_MS);
    // in case of lvgl display not ready yet
    time_till_next_ms = MIN(time_till_next_ms, EXAMPLE_LVGL_TASK_MAX_DELAY_MS);
    usleep(1000 * time_till_next_ms);
  }
}

#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
static void example_lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data) {
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
}
#endif

void app_main(void) {
  lv_display_t *display = display_init();

#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
  esp_lcd_panel_io_handle_t tp_io_handle = NULL;
  esp_lcd_panel_io_spi_config_t tp_io_config =
#ifdef CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
      ESP_LCD_TOUCH_IO_SPI_STMPE610_CONFIG(EXAMPLE_PIN_NUM_TOUCH_CS);
#elif CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_XPT2046
      ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(EXAMPLE_PIN_NUM_TOUCH_CS);
#endif
  // Attach the TOUCH to the SPI bus
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                           &tp_io_config, &tp_io_handle));

  esp_lcd_touch_config_t tp_cfg = {
      .x_max = EXAMPLE_LCD_H_RES,
      .y_max = EXAMPLE_LCD_V_RES,
      .rst_gpio_num = -1,
      .int_gpio_num = -1,
      .flags =
          {
              .swap_xy = 0,
              .mirror_x = 0,
              .mirror_y = CONFIG_EXAMPLE_LCD_MIRROR_Y,
          },
  };
  esp_lcd_touch_handle_t tp = NULL;

#if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
  ESP_LOGI(TAG, "Initialize touch controller STMPE610");
  ESP_ERROR_CHECK(esp_lcd_touch_new_spi_stmpe610(tp_io_handle, &tp_cfg, &tp));
#elif CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_XPT2046
  ESP_LOGI(TAG, "Initialize touch controller XPT2046");
  ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));
#endif

  static lv_indev_t *indev;
  indev = lv_indev_create(); // Input device driver (Touch)
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_display(indev, display);
  lv_indev_set_user_data(indev, tp);
  lv_indev_set_read_cb(indev, example_lvgl_touch_cb);
#endif

  // Create a task to for display
  ESP_LOGI(TAG, "Create LVGL task");
  xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE,
              NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);

  ESP_LOGI(TAG, "Display LVGL Meter Widget");
  // Lock the mutex due to the LVGL APIs are not thread-safe
  _lock_acquire(&lvgl_api_lock);
  example_lvgl_demo_ui(display);
  _lock_release(&lvgl_api_lock);
}
