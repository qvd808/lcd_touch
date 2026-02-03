#ifndef __MOD_SCREEN_CONTROLLER_H__
#define __MOD_SCREEN_CONTROLLER_H__

#include "lvgl.h"

/**
 * @brief Function pointer for screen creation
 */
typedef void (*ui_screen_create_cb)(lv_obj_t *scr);

/**
 * @brief Screen definition
 */
typedef struct {
  ui_screen_create_cb create_cb;
  const char *name;
} ui_screen_t;

/**
 * @brief Initialize the screen controller with available screens
 */
void ui_controller_init(void);

/**
 * @brief Navigate to the next screen in the circular list
 */
void ui_controller_next(void);

/**
 * @brief Navigate to the previous screen in the circular list
 */
void ui_controller_prev(void);

/**
 * @brief Load the initial screen
 */
void ui_controller_load_initial(void);

/**
 * @brief Initialize gestures for a screen
 */
void ui_init_gestures(lv_obj_t *scr);

#endif // __MOD_SCREEN_CONTROLLER_H__
