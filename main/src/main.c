/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * * * SPDX-License-Identifier: CC0-1.0
 */
#include "bluetooth.h"
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "mod_lvgl.h"
#include "nvs_flash.h"
#include "scli.h"
#include "sdkconfig.h"
#include "util.h"

static const char *TAG = "MAIN";

/* -------------------- app_main -------------------- */

void app_main(void) {
  int rc;
  ESP_LOGI(TAG, "Starting app_main entry");

  /* Initialize NVS — it is used to store PHY calibration data */
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // bluetooth_start();
  xTaskCreatePinnedToCore(bluetooth_main_task, "bluetooth",
                          CONFIG_NIMBLE_TASK_STACK_SIZE, NULL,
                          (configMAX_PRIORITIES - 4), NULL,
                          CONFIG_NIMBLE_PINNED_TO_CORE);

  /* -------------------- LVGL Main Loop -------------------- */
  xTaskCreate(lvgl_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY,
              NULL);

  /* Initialize command line interface to accept input from user */
  rc = scli_init();
  if (rc != ESP_OK) {
    ESP_LOGE(TAG, "scli_init() failed");
  }
}
