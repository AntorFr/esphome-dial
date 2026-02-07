/**
 * @file climate_app.h
 * @brief Climate App - Controls a single ESPHome climate entity (thermostat)
 * 
 * This app displays a thermostat UI and allows adjusting target temperature
 * using the encoder rotation - perfect for the M5 Dial!
 * 
 * Only supports a single climate entity to maximize encoder usability.
 */
#pragma once

#include "esphome/core/defines.h"

#ifdef USE_DIAL_MENU_CLIMATE

#include "dial_menu_controller.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/font/font.h"

namespace esphome {
namespace dial_menu {

/**
 * @brief App that controls a single ESPHome climate entity
 * 
 * Features:
 * - Single climate for direct encoder control
 * - Rotate encoder to adjust target temperature
 * - Visual arc showing current vs target temperature
 * - Mode buttons (heat, cool, off, auto)
 * - Current temperature display
 * - Action indicator (heating, cooling, idle)
 */
class ClimateApp : public DialApp {
 public:
  // Set the climate entity to control
  void set_climate(climate::Climate *climate) { this->climate_ = climate; }
  
  // Set temperature step (how much each encoder click changes temp)
  void set_temperature_step(float step) { this->temperature_step_ = step; }
  
  // Set custom font for labels
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  void set_font_18(font::Font *font) { this->font_18_ = font; }
  
  // App lifecycle - called by DialMenuController
  void on_enter() override;
  void on_exit() override;
  void on_button_press() override;
  void on_encoder_rotate(int delta) override;
  
  // This app needs its own UI page
  bool needs_ui() const override { return true; }
  
  // Create the app's UI (called during setup)
  void create_app_ui() override;
  
  // Update UI to match current climate state
  void update_state();
  
  // Temperature control
  void increase_temperature();
  void decrease_temperature();
  void set_target_temperature(float temp);
  
  // Mode control
  void toggle_mode();
  void set_mode(climate::ClimateMode mode);
  
  // Get the app page
  lv_obj_t *get_page() const { return this->page_; }

 protected:
  climate::Climate *climate_{nullptr};
  float temperature_step_{0.5f};
  
  // Custom fonts (optional)
  font::Font *font_14_{nullptr};
  font::Font *font_18_{nullptr};
  
  // Track pending temperature change for debouncing
  float pending_target_temp_{0.0f};
  uint32_t last_encoder_time_{0};
  bool has_pending_change_{false};
  
  // LVGL objects for this app's UI
  lv_obj_t *page_{nullptr};
  lv_obj_t *name_label_{nullptr};
  lv_obj_t *current_temp_label_{nullptr};
  lv_obj_t *target_temp_label_{nullptr};
  lv_obj_t *unit_label_{nullptr};
  lv_obj_t *action_label_{nullptr};
  lv_obj_t *mode_label_{nullptr};
  lv_obj_t *temp_arc_{nullptr};
  
  // Mode button
  lv_obj_t *mode_btn_{nullptr};
  
  // Event callbacks
  static void mode_btn_event_cb(lv_event_t *e);
  
  // Helper functions
  const char* get_action_text(climate::ClimateAction action);
  const char* get_mode_text(climate::ClimateMode mode);
  const char* get_mode_icon(climate::ClimateMode mode);
  uint32_t get_action_color(climate::ClimateAction action);
  
  // Apply pending temperature change
  void apply_pending_change();
};

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_CLIMATE
