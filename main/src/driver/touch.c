#include "driver/touch.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "TOUCH_CONTROLLER";

esp_lcd_touch_handle_t touch_controller_init(void) {
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
  esp_lcd_panel_io_handle_t tp_io_handle = NULL;
  esp_lcd_touch_handle_t tp = NULL;

  // Configure SPI interface based on touch controller type
#if defined(CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610)
  esp_lcd_panel_io_spi_config_t tp_io_config =
      ESP_LCD_TOUCH_IO_SPI_STMPE610_CONFIG(EXAMPLE_PIN_NUM_TOUCH_CS);
#elif defined(CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_XPT2046)
  esp_lcd_panel_io_spi_config_t tp_io_config =
      ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(EXAMPLE_PIN_NUM_TOUCH_CS);
#else
  // No supported touch controller configured
  ESP_LOGW(TAG, "Touch enabled but no controller configured");
  return NULL;
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

  // Initialize the specific touch controller
#if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
  ESP_LOGI(TAG, "Initialize touch controller STMPE610");
  ESP_ERROR_CHECK(esp_lcd_touch_new_spi_stmpe610(tp_io_handle, &tp_cfg, &tp));
#elif CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_XPT2046
  ESP_LOGI(TAG, "Initialize touch controller XPT2046");
  ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));
#endif

  return tp;
#else
  // Touch not enabled
  return NULL;
#endif
}
