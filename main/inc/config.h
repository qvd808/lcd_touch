#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "sdkconfig.h"

#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#include "esp_lcd_ili9341.h"
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#include "esp_lcd_gc9a01.h"
#endif

#if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
#include "esp_lcd_touch_stmpe610.h"
#elif CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_XPT2046
#include "esp_lcd_touch_xpt2046.h"
#endif

// Using SPI2 in the example
#define LCD_HOST SPI2_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK 6
#define EXAMPLE_PIN_NUM_MOSI 7
#define EXAMPLE_PIN_NUM_MISO 2
#define EXAMPLE_PIN_NUM_LCD_DC 5
#define EXAMPLE_PIN_NUM_LCD_RST 3
#define EXAMPLE_PIN_NUM_LCD_CS 4
#define EXAMPLE_PIN_NUM_BK_LIGHT 19
#define EXAMPLE_PIN_NUM_TOUCH_CS 20

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 320
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8

#define EXAMPLE_LVGL_DRAW_BUF_LINES                                            \
  20 // number of display lines in each draw buffer
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

#endif //__CONFIG_H__
