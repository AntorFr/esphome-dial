/**
 * @file switch_app.cpp
 * @brief Implementation of the Switch App with multi-switch support
 */

#include "switch_app.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "switch_app";

// Store app pointer for static callback
static SwitchApp *g_current_switch_app = nullptr;

void SwitchApp::add_switch(switch_::Switch *sw, const std::string &name, uint32_t color) {
  SwitchItem item;
  item.sw = sw;
  item.name = name;
  item.color = color;
  this->switches_.push_back(item);
  ESP_LOGD(TAG, "Added switch: %s (total: %d)", name.c_str(), this->switches_.size());
}

void SwitchApp::on_enter() {
  ESP_LOGI(TAG, "Entering Switch App: %s", this->name_.c_str());
  g_current_switch_app = this;
  
  // Show the app page
  if (this->page_ != nullptr) {
    lv_scr_load(this->page_);
    this->update_state();
    this->update_dots();
  }
}

void SwitchApp::on_exit() {
  ESP_LOGI(TAG, "Exiting Switch App: %s", this->name_.c_str());
  g_current_switch_app = nullptr;
}

void SwitchApp::on_button_press() {
  ESP_LOGD(TAG, "Button pressed in Switch App");
  this->toggle();
}

void SwitchApp::on_encoder_rotate(int delta) {
  ESP_LOGD(TAG, "Encoder rotated: %d", delta);
  if (delta > 0) {
    this->next_switch();
  } else if (delta < 0) {
    this->previous_switch();
  }
}

void SwitchApp::next_switch() {
  if (this->switches_.size() <= 1) return;
  
  this->current_index_ = (this->current_index_ + 1) % this->switches_.size();
  ESP_LOGD(TAG, "Next switch: index=%d", this->current_index_);
  this->update_state();
  this->update_dots();
}

void SwitchApp::previous_switch() {
  if (this->switches_.size() <= 1) return;
  
  this->current_index_ = (this->current_index_ - 1 + this->switches_.size()) % this->switches_.size();
  ESP_LOGD(TAG, "Previous switch: index=%d", this->current_index_);
  this->update_state();
  this->update_dots();
}

void SwitchApp::select_switch(int index) {
  if (index >= 0 && index < (int)this->switches_.size()) {
    this->current_index_ = index;
    this->update_state();
    this->update_dots();
  }
}

void SwitchApp::create_app_ui() {
  ESP_LOGI(TAG, "Creating UI for Switch App: %s (%d switches)", this->name_.c_str(), this->switches_.size());
  
  // Create a new screen/page for this app
  this->page_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(0x000000), 0);
  
  // App name at top (will show current switch name)
  this->name_label_ = lv_label_create(this->page_);
  lv_obj_align(this->name_label_, LV_ALIGN_TOP_MID, 0, 35);
  lv_obj_set_style_text_color(this->name_label_, lv_color_hex(0xFFFFFF), 0);
  // Use custom font if set, otherwise fallback to built-in
  const lv_font_t *font_14 = this->font_14_ ? this->font_14_->get_lv_font() : &lv_font_montserrat_14;
  lv_obj_set_style_text_font(this->name_label_, font_14, 0);
  lv_label_set_text(this->name_label_, this->name_.c_str());
  
  // Large state button in center
  this->state_btn_ = lv_btn_create(this->page_);
  lv_obj_set_size(this->state_btn_, 120, 120);
  lv_obj_align(this->state_btn_, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(this->state_btn_, 60, 0);
  lv_obj_set_style_border_width(this->state_btn_, 3, 0);
  lv_obj_set_style_shadow_width(this->state_btn_, 20, 0);
  lv_obj_set_style_shadow_opa(this->state_btn_, LV_OPA_50, 0);
  
  // Store this pointer for callback
  lv_obj_set_user_data(this->state_btn_, this);
  lv_obj_add_event_cb(this->state_btn_, state_btn_event_cb, LV_EVENT_CLICKED, nullptr);
  
  // State label inside button (power icon)
  this->state_label_ = lv_label_create(this->state_btn_);
  lv_obj_center(this->state_label_);
  lv_obj_set_style_text_color(this->state_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->state_label_, &lv_font_montserrat_48, 0);
  
  // Dots indicator (pagination) - only if multiple switches
  if (this->switches_.size() > 1) {
    this->dots_container_ = lv_obj_create(this->page_);
    int container_width = this->switches_.size() * 16;
    lv_obj_set_size(this->dots_container_, container_width, 12);
    lv_obj_align(this->dots_container_, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_opa(this->dots_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(this->dots_container_, 0, 0);
    lv_obj_set_style_pad_all(this->dots_container_, 0, 0);
    lv_obj_clear_flag(this->dots_container_, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create dots for each switch - manually positioned
    int dot_spacing = 16;
    int start_x = (container_width - (this->switches_.size() * dot_spacing - (dot_spacing - 8))) / 2;
    for (size_t i = 0; i < this->switches_.size(); i++) {
      lv_obj_t *dot = lv_obj_create(this->dots_container_);
      lv_obj_set_size(dot, 8, 8);
      lv_obj_set_pos(dot, start_x + i * dot_spacing, 2);
      lv_obj_set_style_radius(dot, 4, 0);
      lv_obj_set_style_border_width(dot, 0, 0);
      lv_obj_set_style_bg_color(dot, lv_color_hex(0x555555), 0);
      this->dots_.push_back(dot);
    }
  }
  
  // Set initial state
  this->update_state();
  this->update_dots();
  
  ESP_LOGI(TAG, "Switch App UI created");
}

void SwitchApp::update_state() {
  if (this->switches_.empty() || this->state_btn_ == nullptr) {
    return;
  }
  
  // Get current switch
  SwitchItem &current = this->switches_[this->current_index_];
  
  if (current.sw == nullptr) {
    return;
  }
  
  bool is_on = current.sw->state;
  uint32_t color = current.color;
  
  // Update name label with current switch name
  if (this->name_label_ != nullptr) {
    lv_label_set_text(this->name_label_, current.name.c_str());
  }
  
  if (is_on) {
    // ON state - use switch color
    lv_obj_set_style_bg_color(this->state_btn_, lv_color_hex(color), 0);
    lv_obj_set_style_border_color(this->state_btn_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_shadow_color(this->state_btn_, lv_color_hex(color), 0);
    lv_label_set_text(this->state_label_, LV_SYMBOL_POWER);
  } else {
    // OFF state - gray
    lv_obj_set_style_bg_color(this->state_btn_, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_color(this->state_btn_, lv_color_hex(0x555555), 0);
    lv_obj_set_style_shadow_color(this->state_btn_, lv_color_hex(0x333333), 0);
    lv_label_set_text(this->state_label_, LV_SYMBOL_POWER);
  }
  
  ESP_LOGD(TAG, "Switch '%s' state: %s", current.name.c_str(), is_on ? "ON" : "OFF");
}

void SwitchApp::update_dots() {
  if (this->dots_.empty()) return;
  
  for (size_t i = 0; i < this->dots_.size(); i++) {
    if (i == (size_t)this->current_index_) {
      // Active dot - white
      lv_obj_set_style_bg_color(this->dots_[i], lv_color_hex(0xFFFFFF), 0);
    } else {
      // Inactive dot - gray
      lv_obj_set_style_bg_color(this->dots_[i], lv_color_hex(0x555555), 0);
    }
  }
}

void SwitchApp::toggle() {
  if (this->switches_.empty()) {
    ESP_LOGW(TAG, "No switches configured");
    return;
  }
  
  SwitchItem &current = this->switches_[this->current_index_];
  if (current.sw == nullptr) {
    ESP_LOGW(TAG, "Current switch is null");
    return;
  }
  
  ESP_LOGI(TAG, "Toggling switch: %s", current.name.c_str());
  current.sw->toggle();
  
  // Update UI
  this->update_state();
}

void SwitchApp::state_btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  SwitchApp *app = static_cast<SwitchApp *>(lv_obj_get_user_data(btn));
  
  if (app != nullptr) {
    app->toggle();
  }
}

}  // namespace dial_menu
}  // namespace esphome
