/**
 * @file switch_app.h
 * @brief Switch App - Controls multiple ESPHome switch entities
 * 
 * This app displays switches and allows toggling them via touch or encoder.
 * Navigate between switches using the encoder rotation or swipe gestures.
 */
#pragma once

#include "dial_menu_controller.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/font/font.h"
#include <vector>

namespace esphome {
namespace dial_menu {

/**
 * @brief Represents a switch with its display name
 */
struct SwitchItem {
  switch_::Switch *sw;
  std::string name;
  uint32_t color;
};

/**
 * @brief App that controls multiple ESPHome switches
 * 
 * Features:
 * - Multiple switches in one app
 * - Navigate with encoder rotation or swipe left/right
 * - Large circular button showing ON/OFF state
 * - Touch to toggle current switch
 * - Dots indicator showing current position
 */
class SwitchApp : public DialApp {
 public:
  // Add a switch to the app (can add multiple)
  void add_switch(switch_::Switch *sw, const std::string &name, uint32_t color);
  
  // Set custom font for labels
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  
  // Legacy single switch support
  void set_switch(switch_::Switch *sw) { 
    if (this->switches_.empty()) {
      this->add_switch(sw, this->name_, this->color_);
    }
  }
  
  // App lifecycle - called by DialMenuController
  void on_enter() override;
  void on_exit() override;
  void on_button_press() override;
  void on_encoder_rotate(int delta) override;
  
  // This app needs its own UI page
  bool needs_ui() const override { return true; }
  
  // Create the app's UI (called during setup)
  void create_app_ui() override;
  
  // Update UI to match current switch state
  void update_state();
  
  // Update the dots indicator
  void update_dots();
  
  // Toggle the current switch
  void toggle();
  
  // Navigation between switches
  void next_switch();
  void previous_switch();
  void select_switch(int index);
  
  // Get the app page
  lv_obj_t *get_page() const { return this->page_; }
  
  // Get number of switches
  size_t get_switch_count() const { return this->switches_.size(); }

 protected:
  std::vector<SwitchItem> switches_;
  int current_index_{0};
  
  // Custom font (optional)
  font::Font *font_14_{nullptr};
  
  // LVGL objects for this app's UI
  lv_obj_t *page_{nullptr};
  lv_obj_t *state_btn_{nullptr};
  lv_obj_t *state_label_{nullptr};
  lv_obj_t *name_label_{nullptr};
  lv_obj_t *dots_container_{nullptr};
  std::vector<lv_obj_t *> dots_;
  
  // Event callbacks
  static void state_btn_event_cb(lv_event_t *e);
};

}  // namespace dial_menu
}  // namespace esphome
