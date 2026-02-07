/**
 * @file dial_menu_controller.h
 * @brief LVGL-based Dial Menu Controller
 * 
 * This controller manages the dial menu and creates LVGL widgets automatically.
 * No YAML configuration needed for the UI - everything is generated from C++.
 */
#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/lvgl/lvgl_esphome.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/font/font.h"
#include "idle_screen.h"
// Note: App-specific headers (switch_app.h, cover_app.h, etc.) should be included
// in the .cpp files that need them, not here, to avoid circular dependencies.
#include <vector>
#include <string>

namespace esphome {
namespace dial_menu {

/**
 * @brief Represents a single app in the dial menu
 */
class DialApp {
 public:
  void set_name(const std::string &name) { this->name_ = name; }
  const std::string &get_name() const { return this->name_; }
  
  void set_index(int index) { this->index_ = index; }
  int get_index() const { return this->index_; }
  
  void set_color(uint32_t color) { this->color_ = color; }
  uint32_t get_color() const { return this->color_; }
  
  void set_icon(const std::string &icon) { this->icon_ = icon; }
  const std::string &get_icon() const { return this->icon_; }
  
  void set_position(int x, int y) { this->pos_x_ = x; this->pos_y_ = y; }
  int get_pos_x() const { return this->pos_x_; }
  int get_pos_y() const { return this->pos_y_; }
  
  // LVGL button reference
  void set_lvgl_obj(lv_obj_t *obj) { this->lvgl_obj_ = obj; }
  lv_obj_t *get_lvgl_obj() const { return this->lvgl_obj_; }
  
  // App lifecycle callbacks (to be overridden by specific app types)
  virtual void on_enter() {}
  virtual void on_exit() {}
  virtual void on_encoder_rotate(int delta) {}
  virtual void on_button_press() {}
  
  // Does this app need its own UI? Override to return true in app types like SwitchApp
  virtual bool needs_ui() const { return false; }
  
  // Create the app-specific UI - called during setup for apps that need it
  virtual void create_app_ui() {}

 protected:
  std::string name_;
  std::string icon_;
  uint32_t color_{0xFFFFFF};
  int index_{0};
  int pos_x_{0};
  int pos_y_{0};
  lv_obj_t *lvgl_obj_{nullptr};
};

/**
 * @brief Main controller for the dial menu
 * 
 * Creates LVGL widgets automatically and manages navigation.
 */
class DialMenuController : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }
  
  // Configuration
  void add_app(DialApp *app) { this->apps_.push_back(app); }
  void set_group_name(const std::string &name) { this->group_name_ = name; }
  void set_button_size(int size) { this->button_size_ = size; }
  void set_button_size_focused(int size) { this->button_size_focused_ = size; }
  void set_idle_timeout(uint32_t timeout_ms) { this->idle_timeout_ms_ = timeout_ms; }
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_font_14(font::Font *font) { this->font_14_ = font; }
  void set_font_18(font::Font *font) { this->font_18_ = font; }
  void set_language(const std::string &lang) {
    if (lang == "fr") {
      this->idle_screen_.set_language(Language::FR);
    } else {
      this->idle_screen_.set_language(Language::EN);
    }
  }
  
  // Get LVGL font (use custom if set, otherwise fallback to built-in)
  const lv_font_t* get_font_14() const { 
    return this->font_14_ ? this->font_14_->get_lv_font() : &lv_font_montserrat_14; 
  }
  const lv_font_t* get_font_18() const { 
    return this->font_18_ ? this->font_18_->get_lv_font() : &lv_font_montserrat_18; 
  }
  
  // Navigation
  void select_app(int index);
  void select_next();
  void select_previous();
  int get_selected_index() const { return this->selected_index_; }
  DialApp *get_selected_app() const;
  
  // App lifecycle
  void open_selected_app();
  void close_current_app();
  bool is_app_open() const { return this->app_open_; }
  
  // Hardware button callbacks
  void on_button_click();     // Short click - open app or perform action
  void on_long_press();       // Long press - goes back to launcher
  void on_encoder_activity(); // Encoder rotation - wake up if idle (deprecated)
  void on_encoder_rotate(int delta); // Encoder rotation with direction
  
  // Idle screen / screensaver
  void reset_idle_timer();  // Reset inactivity timer
  void show_idle_screen();  // Show the idle/clock screen
  void wake_up();           // Wake from idle screen
  bool is_idle() const { return this->idle_active_; }
  
  // LVGL callbacks
  void on_app_focused(int index);
  void on_app_clicked(int index);
  
 protected:
  // Create all LVGL widgets
  void create_lvgl_ui();
  void create_center_circle();
  void create_app_button(DialApp *app);
  void update_focus_style(DialApp *app, bool focused);
  
  // LVGL event callback
  static void button_event_cb(lv_event_t *e);
  
  std::vector<DialApp *> apps_;
  std::string group_name_{"dial_menu_group"};
  int selected_index_{0};
  bool app_open_{false};
  int button_size_{50};
  int button_size_focused_{58};
  
  // Custom fonts (optional, nullptr = use built-in)
  font::Font *font_14_{nullptr};
  font::Font *font_18_{nullptr};
  
  // LVGL objects
  lv_obj_t *launcher_page_{nullptr};
  lv_obj_t *app_name_label_{nullptr};
  lv_obj_t *hint_label_{nullptr};
  lv_group_t *group_{nullptr};
  
  // Idle screen
  IdleScreen idle_screen_;
  time::RealTimeClock *time_{nullptr};
  uint32_t idle_timeout_ms_{30000};  // Default 30 seconds
  uint32_t last_activity_time_{0};
  bool idle_active_{false};
  int last_encoder_value_{0};
  
  // Flag to ignore the click event after a long press
  bool ignore_next_click_{false};
};

}  // namespace dial_menu
}  // namespace esphome

// Global function to close the current app - callable from any app
namespace esphome {
namespace dial_menu {
void close_current_app_global();
}  // namespace dial_menu
}  // namespace esphome
