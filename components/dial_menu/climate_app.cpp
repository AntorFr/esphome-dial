/**
 * @file climate_app.cpp
 * @brief Implementation of the Climate App for thermostat control
 */

#ifdef USE_DIAL_MENU_CLIMATE

#include "climate_app.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "climate_app";

// Store app pointer for static callback
static ClimateApp *g_current_climate_app = nullptr;

// Debounce delay in ms before applying temperature change
static const uint32_t TEMP_CHANGE_DEBOUNCE_MS = 800;

void ClimateApp::on_enter() {
  ESP_LOGI(TAG, "Entering Climate App: %s", this->name_.c_str());
  g_current_climate_app = this;
  
  // Initialize pending temp from current target
  if (this->climate_ != nullptr) {
    this->pending_target_temp_ = this->climate_->target_temperature;
  }
  this->has_pending_change_ = false;
  
  // Show the app page
  if (this->page_ != nullptr) {
    lv_scr_load(this->page_);
    this->update_state();
  }
}

void ClimateApp::on_exit() {
  ESP_LOGI(TAG, "Exiting Climate App: %s", this->name_.c_str());
  
  // Apply any pending change before leaving
  if (this->has_pending_change_) {
    this->apply_pending_change();
  }
  
  g_current_climate_app = nullptr;
}

void ClimateApp::on_button_press() {
  ESP_LOGD(TAG, "Button pressed in Climate App");
  // Toggle between modes
  this->toggle_mode();
}

void ClimateApp::on_encoder_rotate(int delta) {
  ESP_LOGD(TAG, "Encoder rotated: %d", delta);
  
  if (delta > 0) {
    this->increase_temperature();
  } else if (delta < 0) {
    this->decrease_temperature();
  }
}

void ClimateApp::increase_temperature() {
  if (this->climate_ == nullptr) return;
  
  float new_temp = this->pending_target_temp_ + this->temperature_step_;
  
  // Clamp to max
  auto traits = this->climate_->get_traits();
  if (new_temp > traits.get_visual_max_temperature()) {
    new_temp = traits.get_visual_max_temperature();
  }
  
  this->pending_target_temp_ = new_temp;
  this->has_pending_change_ = true;
  this->last_encoder_time_ = millis();
  
  ESP_LOGD(TAG, "Target temperature increased to: %.1f (pending)", new_temp);
  this->update_state();
}

void ClimateApp::decrease_temperature() {
  if (this->climate_ == nullptr) return;
  
  float new_temp = this->pending_target_temp_ - this->temperature_step_;
  
  // Clamp to min
  auto traits = this->climate_->get_traits();
  if (new_temp < traits.get_visual_min_temperature()) {
    new_temp = traits.get_visual_min_temperature();
  }
  
  this->pending_target_temp_ = new_temp;
  this->has_pending_change_ = true;
  this->last_encoder_time_ = millis();
  
  ESP_LOGD(TAG, "Target temperature decreased to: %.1f (pending)", new_temp);
  this->update_state();
}

void ClimateApp::set_target_temperature(float temp) {
  if (this->climate_ == nullptr) return;
  
  ESP_LOGI(TAG, "Setting target temperature to: %.1f", temp);
  
  auto call = this->climate_->make_call();
  call.set_target_temperature(temp);
  call.perform();
}

void ClimateApp::apply_pending_change() {
  if (!this->has_pending_change_ || this->climate_ == nullptr) return;
  
  ESP_LOGI(TAG, "Applying pending temperature: %.1f", this->pending_target_temp_);
  this->set_target_temperature(this->pending_target_temp_);
  this->has_pending_change_ = false;
}

void ClimateApp::toggle_mode() {
  if (this->climate_ == nullptr) return;
  
  auto traits = this->climate_->get_traits();
  auto modes = traits.get_supported_modes();
  
  // Find current mode index
  climate::ClimateMode current = this->climate_->mode;
  int current_idx = -1;
  int i = 0;
  for (auto mode : modes) {
    if (mode == current) {
      current_idx = i;
      break;
    }
    i++;
  }
  
  // Move to next mode
  int next_idx = (current_idx + 1) % modes.size();
  i = 0;
  for (auto mode : modes) {
    if (i == next_idx) {
      this->set_mode(mode);
      break;
    }
    i++;
  }
}

void ClimateApp::set_mode(climate::ClimateMode mode) {
  if (this->climate_ == nullptr) return;
  
  ESP_LOGI(TAG, "Setting mode to: %s", climate::climate_mode_to_string(mode));
  
  auto call = this->climate_->make_call();
  call.set_mode(mode);
  call.perform();
}

void ClimateApp::create_app_ui() {
  ESP_LOGI(TAG, "Creating UI for Climate App: %s", this->name_.c_str());
  
  // Create a new screen/page for this app
  this->page_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(0x000000), 0);
  
  // Get fonts
  const lv_font_t *font_14 = this->font_14_ ? this->font_14_->get_lv_font() : &lv_font_montserrat_14;
  const lv_font_t *font_18 = this->font_18_ ? this->font_18_->get_lv_font() : &lv_font_montserrat_18;
  
  // Climate name at top
  this->name_label_ = lv_label_create(this->page_);
  lv_obj_align(this->name_label_, LV_ALIGN_TOP_MID, 0, 25);
  lv_obj_set_style_text_color(this->name_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->name_label_, font_14, 0);
  lv_label_set_text(this->name_label_, this->name_.c_str());
  
  // Temperature arc (background)
  this->temp_arc_ = lv_arc_create(this->page_);
  lv_obj_set_size(this->temp_arc_, 180, 180);
  lv_obj_align(this->temp_arc_, LV_ALIGN_CENTER, 0, 5);
  lv_arc_set_rotation(this->temp_arc_, 135);
  lv_arc_set_bg_angles(this->temp_arc_, 0, 270);
  lv_arc_set_range(this->temp_arc_, 7, 35);  // 7-35°C range
  lv_arc_set_value(this->temp_arc_, 20);
  lv_obj_remove_style(this->temp_arc_, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(this->temp_arc_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(this->temp_arc_, 12, LV_PART_MAIN);
  lv_obj_set_style_arc_width(this->temp_arc_, 12, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(this->temp_arc_, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_arc_color(this->temp_arc_, lv_color_hex(0xEB8429), LV_PART_INDICATOR);
  
  // Target temperature (large, center)
  this->target_temp_label_ = lv_label_create(this->page_);
  lv_obj_align(this->target_temp_label_, LV_ALIGN_CENTER, 0, -15);
  lv_obj_set_style_text_color(this->target_temp_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->target_temp_label_, &lv_font_montserrat_48, 0);
  lv_label_set_text(this->target_temp_label_, "--");
  
  // Unit label (°C)
  this->unit_label_ = lv_label_create(this->page_);
  lv_obj_align(this->unit_label_, LV_ALIGN_CENTER, 50, -25);
  lv_obj_set_style_text_color(this->unit_label_, lv_color_hex(0x888888), 0);
  lv_obj_set_style_text_font(this->unit_label_, font_18, 0);
  lv_label_set_text(this->unit_label_, "°C");
  
  // Current temperature (smaller, below target)
  this->current_temp_label_ = lv_label_create(this->page_);
  lv_obj_align(this->current_temp_label_, LV_ALIGN_CENTER, 0, 25);
  lv_obj_set_style_text_color(this->current_temp_label_, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_font(this->current_temp_label_, font_14, 0);
  lv_label_set_text(this->current_temp_label_, "Actuel: --°C");
  
  // Action label (Heating, Cooling, Idle)
  this->action_label_ = lv_label_create(this->page_);
  lv_obj_align(this->action_label_, LV_ALIGN_CENTER, 0, 45);
  lv_obj_set_style_text_color(this->action_label_, lv_color_hex(0xEB8429), 0);
  lv_obj_set_style_text_font(this->action_label_, font_14, 0);
  lv_label_set_text(this->action_label_, "");
  
  // Mode button at bottom
  this->mode_btn_ = lv_btn_create(this->page_);
  lv_obj_set_size(this->mode_btn_, 80, 36);
  lv_obj_align(this->mode_btn_, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_radius(this->mode_btn_, 18, 0);
  lv_obj_set_style_bg_color(this->mode_btn_, lv_color_hex(0x333333), 0);
  lv_obj_set_style_border_width(this->mode_btn_, 2, 0);
  lv_obj_set_style_border_color(this->mode_btn_, lv_color_hex(0x555555), 0);
  lv_obj_set_user_data(this->mode_btn_, this);
  lv_obj_add_event_cb(this->mode_btn_, mode_btn_event_cb, LV_EVENT_CLICKED, nullptr);
  
  this->mode_label_ = lv_label_create(this->mode_btn_);
  lv_obj_center(this->mode_label_);
  lv_obj_set_style_text_color(this->mode_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->mode_label_, font_14, 0);
  lv_label_set_text(this->mode_label_, "OFF");
  
  // Register state callback
  if (this->climate_ != nullptr) {
    this->climate_->add_on_state_callback([this]() {
      if (g_current_climate_app == this) {
        ESP_LOGD(TAG, "Climate state changed, refreshing UI");
        // Update pending temp if no pending change
        if (!this->has_pending_change_) {
          this->pending_target_temp_ = this->climate_->target_temperature;
        }
        this->update_state();
      }
    });
    
    // Initialize pending temp
    this->pending_target_temp_ = this->climate_->target_temperature;
  }
  
  // Set initial state
  this->update_state();
  
  ESP_LOGI(TAG, "Climate App UI created");
}

void ClimateApp::update_state() {
  if (this->climate_ == nullptr || this->page_ == nullptr) {
    return;
  }
  
  // Check if we should apply pending change (debounce)
  if (this->has_pending_change_ && (millis() - this->last_encoder_time_ > TEMP_CHANGE_DEBOUNCE_MS)) {
    this->apply_pending_change();
  }
  
  // Get climate state
  float current_temp = this->climate_->current_temperature;
  float target_temp = this->has_pending_change_ ? this->pending_target_temp_ : this->climate_->target_temperature;
  climate::ClimateMode mode = this->climate_->mode;
  climate::ClimateAction action = this->climate_->action;
  
  // Update target temperature label
  if (this->target_temp_label_ != nullptr) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", target_temp);
    lv_label_set_text(this->target_temp_label_, buf);
    
    // Show different color when pending
    if (this->has_pending_change_) {
      lv_obj_set_style_text_color(this->target_temp_label_, lv_color_hex(0xFFFF00), 0);  // Yellow when pending
    } else {
      lv_obj_set_style_text_color(this->target_temp_label_, lv_color_hex(0xFFFFFF), 0);
    }
  }
  
  // Update current temperature label
  if (this->current_temp_label_ != nullptr) {
    char buf[20];
    if (!std::isnan(current_temp)) {
      snprintf(buf, sizeof(buf), "Actuel: %.1f°C", current_temp);
    } else {
      snprintf(buf, sizeof(buf), "Actuel: --°C");
    }
    lv_label_set_text(this->current_temp_label_, buf);
  }
  
  // Update arc
  if (this->temp_arc_ != nullptr) {
    auto traits = this->climate_->get_traits();
    lv_arc_set_range(this->temp_arc_, 
                     (int)traits.get_visual_min_temperature(), 
                     (int)traits.get_visual_max_temperature());
    lv_arc_set_value(this->temp_arc_, (int)target_temp);
    
    // Arc color based on action
    uint32_t arc_color = this->get_action_color(action);
    lv_obj_set_style_arc_color(this->temp_arc_, lv_color_hex(arc_color), LV_PART_INDICATOR);
  }
  
  // Update action label
  if (this->action_label_ != nullptr) {
    lv_label_set_text(this->action_label_, this->get_action_text(action));
    lv_obj_set_style_text_color(this->action_label_, lv_color_hex(this->get_action_color(action)), 0);
  }
  
  // Update mode button
  if (this->mode_label_ != nullptr) {
    lv_label_set_text(this->mode_label_, this->get_mode_text(mode));
  }
  
  if (this->mode_btn_ != nullptr) {
    uint32_t mode_color = 0x555555;
    switch (mode) {
      case climate::CLIMATE_MODE_HEAT:
        mode_color = 0xEB8429;
        break;
      case climate::CLIMATE_MODE_COOL:
        mode_color = 0x577EFF;
        break;
      case climate::CLIMATE_MODE_HEAT_COOL:
      case climate::CLIMATE_MODE_AUTO:
        mode_color = 0x03A964;
        break;
      default:
        mode_color = 0x555555;
        break;
    }
    lv_obj_set_style_border_color(this->mode_btn_, lv_color_hex(mode_color), 0);
  }
  
  ESP_LOGD(TAG, "Climate state: current=%.1f, target=%.1f, mode=%s, action=%s",
           current_temp, target_temp, 
           climate::climate_mode_to_string(mode),
           climate::climate_action_to_string(action));
}

const char* ClimateApp::get_action_text(climate::ClimateAction action) {
  switch (action) {
    case climate::CLIMATE_ACTION_HEATING:
      return "Chauffage...";
    case climate::CLIMATE_ACTION_COOLING:
      return "Refroidissement...";
    case climate::CLIMATE_ACTION_IDLE:
      return "En attente";
    case climate::CLIMATE_ACTION_DRYING:
      return "Séchage...";
    case climate::CLIMATE_ACTION_FAN:
      return "Ventilation...";
    case climate::CLIMATE_ACTION_OFF:
    default:
      return "Arrêté";
  }
}

const char* ClimateApp::get_mode_text(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_HEAT:
      return "Chauff.";
    case climate::CLIMATE_MODE_COOL:
      return "Froid";
    case climate::CLIMATE_MODE_HEAT_COOL:
      return "Auto";
    case climate::CLIMATE_MODE_AUTO:
      return "Auto";
    case climate::CLIMATE_MODE_DRY:
      return "Sec";
    case climate::CLIMATE_MODE_FAN_ONLY:
      return "Vent.";
    case climate::CLIMATE_MODE_OFF:
    default:
      return "OFF";
  }
}

const char* ClimateApp::get_mode_icon(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_HEAT:
      return LV_SYMBOL_UP;
    case climate::CLIMATE_MODE_COOL:
      return LV_SYMBOL_DOWN;
    case climate::CLIMATE_MODE_HEAT_COOL:
    case climate::CLIMATE_MODE_AUTO:
      return LV_SYMBOL_REFRESH;
    case climate::CLIMATE_MODE_OFF:
    default:
      return LV_SYMBOL_POWER;
  }
}

uint32_t ClimateApp::get_action_color(climate::ClimateAction action) {
  switch (action) {
    case climate::CLIMATE_ACTION_HEATING:
      return 0xEB8429;  // Orange
    case climate::CLIMATE_ACTION_COOLING:
      return 0x577EFF;  // Blue
    case climate::CLIMATE_ACTION_IDLE:
      return 0x888888;  // Gray
    case climate::CLIMATE_ACTION_DRYING:
      return 0xFFB300;  // Amber
    case climate::CLIMATE_ACTION_FAN:
      return 0x03A964;  // Green
    case climate::CLIMATE_ACTION_OFF:
    default:
      return 0x555555;  // Dark gray
  }
}

void ClimateApp::mode_btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  ClimateApp *app = static_cast<ClimateApp *>(lv_obj_get_user_data(btn));
  
  if (app != nullptr) {
    app->toggle_mode();
  }
}

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_CLIMATE
