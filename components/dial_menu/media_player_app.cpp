#include "esphome/core/defines.h"

#ifdef USE_DIAL_MENU_MEDIA_PLAYER

#include "media_player_app.h"
#include "dial_menu_controller.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "media_player_app";

// LV_SYMBOL definitions
#define SYMBOL_PLAY "\xEF\x81\x8B"       // 
#define SYMBOL_PAUSE "\xEF\x81\x8C"      // 
#define SYMBOL_PREV "\xEF\x81\x88"       // 
#define SYMBOL_NEXT "\xEF\x81\x91"       // 
#define SYMBOL_STOP "\xEF\x81\x8D"       // 
#define SYMBOL_VOLUME_UP "\xEF\x80\xA8"  // 
#define SYMBOL_MUTE "\xEF\x80\xA6"       // 

void MediaPlayerApp::create_app_ui() {
  ESP_LOGD(TAG, "Creating MediaPlayerApp UI for '%s'", this->name_.c_str());

  // Main container
  this->container_ = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(this->container_);
  lv_obj_set_size(this->container_, 240, 240);
  lv_obj_center(this->container_);
  lv_obj_set_style_bg_color(this->container_, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(this->container_, LV_OPA_COVER, 0);

  // Volume arc (background)
  this->volume_arc_ = lv_arc_create(this->container_);
  lv_obj_set_size(this->volume_arc_, 230, 230);
  lv_obj_center(this->volume_arc_);
  lv_arc_set_rotation(this->volume_arc_, 135);
  lv_arc_set_bg_angles(this->volume_arc_, 0, 270);
  lv_arc_set_range(this->volume_arc_, 0, 100);
  lv_arc_set_value(this->volume_arc_, 0);
  lv_obj_remove_style(this->volume_arc_, nullptr, LV_PART_KNOB);
  lv_obj_clear_flag(this->volume_arc_, LV_OBJ_FLAG_CLICKABLE);

  // Arc styling
  lv_obj_set_style_arc_color(this->volume_arc_, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_arc_width(this->volume_arc_, 8, LV_PART_MAIN);
  lv_obj_set_style_arc_color(this->volume_arc_, lv_color_hex(this->color_), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(this->volume_arc_, 8, LV_PART_INDICATOR);

  // State/source label (top)
  this->state_label_ = lv_label_create(this->container_);
  lv_obj_set_style_text_font(this->state_label_, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(this->state_label_, lv_color_hex(0x888888), 0);
  lv_obj_align(this->state_label_, LV_ALIGN_TOP_MID, 0, 35);
  lv_label_set_text(this->state_label_, "");

  // Media title (center-top)
  this->title_label_ = lv_label_create(this->container_);
  lv_obj_set_style_text_font(this->title_label_, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(this->title_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_width(this->title_label_, 180);
  lv_label_set_long_mode(this->title_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_align(this->title_label_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(this->title_label_, LV_ALIGN_CENTER, 0, -35);
  lv_label_set_text(this->title_label_, "");

  // Media artist (center)
  this->artist_label_ = lv_label_create(this->container_);
  lv_obj_set_style_text_font(this->artist_label_, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(this->artist_label_, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_width(this->artist_label_, 160);
  lv_label_set_long_mode(this->artist_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_align(this->artist_label_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(this->artist_label_, LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text(this->artist_label_, "");

  // Volume label (center-bottom)
  this->volume_label_ = lv_label_create(this->container_);
  lv_obj_set_style_text_font(this->volume_label_, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(this->volume_label_, lv_color_hex(this->color_), 0);
  lv_obj_align(this->volume_label_, LV_ALIGN_CENTER, 0, 15);
  lv_label_set_text(this->volume_label_, "");

  // Control buttons container
  lv_obj_t *btn_container = lv_obj_create(this->container_);
  lv_obj_remove_style_all(btn_container);
  lv_obj_set_size(btn_container, 180, 50);
  lv_obj_align(btn_container, LV_ALIGN_CENTER, 0, 55);

  // Previous button (left)
  this->btn_prev_ = lv_btn_create(btn_container);
  lv_obj_set_size(this->btn_prev_, 50, 50);
  lv_obj_align(this->btn_prev_, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_radius(this->btn_prev_, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(this->btn_prev_, lv_color_hex(0x333333), 0);
  this->btn_prev_label_ = lv_label_create(this->btn_prev_);
  lv_label_set_text(this->btn_prev_label_, SYMBOL_PREV);
  lv_obj_set_style_text_font(this->btn_prev_label_, &lv_font_montserrat_18, 0);
  lv_obj_center(this->btn_prev_label_);

  // Play/Pause button (center)
  this->btn_play_ = lv_btn_create(btn_container);
  lv_obj_set_size(this->btn_play_, 50, 50);
  lv_obj_align(this->btn_play_, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(this->btn_play_, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(this->btn_play_, lv_color_hex(this->color_), 0);
  this->btn_play_label_ = lv_label_create(this->btn_play_);
  lv_label_set_text(this->btn_play_label_, SYMBOL_PLAY);
  lv_obj_set_style_text_font(this->btn_play_label_, &lv_font_montserrat_18, 0);
  lv_obj_center(this->btn_play_label_);

  // Next button (right)
  this->btn_next_ = lv_btn_create(btn_container);
  lv_obj_set_size(this->btn_next_, 50, 50);
  lv_obj_align(this->btn_next_, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_style_radius(this->btn_next_, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(this->btn_next_, lv_color_hex(0x333333), 0);
  this->btn_next_label_ = lv_label_create(this->btn_next_);
  lv_label_set_text(this->btn_next_label_, SYMBOL_NEXT);
  lv_obj_set_style_text_font(this->btn_next_label_, &lv_font_montserrat_18, 0);
  lv_obj_center(this->btn_next_label_);

  // Set initial button selection visual
  this->selected_button_ = 1;  // Play/pause selected by default
  lv_obj_set_style_outline_width(this->btn_play_, 2, 0);
  lv_obj_set_style_outline_color(this->btn_play_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_outline_pad(this->btn_play_, 3, 0);
}

void MediaPlayerApp::on_exit() {
  if (this->container_ != nullptr) {
    lv_obj_del(this->container_);
    this->container_ = nullptr;
    this->volume_arc_ = nullptr;
    this->title_label_ = nullptr;
    this->artist_label_ = nullptr;
    this->state_label_ = nullptr;
    this->volume_label_ = nullptr;
    this->btn_prev_ = nullptr;
    this->btn_play_ = nullptr;
    this->btn_next_ = nullptr;
  }
}

void MediaPlayerApp::on_enter() {
  // Create the UI when entering the app
  this->create_app_ui();
  
  // Register state callback
  if (this->media_player_ != nullptr) {
    this->media_player_->add_on_state_callback([this]() {
      this->update_ui_();
    });
  }

  // Initial UI update
  this->update_ui_();
}

void MediaPlayerApp::update_ui_() {
  if (this->container_ == nullptr || this->media_player_ == nullptr) {
    return;
  }

  // Check for pending volume change
  if (this->pending_volume_ >= 0 && (millis() - this->last_volume_change_) >= VOLUME_DEBOUNCE_MS) {
    ESP_LOGD(TAG, "Sending debounced volume: %.2f", this->pending_volume_);
    this->media_player_->set_volume(this->pending_volume_);
    this->pending_volume_ = -1.0f;
  }

  this->update_state_display_();
  this->update_media_info_();
  this->update_volume_arc_();
}

void MediaPlayerApp::update_state_display_() {
  if (this->state_label_ == nullptr || this->media_player_ == nullptr) return;

  std::string state_text = this->get_state_text_();
  
  // Add source if available
  const std::string &source = this->media_player_->get_source();
  if (!source.empty()) {
    if (!state_text.empty()) {
      state_text += " • ";
    }
    state_text += source;
  }

  lv_label_set_text(this->state_label_, state_text.c_str());

  // Update play/pause button icon
  if (this->btn_play_label_ != nullptr) {
    auto state = this->media_player_->get_state();
    if (state == homeassistant_addon::MediaPlayerState::PLAYING) {
      lv_label_set_text(this->btn_play_label_, SYMBOL_PAUSE);
    } else {
      lv_label_set_text(this->btn_play_label_, SYMBOL_PLAY);
    }
  }
}

void MediaPlayerApp::update_media_info_() {
  if (this->media_player_ == nullptr) return;

  if (this->title_label_ != nullptr) {
    const std::string &title = this->media_player_->get_media_title();
    if (title.empty()) {
      lv_label_set_text(this->title_label_, this->name_.c_str());
    } else {
      lv_label_set_text(this->title_label_, title.c_str());
    }
  }

  if (this->artist_label_ != nullptr) {
    const std::string &artist = this->media_player_->get_media_artist();
    lv_label_set_text(this->artist_label_, artist.c_str());
  }
}

void MediaPlayerApp::update_volume_arc_() {
  if (this->volume_arc_ == nullptr || this->media_player_ == nullptr) return;

  float volume = this->media_player_->get_volume();
  int vol_percent = static_cast<int>(volume * 100);

  lv_arc_set_value(this->volume_arc_, vol_percent);

  if (this->volume_label_ != nullptr) {
    bool muted = this->media_player_->is_muted();
    if (muted) {
      lv_label_set_text(this->volume_label_, SYMBOL_MUTE " Muet");
      lv_obj_set_style_text_color(this->volume_label_, lv_color_hex(0x888888), 0);
    } else {
      char buf[16];
      snprintf(buf, sizeof(buf), SYMBOL_VOLUME_UP " %d%%", vol_percent);
      lv_label_set_text(this->volume_label_, buf);
      lv_obj_set_style_text_color(this->volume_label_, lv_color_hex(this->color_), 0);
    }
  }
}

std::string MediaPlayerApp::get_state_text_() {
  if (this->media_player_ == nullptr) return "";

  bool is_french = true;
  if (this->controller_ != nullptr) {
    is_french = this->controller_->is_french();
  }

  auto state = this->media_player_->get_state();
  switch (state) {
    case homeassistant_addon::MediaPlayerState::PLAYING:
      return is_french ? "Lecture" : "Playing";
    case homeassistant_addon::MediaPlayerState::PAUSED:
      return is_french ? "Pause" : "Paused";
    case homeassistant_addon::MediaPlayerState::IDLE:
      return is_french ? "Inactif" : "Idle";
    case homeassistant_addon::MediaPlayerState::OFF:
      return is_french ? "Éteint" : "Off";
    case homeassistant_addon::MediaPlayerState::ON:
      return is_french ? "Allumé" : "On";
    case homeassistant_addon::MediaPlayerState::STANDBY:
      return is_french ? "Veille" : "Standby";
    case homeassistant_addon::MediaPlayerState::BUFFERING:
      return is_french ? "Chargement..." : "Buffering...";
    default:
      return is_french ? "Inconnu" : "Unknown";
  }
}

void MediaPlayerApp::on_encoder_rotate(int direction) {
  if (this->media_player_ == nullptr) return;

  // Adjust volume with encoder
  float current_volume = this->media_player_->get_volume();
  float step = this->volume_step_;  // Use local volume step
  float new_volume = current_volume + (direction * step);

  if (new_volume < 0.0f) new_volume = 0.0f;
  if (new_volume > 1.0f) new_volume = 1.0f;

  ESP_LOGD(TAG, "Volume change: %.2f -> %.2f (pending)", current_volume, new_volume);

  // Debounced volume update
  this->pending_volume_ = new_volume;
  this->last_volume_change_ = millis();

  // Update arc immediately for visual feedback
  if (this->volume_arc_ != nullptr) {
    lv_arc_set_value(this->volume_arc_, static_cast<int>(new_volume * 100));
  }
  if (this->volume_label_ != nullptr) {
    char buf[16];
    snprintf(buf, sizeof(buf), SYMBOL_VOLUME_UP " %d%%", static_cast<int>(new_volume * 100));
    lv_label_set_text(this->volume_label_, buf);
  }
}

void MediaPlayerApp::on_button_press() {
  if (this->media_player_ == nullptr) return;

  // Perform action based on selected button
  switch (this->selected_button_) {
    case 0:  // Previous
      ESP_LOGD(TAG, "Previous track");
      this->media_player_->previous_track();
      break;
    case 1:  // Play/Pause
      ESP_LOGD(TAG, "Play/Pause");
      this->media_player_->play_pause();
      break;
    case 2:  // Next
      ESP_LOGD(TAG, "Next track");
      this->media_player_->next_track();
      break;
  }

  // Cycle to next button
  int old_selection = this->selected_button_;
  this->selected_button_ = (this->selected_button_ + 1) % 3;

  // Update button visuals
  lv_obj_t *buttons[] = {this->btn_prev_, this->btn_play_, this->btn_next_};
  
  // Clear old selection outline
  if (buttons[old_selection] != nullptr) {
    lv_obj_set_style_outline_width(buttons[old_selection], 0, 0);
  }
  
  // Set new selection outline
  if (buttons[this->selected_button_] != nullptr) {
    lv_obj_set_style_outline_width(buttons[this->selected_button_], 2, 0);
    lv_obj_set_style_outline_color(buttons[this->selected_button_], lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_outline_pad(buttons[this->selected_button_], 3, 0);
  }
}

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_MEDIA_PLAYER
