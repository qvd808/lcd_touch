#include "hardware/spi.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "HAL_SPI";

#define PIN_SCLK EXAMPLE_PIN_NUM_SCLK
#define PIN_MOSI EXAMPLE_PIN_NUM_MOSI
#define PIN_MISO EXAMPLE_PIN_NUM_MISO

#define MAX_TRANSFER_SIZE EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t)

bool init_spi() {

  ESP_LOGI(TAG, "Initialize SPI bus");

  spi_bus_config_t buscfg = {
      .sclk_io_num = PIN_SCLK,
      .mosi_io_num = PIN_MOSI,
      .miso_io_num = PIN_MISO,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = MAX_TRANSFER_SIZE,
  };

  esp_err_t ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  return (ret == ESP_OK);
}
