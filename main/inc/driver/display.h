#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "esp_lcd_types.h"

typedef struct {
  esp_lcd_panel_handle_t panel_handle;
  esp_lcd_panel_io_handle_t io_handle;
} display_handle_t;

display_handle_t display_init(void);

#endif //__DISPLAY_H__
