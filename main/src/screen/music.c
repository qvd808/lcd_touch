#include "lvgl.h"
#include "screen/music.h"

/* Global variables for player state */
static lv_obj_t *play_pause_icon;
static lv_obj_t *progress_bar;
static lv_obj_t *current_time_label;
static lv_obj_t *album_art_container;
static bool is_playing = false;
static lv_timer_t *progress_timer = NULL;

/* Forward declarations */
static void play_pause_event_cb(lv_event_t *e);
static void backward_event_cb(lv_event_t *e);
static void forward_event_cb(lv_event_t *e);
static void progress_timer_cb(lv_timer_t *timer);

void music_screen(lv_obj_t *scr) {
  /* Disable scrolling */
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

  /* Gradient background */
  static lv_style_t bg_style;
  lv_style_init(&bg_style);
  lv_style_set_bg_color(&bg_style, lv_color_hex(0x0F0F0F));
  lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x1B1B1B));
  lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
  lv_style_set_bg_opa(&bg_style, LV_OPA_COVER);
  lv_obj_add_style(scr, &bg_style, 0);

  /* Album art container - reduced for more padding */
  album_art_container = lv_obj_create(scr);
  lv_obj_remove_flag(album_art_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(album_art_container, 100, 100);
  lv_obj_align(album_art_container, LV_ALIGN_TOP_MID, 0, 15);
  lv_obj_set_style_bg_color(album_art_container, lv_color_hex(0x1DB954), 0);
  lv_obj_set_style_radius(album_art_container, 8, 0);
  lv_obj_set_style_border_width(album_art_container, 0, 0);
  lv_obj_set_style_shadow_width(album_art_container, 15, 0);
  lv_obj_set_style_shadow_color(album_art_container, lv_color_hex(0x1DB954), 0);
  lv_obj_set_style_shadow_opa(album_art_container, LV_OPA_30, 0);

  /* Album icon */
  lv_obj_t *spotify_icon = lv_label_create(album_art_container);
  lv_label_set_text(spotify_icon, LV_SYMBOL_AUDIO);
  lv_obj_set_style_text_font(spotify_icon, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(spotify_icon, lv_color_white(), 0);
  lv_obj_center(spotify_icon);

  /* Song title */
  lv_obj_t *song_title = lv_label_create(scr);
  lv_label_set_text(song_title, "Summer Vibes");
  lv_obj_set_style_text_font(song_title, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_letter_space(song_title, 1, 0);
  lv_obj_set_style_text_color(song_title, lv_color_white(), 0);
  lv_obj_align(song_title, LV_ALIGN_TOP_MID, 0, 125);

  /* Artist name */
  lv_obj_t *artist_name = lv_label_create(scr);
  lv_label_set_text(artist_name, "Digital Dreams");
  lv_obj_set_style_text_font(artist_name, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(artist_name, lv_color_hex(0xB3B3B3), 0);
  lv_obj_set_style_text_opa(artist_name, LV_OPA_70, 0);
  lv_obj_align(artist_name, LV_ALIGN_TOP_MID, 0, 142);

  /* Progress container */
  lv_obj_t *progress_container = lv_obj_create(scr);
  lv_obj_remove_flag(progress_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(progress_container, 240, 20);
  lv_obj_align(progress_container, LV_ALIGN_TOP_MID, 0, 162);
  lv_obj_set_style_bg_opa(progress_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(progress_container, 0, 0);
  lv_obj_set_style_pad_all(progress_container, 0, 0);

  /* Progress bar */
  progress_bar = lv_bar_create(progress_container);
  lv_obj_set_size(progress_bar, 170, 4);
  lv_obj_align(progress_bar, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(progress_bar, 2, 0);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x404040), 0);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x1DB954), LV_PART_INDICATOR);
  lv_bar_set_range(progress_bar, 0, 225);
  lv_bar_set_value(progress_bar, 83, LV_ANIM_OFF);

  /* Time labels */
  current_time_label = lv_label_create(progress_container);
  lv_label_set_text(current_time_label, "1:23");
  lv_obj_set_style_text_font(current_time_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(current_time_label, lv_color_hex(0xB3B3B3), 0);
  lv_obj_align(current_time_label, LV_ALIGN_LEFT_MID, 0, 0);

  lv_obj_t *total_time_label = lv_label_create(progress_container);
  lv_label_set_text(total_time_label, "3:45");
  lv_obj_set_style_text_font(total_time_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(total_time_label, lv_color_hex(0xB3B3B3), 0);
  lv_obj_align(total_time_label, LV_ALIGN_RIGHT_MID, 0, 0);

  /* Controls container - bottom oriented with padding */
  lv_obj_t *controls_container = lv_obj_create(scr);
  lv_obj_remove_flag(controls_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(controls_container, 240, 40);
  lv_obj_align(controls_container, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_bg_opa(controls_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(controls_container, 0, 0);
  lv_obj_set_flex_flow(controls_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(controls_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* Backward button */
  lv_obj_t *backward_btn = lv_button_create(controls_container);
  lv_obj_set_size(backward_btn, 36, 36);
  lv_obj_set_style_bg_color(backward_btn, lv_color_hex(0x1A1A1A), 0);
  lv_obj_set_style_radius(backward_btn, 18, 0);
  lv_obj_add_event_cb(backward_btn, backward_event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *backward_icon = lv_label_create(backward_btn);
  lv_label_set_text(backward_icon, LV_SYMBOL_PREV);
  lv_obj_set_style_text_font(backward_icon, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(backward_icon, lv_color_hex(0xB3B3B3), 0);
  lv_obj_center(backward_icon);

  /* Play / Pause button */
  lv_obj_t *play_pause_btn = lv_button_create(controls_container);
  lv_obj_set_size(play_pause_btn, 40, 40);
  lv_obj_set_style_bg_color(play_pause_btn, lv_color_white(), 0);
  lv_obj_set_style_radius(play_pause_btn, 20, 0);
  lv_obj_add_event_cb(play_pause_btn, play_pause_event_cb, LV_EVENT_CLICKED, NULL);

  play_pause_icon = lv_label_create(play_pause_btn);
  lv_label_set_text(play_pause_icon, LV_SYMBOL_PLAY);
  lv_obj_set_style_text_font(play_pause_icon, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(play_pause_icon, lv_color_black(), 0);
  lv_obj_center(play_pause_icon);

  /* Forward button */
  lv_obj_t *forward_btn = lv_button_create(controls_container);
  lv_obj_set_size(forward_btn, 36, 36);
  lv_obj_set_style_bg_color(forward_btn, lv_color_hex(0x1A1A1A), 0);
  lv_obj_set_style_radius(forward_btn, 18, 0);
  lv_obj_add_event_cb(forward_btn, forward_event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *forward_icon = lv_label_create(forward_btn);
  lv_label_set_text(forward_icon, LV_SYMBOL_NEXT);
  lv_obj_set_style_text_font(forward_icon, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(forward_icon, lv_color_hex(0xB3B3B3), 0);
  lv_obj_center(forward_icon);
}

/* === Callbacks === */

static void play_pause_event_cb(lv_event_t *e) {
  is_playing = !is_playing;

  if (is_playing) {
    lv_label_set_text(play_pause_icon, LV_SYMBOL_PAUSE);
    if (!progress_timer)
      progress_timer = lv_timer_create(progress_timer_cb, 1000, NULL);
    else
      lv_timer_resume(progress_timer);
  } else {
    lv_label_set_text(play_pause_icon, LV_SYMBOL_PLAY);
    if (progress_timer)
      lv_timer_pause(progress_timer);
  }
}

static void backward_event_cb(lv_event_t *e) {
  int32_t v = lv_bar_get_value(progress_bar);
  v = LV_MAX(v - 10, 0);
  lv_bar_set_value(progress_bar, v, LV_ANIM_ON);
  lv_label_set_text_fmt(current_time_label, "%d:%02d", (int)(v / 60), (int)(v % 60));
}

static void forward_event_cb(lv_event_t *e) {
  int32_t v = lv_bar_get_value(progress_bar);
  int32_t max = lv_bar_get_max_value(progress_bar);
  v = LV_MIN(v + 10, max);
  lv_bar_set_value(progress_bar, v, LV_ANIM_ON);
  lv_label_set_text_fmt(current_time_label, "%d:%02d", (int)(v / 60), (int)(v % 60));
}

static void progress_timer_cb(lv_timer_t *timer) {
  if (!is_playing) return;

  int32_t v = lv_bar_get_value(progress_bar);
  int32_t max = lv_bar_get_max_value(progress_bar);

  if (v < max) {
    v++;
    lv_bar_set_value(progress_bar, v, LV_ANIM_OFF);
    lv_label_set_text_fmt(current_time_label, "%d:%02d", (int)(v / 60), (int)(v % 60));
  } else {
    is_playing = false;
    lv_label_set_text(play_pause_icon, LV_SYMBOL_PLAY);
    lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
    lv_label_set_text(current_time_label, "0:00");
    lv_timer_pause(progress_timer);
  }
}
