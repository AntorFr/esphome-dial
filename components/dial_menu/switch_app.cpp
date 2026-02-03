/**
 * @file switch_app.cpp
 * @brief Implementation of the Switch App
 */

#include "switch_app.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "switch_app";

// Store app pointer for static callback
static SwitchApp *g_current_switch_app = nullptr;

void SwitchApp::on_enter() {
  ESP_LOGI(TAG, "Entering Switch App: %s", this->name_.c_str());
  g_current_switch_app = this;
  
  // Show the app page
  if (this->page_ != nullptr) {
    lv_scr_load(this->page_);
    this->update_state();
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

void SwitchApp::create_app_ui() {
  ESP_LOGI(TAG, "Creating UI for Switch App: %s", this->name_.c_str());
  
  // Create a new screen/page for this app
  this->page_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(0x000000), 0);
  
  // App name at top
  this->name_label_ = lv_label_create(this->page_);
  lv_obj_align(this->name_label_, LV_ALIGN_TOP_MID, 0, 35);
  lv_obj_set_style_text_color(this->name_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->name_label_, &lv_font_montserrat_14, 0);
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
  
  // State label inside button (ON/OFF or icon)
  this->state_label_ = lv_label_create(this->state_btn_);
  lv_obj_center(this->state_label_);
  lv_obj_set_style_text_color(this->state_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->state_label_, &lv_font_montserrat_14, 0);
  
  // Hint at bottom - show long press to go back
  this->back_label_ = lv_label_create(this->page_);
  lv_obj_align(this->back_label_, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_set_style_text_color(this->back_label_, lv_color_hex(0x555555), 0);
  lv_obj_set_style_text_font(this->back_label_, &lv_font_montserrat_14, 0);
  lv_label_set_text(this->back_label_, "Long press to exit");
  
  // Set initial state
  this->update_state();
  
  ESP_LOGI(TAG, "Switch App UI created");
}

void SwitchApp::update_state() {
  if (this->switch_ == nullptr || this->state_btn_ == nullptr) {
    return;
  }
  
  bool is_on = this->switch_->state;
  uint32_t color = this->color_;
  
  if (is_on) {
    // ON state - use app color
    lv_obj_set_style_bg_color(this->state_btn_, lv_color_hex(color), 0);
    lv_obj_set_style_border_color(this->state_btn_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_shadow_color(this->state_btn_, lv_color_hex(color), 0);
    lv_label_set_text(this->state_label_, LV_SYMBOL_OK);
  } else {
    // OFF state - gray
    lv_obj_set_style_bg_color(this->state_btn_, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_color(this->state_btn_, lv_color_hex(0x555555), 0);
    lv_obj_set_style_shadow_color(this->state_btn_, lv_color_hex(0x333333), 0);
    lv_label_set_text(this->state_label_, LV_SYMBOL_CLOSE);
  }
  
  ESP_LOGD(TAG, "Switch state updated: %s", is_on ? "ON" : "OFF");
}

void SwitchApp::toggle() {
  if (this->switch_ == nullptr) {
    ESP_LOGW(TAG, "No switch configured");
    return;
  }
  
  ESP_LOGI(TAG, "Toggling switch: %s", this->name_.c_str());
  this->switch_->toggle();
  
  // Update UI after a short delay to let the state change propagate
  // In practice, we should use a listener/callback from the switch
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
