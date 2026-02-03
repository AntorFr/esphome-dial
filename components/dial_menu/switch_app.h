/**
 * @file switch_app.h
 * @brief Switch App - Controls an ESPHome switch entity
 * 
 * This app displays the current state of a switch and allows toggling it
 * via touch or encoder button press.
 */
#pragma once

#include "dial_menu_controller.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace dial_menu {

/**
 * @brief App that controls an ESPHome switch
 * 
 * Features:
 * - Large circular button showing ON/OFF state
 * - Touch to toggle
 * - Encoder button to toggle
 * - Back to launcher on long press or edge swipe
 */
class SwitchApp : public DialApp {
 public:
  // Set the switch to control
  void set_switch(switch_::Switch *sw) { this->switch_ = sw; }
  
  // App lifecycle - called by DialMenuController
  void on_enter() override;
  void on_exit() override;
  void on_button_press() override;
  
  // This app needs its own UI page
  bool needs_ui() const override { return true; }
  
  // Create the app's UI (called during setup)
  void create_app_ui() override;
  
  // Update UI to match switch state
  void update_state();
  
  // Toggle the switch
  void toggle();
  
  // Get the app page
  lv_obj_t *get_page() const { return this->page_; }

 protected:
  switch_::Switch *switch_{nullptr};
  
  // LVGL objects for this app's UI
  lv_obj_t *page_{nullptr};
  lv_obj_t *state_btn_{nullptr};
  lv_obj_t *state_label_{nullptr};
  lv_obj_t *name_label_{nullptr};
  lv_obj_t *back_label_{nullptr};
  
  // Event callback
  static void state_btn_event_cb(lv_event_t *e);
};

}  // namespace dial_menu
}  // namespace esphome
