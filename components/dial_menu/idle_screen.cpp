/**
 * @file idle_screen.cpp
 * @brief Implementation of the Idle Screen / Screensaver
 */

#include "idle_screen.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "idle_screen";

// English day and month names
static const char *const DAYS_EN[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char *const MONTHS_EN[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// French day and month names
static const char *const DAYS_FR[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
static const char *const MONTHS_FR[] = {"Jan", "Fév", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sep", "Oct", "Nov", "Déc"};

const char *IdleScreen::get_day_name(int day_of_week) {
  if (day_of_week < 1 || day_of_week > 7) return "";
  if (this->language_ == Language::FR) {
    return DAYS_FR[day_of_week - 1];
  }
  return DAYS_EN[day_of_week - 1];
}

const char *IdleScreen::get_month_name(int month) {
  if (month < 1 || month > 12) return "";
  if (this->language_ == Language::FR) {
    return MONTHS_FR[month - 1];
  }
  return MONTHS_EN[month - 1];
}

void IdleScreen::create_ui() {
  ESP_LOGI(TAG, "Creating idle screen UI");
  
  // Create the page
  this->page_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(0x0a1628), 0);  // Default dark blue
  
  // Day of week at top
  this->day_label_ = lv_label_create(this->page_);
  lv_obj_align(this->day_label_, LV_ALIGN_TOP_MID, 0, 40);
  lv_obj_set_style_text_color(this->day_label_, lv_color_hex(0xAAAAAA), 0);
  // Use custom font if available (for French accents), otherwise fallback to LVGL default
  lv_obj_set_style_text_font(this->day_label_, this->custom_font_18_ ? this->custom_font_18_ : &lv_font_montserrat_18, 0);
  lv_label_set_text(this->day_label_, "Monday");
  
  // Large time display - hours
  this->time_label_ = lv_label_create(this->page_);
  lv_obj_align(this->time_label_, LV_ALIGN_CENTER, -25, -20);
  lv_obj_set_style_text_color(this->time_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->time_label_, &lv_font_montserrat_48, 0);
  lv_label_set_text(this->time_label_, "12");
  
  // Minutes below hours
  this->minute_label_ = lv_label_create(this->page_);
  lv_obj_align(this->minute_label_, LV_ALIGN_CENTER, -25, 35);
  lv_obj_set_style_text_color(this->minute_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->minute_label_, &lv_font_montserrat_48, 0);
  lv_label_set_text(this->minute_label_, "34");
  
  // Date on the right side (day number)
  this->date_label_ = lv_label_create(this->page_);
  lv_obj_align(this->date_label_, LV_ALIGN_CENTER, 60, -15);
  lv_obj_set_style_text_color(this->date_label_, lv_color_hex(0xCCCCCC), 0);
  lv_obj_set_style_text_font(this->date_label_, &lv_font_montserrat_28, 0);
  lv_label_set_text(this->date_label_, "13");
  
  // Month below day number
  this->month_label_ = lv_label_create(this->page_);
  lv_obj_align(this->month_label_, LV_ALIGN_CENTER, 60, 15);
  lv_obj_set_style_text_color(this->month_label_, lv_color_hex(0xAAAAAA), 0);
  // Use custom font if available (for French accents), otherwise fallback to LVGL default
  lv_obj_set_style_text_font(this->month_label_, this->custom_font_18_ ? this->custom_font_18_ : &lv_font_montserrat_18, 0);
  lv_label_set_text(this->month_label_, "June");
  
  ESP_LOGI(TAG, "Idle screen UI created");
}

void IdleScreen::show() {
  if (this->page_ == nullptr) {
    this->create_ui();
  }
  
  ESP_LOGI(TAG, "Showing idle screen");
  this->visible_ = true;
  this->update();
  lv_scr_load(this->page_);
}

void IdleScreen::hide() {
  ESP_LOGI(TAG, "Hiding idle screen");
  this->visible_ = false;
}

void IdleScreen::update() {
  if (!this->visible_ || this->time_ == nullptr) {
    return;
  }
  
  auto now = this->time_->now();
  if (!now.is_valid()) {
    return;
  }
  
  // Update hours
  char hour_buf[4];
  snprintf(hour_buf, sizeof(hour_buf), "%02d", now.hour);
  lv_label_set_text(this->time_label_, hour_buf);
  
  // Update minutes
  char min_buf[4];
  snprintf(min_buf, sizeof(min_buf), "%02d", now.minute);
  lv_label_set_text(this->minute_label_, min_buf);
  
  // Update day of week
  lv_label_set_text(this->day_label_, this->get_day_name(now.day_of_week));
  
  // Update day number
  char day_buf[4];
  snprintf(day_buf, sizeof(day_buf), "%d", now.day_of_month);
  lv_label_set_text(this->date_label_, day_buf);
  
  // Update month
  lv_label_set_text(this->month_label_, this->get_month_name(now.month));
  
  // Update background color based on time
  this->update_background_color();
}

void IdleScreen::update_background_color() {
  if (this->page_ == nullptr) {
    return;
  }
  
  uint32_t color = this->get_time_based_color();
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(color), 0);
}

uint32_t IdleScreen::get_time_based_color() {
  if (this->time_ == nullptr) {
    return 0x0a1628;  // Default dark blue
  }
  
  auto now = this->time_->now();
  if (!now.is_valid()) {
    return 0x0a1628;
  }
  
  int hour = now.hour;
  
  // Night (22:00 - 06:00): Very dark blue/purple
  if (hour >= 22 || hour < 6) {
    return 0x0a0a1a;  // Very dark purple-blue
  }
  
  // Early morning (06:00 - 08:00): Dawn colors - dark orange/red
  if (hour >= 6 && hour < 8) {
    return 0x1a1020;  // Dark purple transitioning to warm
  }
  
  // Morning (08:00 - 12:00): Warm light blue
  if (hour >= 8 && hour < 12) {
    return 0x0a1628;  // Medium blue
  }
  
  // Afternoon (12:00 - 17:00): Bright blue
  if (hour >= 12 && hour < 17) {
    return 0x0a2040;  // Brighter blue
  }
  
  // Evening (17:00 - 20:00): Sunset colors - warm blue
  if (hour >= 17 && hour < 20) {
    return 0x1a1830;  // Purple-blue sunset
  }
  
  // Late evening (20:00 - 22:00): Getting darker
  return 0x101020;  // Dark blue-purple
}

}  // namespace dial_menu
}  // namespace esphome
