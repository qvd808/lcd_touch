/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "config.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "lvgl_display.h"
#include "mod_wifi.h"
#include "nvs_flash.h"
#include "touch_controller.h"
#include "ui.h"
#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>

static const char *TAG = "MAIN";

// LVGL library is not thread-safe, this example will call LVGL APIs from
// different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

static void example_lvgl_port_task(void *arg) {
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

// Task to update time every second
static void time_update_task(void *arg) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second

    // Protect LVGL API calls with mutex
    _lock_acquire(&lvgl_api_lock);
    increment_time(); // This will update the display
    _lock_release(&lvgl_api_lock);
  }
}

void app_main(void) {
  // Init hardware
  lv_display_t *display = display_init();
  touch_controller_init(display);
  // Initialize the screen once
  _lock_acquire(&lvgl_api_lock);
  lv_screen(display);
  set_time(12, 30, 45); // Set initial time (12:30:45)
  _lock_release(&lvgl_api_lock);

  // Create LVGL task
  xTaskCreate(example_lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL,
              LVGL_TASK_PRIORITY, NULL);
  // Create time update task
  xTaskCreate(time_update_task, "TIME", 2048, NULL, LVGL_TASK_PRIORITY - 1,

              NULL);
}
