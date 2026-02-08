#pragma once

#include "esphome/core/defines.h"

#ifdef USE_DIAL_MENU_MEDIA_PLAYER

#include "dial_menu_controller.h"
#include "esphome/components/font/font.h"
#include "esphome/components/homeassistant_addon/homeassistant_media_player.h"
#include <string>

namespace esphome {
namespace dial_menu {

/**
 * @brief App that controls a Home Assistant media player
 * 
 * Features:
 * - Volume control with encoder rotation
 * - Play/Pause/Previous/Next controls
 * - Media info display (title, artist)
 * - Mute toggle
 */
class MediaPlayerApp : public DialApp {
 public:
  void set_controller(DialMenuController *controller) { this->controller_ = controller; }
  void set_media_player(homeassistant_addon::HomeassistantMediaPlayer *media_player) {
    this->media_player_ = media_player;
  }
  void set_volume_step(float step) { this->volume_step_ = step; }
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  void set_font_18(font::Font *font) { this->font_18_ = font; }

  // App lifecycle - called by DialMenuController
  void on_enter() override;
  void on_exit() override;
  void on_button_press() override;
  void on_encoder_rotate(int delta) override;

  // This app needs its own UI page
  bool needs_ui() const override { return true; }
  
  // Create the app-specific UI
  void create_app_ui() override;

 protected:
  void update_ui_();
  void update_state_display_();
  void update_media_info_();
  void update_volume_arc_();
  std::string get_state_text_();
  const char *get_state_icon_();

  homeassistant_addon::HomeassistantMediaPlayer *media_player_{nullptr};
  DialMenuController *controller_{nullptr};
  float volume_step_{0.05f};
  font::Font *font_14_{nullptr};
  font::Font *font_18_{nullptr};

  // UI elements
  lv_obj_t *page_{nullptr};
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
