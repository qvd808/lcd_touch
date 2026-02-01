#include "driver/display.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <sys/unistd.h>

static const char *TAG = "DRIVER_DISPLAY";

display_handle_t display_init(void) {

  init_gpio();
  init_spi();

  /*
   * Initalize the panel for the display
   */
  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
      .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
      .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
      .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
      .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
      .spi_mode = 0,
      .trans_queue_depth = 10,
  };
  // Attach the LCD to the SPI bus
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                           &io_config, &io_handle));

  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
      .bits_per_pixel = 16,
  };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
  ESP_LOGI(TAG, "Install ILI9341 panel driver");
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
  ESP_LOGI(TAG, "Install GC9A01 panel driver");
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
#endif

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#if CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

  // user can flush pre-defined pattern to the screen before we turn on the
  // screen or backlight
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  ESP_LOGI(TAG, "Turn on LCD backlight");
  gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

  display_handle_t display_handle = {.panel_handle = panel_handle,
                                     .io_handle = io_handle};

  return display_handle;
}
