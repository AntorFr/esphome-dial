#pragma once

#ifdef USE_DIAL_MENU_MEDIA_PLAYER

#include "esphome/core/component.h"
#include "esphome/components/lvgl/lvgl_esphome.h"
#include "esphome/components/font/font.h"
#include "esphome/components/homeassistant_media_player/homeassistant_media_player.h"
#include <functional>
#include <string>

namespace esphome {
namespace dial_menu {

class DialMenuController;

class MediaPlayerApp {
 public:
  MediaPlayerApp(DialMenuController *controller) : controller_(controller) {}

  void set_name(const std::string &name) { this->name_ = name; }
  void set_color(uint32_t color) { this->color_ = color; }
  void set_media_player(homeassistant_media_player::HomeassistantMediaPlayer *media_player) {
    this->media_player_ = media_player;
  }
  void set_volume_step(float step) { this->volume_step_ = step; }
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  void set_font_18(font::Font *font) { this->font_18_ = font; }

  const std::string &get_name() const { return this->name_; }
  uint32_t get_color() const { return this->color_; }

  void create_ui(lv_obj_t *parent);
  void destroy_ui();
  void update_ui();

  // User interactions
  void on_encoder_rotate(int direction);
  void on_button_click();

 protected:
  void update_state_display_();
  void update_media_info_();
  void update_volume_arc_();
  std::string get_state_text_();
  const char *get_state_icon_();

  DialMenuController *controller_;
  std::string name_;
  uint32_t color_{0xFFFFFF};

  homeassistant_media_player::HomeassistantMediaPlayer *media_player_{nullptr};
  float volume_step_{0.05f};
  font::Font *font_14_{nullptr};
  font::Font *font_18_{nullptr};

  // UI elements
  lv_obj_t *container_{nullptr};
  lv_obj_t *volume_arc_{nullptr};
  lv_obj_t *title_label_{nullptr};
  lv_obj_t *artist_label_{nullptr};
  lv_obj_t *state_label_{nullptr};
  lv_obj_t *volume_label_{nullptr};

  // Control buttons
  lv_obj_t *btn_prev_{nullptr};
  lv_obj_t *btn_play_{nullptr};
  lv_obj_t *btn_next_{nullptr};
  lv_obj_t *btn_prev_label_{nullptr};
  lv_obj_t *btn_play_label_{nullptr};
  lv_obj_t *btn_next_label_{nullptr};

  // Current button selection (0=prev, 1=play/pause, 2=next)
  int selected_button_{1};

  // Volume debounce
  float pending_volume_{-1.0f};
  uint32_t last_volume_change_{0};
  static constexpr uint32_t VOLUME_DEBOUNCE_MS = 500;
};

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_MEDIA_PLAYER
