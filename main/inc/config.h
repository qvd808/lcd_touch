#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "sdkconfig.h"

#if CONFIG_LCD_CONTROLLER_ILI9341
#include "esp_lcd_ili9341.h"
#elif CONFIG_LCD_CONTROLLER_GC9A01
#include "esp_lcd_gc9a01.h"
#endif

#if CONFIG_LCD_TOUCH_CONTROLLER_STMPE610
#include "esp_lcd_touch_stmpe610.h"
#elif CONFIG_LCD_TOUCH_CONTROLLER_XPT2046
#include "esp_lcd_touch_xpt2046.h"
#endif

// Using SPI2 in the example
#define LCD_HOST SPI2_HOST

#define CONSTANT_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define CONSTANT_LCD_BK_LIGHT_ON_LEVEL 1
#define CONSTANT_LCD_BK_LIGHT_OFF_LEVEL !CONSTANT_LCD_BK_LIGHT_ON_LEVEL
#if CONFIG_IDF_TARGET_ESP32
#define CONSTANT_PIN_NUM_SCLK 18
#define CONSTANT_PIN_NUM_MOSI 23
#define CONSTANT_PIN_NUM_MISO 19
#define CONSTANT_PIN_NUM_LCD_DC 2
#define CONSTANT_PIN_NUM_LCD_RST 4
#define CONSTANT_PIN_NUM_LCD_CS 5
#define CONSTANT_PIN_NUM_BK_LIGHT 15
#define CONSTANT_PIN_NUM_TOUCH_CS 21
#elif CONFIG_IDF_TARGET_ESP32C6
#define CONSTANT_PIN_NUM_SCLK 6
#define CONSTANT_PIN_NUM_MOSI 7
#define CONSTANT_PIN_NUM_MISO 2
#define CONSTANT_PIN_NUM_LCD_DC 5
#define CONSTANT_PIN_NUM_LCD_RST 3
#define CONSTANT_PIN_NUM_LCD_CS 4
#define CONSTANT_PIN_NUM_BK_LIGHT 19
#define CONSTANT_PIN_NUM_TOUCH_CS 20
#endif

// The pixel number in horizontal and vertical
#if CONFIG_LCD_CONTROLLER_ILI9341
#define CONSTANT_LCD_H_RES 240
#define CONSTANT_LCD_V_RES 320
#elif CONFIG_LCD_CONTROLLER_GC9A01
#define CONSTANT_LCD_H_RES 240
#define CONSTANT_LCD_V_RES 240
#endif
// Bit number used to represent command and parameter
#define CONSTANT_LCD_CMD_BITS 8
#define CONSTANT_LCD_PARAM_BITS 8

#define CONSTANT_LVGL_DRAW_BUF_LINES                                            \
  20 // number of display lines in each draw buffer
#define CONSTANT_LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

// Bluetooth GATT Service and Characteristic UUIDs
#define GATT_SVR_SVC_ALERT_UUID 0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID 0x2A47
#define GATT_SVR_CHR_NEW_ALERT 0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID 0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID 0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT 0x2A44

// Maximum number of characteristics with the notify flag
#define MAX_NOTIFY 5

#define CONSTANT_IO_TYPE 3

#endif //__CONFIG_H__
