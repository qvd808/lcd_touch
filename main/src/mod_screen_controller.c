#include "mod_screen_controller.h"
#include "screen/home.h"
#include "screen/music.h"
#include "esp_log.h"

static const char *TAG = "SCREEN_CTRL";

static const ui_screen_t screens[] = {
    {.create_cb = home_screen, .name = "Home"},
    {.create_cb = music_screen, .name = "Music"},
};

#define SCREEN_COUNT (sizeof(screens) / sizeof(ui_screen_t))

static int current_index = 0;

void ui_controller_init(void) {
  ESP_LOGI(TAG, "Screen controller initialized with %d screens", SCREEN_COUNT);
  current_index = 0;
}

void ui_controller_load_initial(void) {
  lv_obj_t *scr = lv_display_get_screen_active(NULL);
  if (screens[current_index].create_cb) {
    screens[current_index].create_cb(scr);
  }
}

static void load_screen_with_anim(int index, lv_screen_load_anim_t anim) {
  ESP_LOGI(TAG, "Loading screen: %s (index %d)", screens[index].name, index);
  
  lv_obj_t *new_scr = lv_obj_create(NULL);
  if (screens[index].create_cb) {
    screens[index].create_cb(new_scr);
  }
  
  lv_screen_load_anim(new_scr, anim, 300, 0, true);
  current_index = index;
}

void ui_controller_next(void) {
  int next_index = (current_index + 1) % SCREEN_COUNT;
  load_screen_with_anim(next_index, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

void ui_controller_prev(void) {
  int prev_index = (current_index - 1 + SCREEN_COUNT) % SCREEN_COUNT;
  load_screen_with_anim(prev_index, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}
