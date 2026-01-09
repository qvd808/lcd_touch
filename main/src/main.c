/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * * * SPDX-License-Identifier: CC0-1.0
 */
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mod_lvgl.h"

static const char *TAG = "MAIN";

/* -------------------- app_main -------------------- */

void app_main(void) {
  ESP_LOGI("MAIN", "Starting simple LVGL screen");

  /* -------------------- LVGL Main Loop -------------------- */
  xTaskCreate(lvgl_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY,
              NULL);
}
