/**
 * @file idle_screen.h
 * @brief Idle Screen / Screensaver - Shows time and date when inactive
 * 
 * Features:
 * - Displays current time in large format
 * - Shows day of week and date
 * - Background color changes based on time of day
 * - Wakes up on touch or encoder interaction
 */
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/lvgl/lvgl_esphome.h"

namespace esphome {
namespace dial_menu {

/**
 * @brief Idle screen that displays time and date
 * 
 * Colors by time of day:
 * - Night (22:00-06:00): Dark blue/purple
 * - Morning (06:00-12:00): Warm orange/yellow gradient
 * - Afternoon (12:00-18:00): Light blue/cyan
 * - Evening (18:00-22:00): Deep blue/purple
 */
// Language enum
enum class Language { EN, FR };

class IdleScreen {
 public:
  // Set the time source
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  
  // Set the display language
  void set_language(Language lang) { this->language_ = lang; }
  
  // Create the idle screen UI
  void create_ui();
  
  // Show/hide the idle screen
  void show();
  void hide();
  bool is_visible() const { return this->visible_; }
  
  // Update the display (call periodically to update time)
  void update();
  
  // Get the page object
  lv_obj_t *get_page() const { return this->page_; }

 protected:
  // Update background color based on time of day
  void update_background_color();
  
  // Get color for current time
  uint32_t get_time_based_color();
  
  time::RealTimeClock *time_{nullptr};
  bool visible_{false};
  Language language_{Language::EN};
  
  // Get localized day/month names
  const char *get_day_name(int day_of_week);
  const char *get_month_name(int month);
  
  // LVGL objects
  lv_obj_t *page_{nullptr};
  lv_obj_t *time_label_{nullptr};      // Large hours display
  lv_obj_t *minute_label_{nullptr};    // Large minutes display
  lv_obj_t *date_label_{nullptr};      // Day number
  lv_obj_t *month_label_{nullptr};     // Month name
  lv_obj_t *day_label_{nullptr};       // Day of week
};

}  // namespace dial_menu
}  // namespace esphome
