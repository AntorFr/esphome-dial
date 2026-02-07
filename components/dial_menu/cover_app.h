/**
 * @file cover_app.h
 * @brief Cover App - Controls multiple ESPHome cover entities (gates, blinds, etc.)
 * 
 * This app displays covers and allows controlling them via touch or encoder.
 * Navigate between covers using the encoder rotation or swipe gestures.
 * Actions: Open, Close, Stop, Toggle
 */
#pragma once

#include "esphome/core/defines.h"

#ifdef USE_DIAL_MENU_COVER

#include "dial_menu_controller.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/font/font.h"
#include <vector>

namespace esphome {
namespace dial_menu {

/**
 * @brief Represents a cover with its display name
 */
struct CoverItem {
  cover::Cover *cover;
  std::string name;
  uint32_t color;
};

/**
 * @brief Cover action types for the UI buttons
 */
enum class CoverAction {
  OPEN,
  STOP,
  CLOSE
};

/**
 * @brief App that controls multiple ESPHome covers
 * 
 * Features:
 * - Multiple covers in one app
 * - Navigate with encoder rotation
 * - Three action buttons: Open, Stop, Close
 * - Visual position indicator
 * - Dots indicator showing current cover
 */
class CoverApp : public DialApp {
 public:
  // Add a cover to the app (can add multiple)
  void add_cover(cover::Cover *cover, const std::string &name, uint32_t color);
  
  // Set custom font for labels
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  
  // App lifecycle - called by DialMenuController
  void on_enter() override;
  void on_exit() override;
  void on_button_press() override;
  void on_encoder_rotate(int delta) override;
  
  // This app needs its own UI page
  bool needs_ui() const override { return true; }
  
  // Create the app's UI (called during setup)
  void create_app_ui() override;
  
  // Update UI to match current cover state
  void update_state();
  
  // Update the dots indicator
  void update_dots();
  
  // Update button focus visual
  void update_action_focus();
  
  // Cover actions
  void open_cover();
  void close_cover();
  void stop_cover();
  void toggle_cover();
  
  // Navigation between covers
  void next_cover();
  void previous_cover();
  void select_cover(int index);
  
  // Action selection (with encoder when in action mode)
  void next_action();
  void previous_action();
  void execute_action();
  
  // Get the app page
  lv_obj_t *get_page() const { return this->page_; }
  
  // Get number of covers
  size_t get_cover_count() const { return this->covers_.size(); }

 protected:
  std::vector<CoverItem> covers_;
  int current_index_{0};
  CoverAction selected_action_{CoverAction::STOP};
  
  // Custom font (optional)
  font::Font *font_14_{nullptr};
  
  // LVGL objects for this app's UI
  lv_obj_t *page_{nullptr};
  lv_obj_t *name_label_{nullptr};
  lv_obj_t *status_label_{nullptr};
  lv_obj_t *position_arc_{nullptr};
  lv_obj_t *position_label_{nullptr};
  
  // Action buttons
  lv_obj_t *btn_open_{nullptr};
  lv_obj_t *btn_stop_{nullptr};
  lv_obj_t *btn_close_{nullptr};
  
  // Dots for pagination
  lv_obj_t *dots_container_{nullptr};
  std::vector<lv_obj_t *> dots_;
  
  // Event callbacks
  static void btn_open_event_cb(lv_event_t *e);
  static void btn_stop_event_cb(lv_event_t *e);
  static void btn_close_event_cb(lv_event_t *e);
  
  // Helper to get state text
  const char* get_state_text(cover::CoverOperation op, float position);
};

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_COVER
