#include "hardware/gpio.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "HAL_GPIO";

#define BACK_LIGHT_PIN EXAMPLE_PIN_NUM_BK_LIGHT

bool init_gpio() {
  ESP_LOGI(TAG, "Turn off LCD backlight");

  gpio_config_t bk_gpio_config = {.mode = GPIO_MODE_OUTPUT,
                                  .pin_bit_mask = 1ULL << BACK_LIGHT_PIN};

  esp_err_t ret = gpio_config(&bk_gpio_config);
  return (ret == ESP_OK);
}
