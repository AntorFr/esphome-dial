/**
 * @file dial_menu_controller.cpp
 * @brief Implementation of the LVGL-based Dial Menu Controller
 * 
 * This creates all LVGL widgets programmatically - no YAML needed for UI.
 */

#include "dial_menu_controller.h"
#include <map>

namespace esphome {
namespace dial_menu {

static const char *const TAG = "dial_menu";

// Store controller reference for static callback
static DialMenuController *g_controller = nullptr;

// Map icon type strings to LVGL built-in symbols
// These symbols are always available in LVGL
static const char* get_lvgl_symbol(const std::string &icon_type) {
  // LVGL built-in symbols (from lv_symbol_def.h)
  if (icon_type == "settings") return LV_SYMBOL_SETTINGS;
  if (icon_type == "wifi") return LV_SYMBOL_WIFI;
  if (icon_type == "bluetooth") return LV_SYMBOL_BLUETOOTH;
  if (icon_type == "brightness") return LV_SYMBOL_IMAGE;  // No sun, use image
  if (icon_type == "home") return LV_SYMBOL_HOME;
  if (icon_type == "music") return LV_SYMBOL_AUDIO;
  if (icon_type == "timer") return LV_SYMBOL_BELL;  // No clock, use bell
  if (icon_type == "temperature") return LV_SYMBOL_CHARGE;  // No thermometer in LVGL
  if (icon_type == "power") return LV_SYMBOL_POWER;
  if (icon_type == "light") return LV_SYMBOL_CHARGE;  // No lightbulb, use charge
  if (icon_type == "fan") return LV_SYMBOL_REFRESH;
  if (icon_type == "lock") return LV_SYMBOL_EYE_CLOSE;
  if (icon_type == "play") return LV_SYMBOL_PLAY;
  if (icon_type == "pause") return LV_SYMBOL_PAUSE;
  if (icon_type == "stop") return LV_SYMBOL_STOP;
  if (icon_type == "next") return LV_SYMBOL_NEXT;
  if (icon_type == "info") return LV_SYMBOL_WARNING;
  if (icon_type == "warning") return LV_SYMBOL_WARNING;
  if (icon_type == "check") return LV_SYMBOL_OK;
  if (icon_type == "cross") return LV_SYMBOL_CLOSE;
  // Media player and speaker icons
  if (icon_type == "speaker") return LV_SYMBOL_VOLUME_MAX;
  if (icon_type == "media_player") return LV_SYMBOL_AUDIO;
  if (icon_type == "tv") return LV_SYMBOL_VIDEO;
  // Climate/thermostat icons - no thermometer in LVGL, use tint (droplet)
  if (icon_type == "thermostat") return LV_SYMBOL_TINT;
  if (icon_type == "hvac") return LV_SYMBOL_TINT;
  // Cover/gate icons - no door in LVGL, use directory (folder with open effect)
  if (icon_type == "gate") return LV_SYMBOL_RIGHT;     // Arrow for gate movement
  if (icon_type == "garage") return LV_SYMBOL_UP;      // Up arrow for garage door
  if (icon_type == "blinds") return LV_SYMBOL_BARS;    // Bars for blinds
  if (icon_type == "window") return LV_SYMBOL_BARS;
  return LV_SYMBOL_DUMMY;  // Empty symbol for unknown
}

void DialMenuController::setup() {
  ESP_LOGI(TAG, "Setting up Dial Menu Controller");
  ESP_LOGI(TAG, "  Number of apps: %d", this->apps_.size());
  ESP_LOGI(TAG, "  Button size: %d / %d (focused)", this->button_size_, this->button_size_focused_);
  ESP_LOGI(TAG, "  Idle timeout: %d ms", this->idle_timeout_ms_);
  
  g_controller = this;
  
  // Create the LVGL UI
  this->create_lvgl_ui();
  
  // Setup idle screen
  if (this->time_ != nullptr) {
    this->idle_screen_.set_time(this->time_);
    // Pass custom font to idle screen for French accents
    if (this->font_18_ != nullptr) {
      this->idle_screen_.set_font_18(this->font_18_->get_lv_font());
    }
    this->idle_screen_.create_ui();
    ESP_LOGI(TAG, "Idle screen initialized with time source");
  }
  
  // Initialize activity timer
  this->last_activity_time_ = millis();
  
  // Select first app by default
  if (!this->apps_.empty()) {
    this->selected_index_ = 0;
    // Focus first button
    if (this->apps_[0]->get_lvgl_obj() != nullptr) {
      lv_group_focus_obj(this->apps_[0]->get_lvgl_obj());
    }
  }
}

void DialMenuController::loop() {
  // Check for idle timeout
  if (!this->idle_active_ && this->idle_timeout_ms_ > 0) {
    uint32_t now = millis();
    if (now - this->last_activity_time_ >= this->idle_timeout_ms_) {
      this->show_idle_screen();
    }
  }
  
  // Update idle screen if visible
  if (this->idle_active_) {
    static uint32_t last_update = 0;
    uint32_t now = millis();
    if (now - last_update >= 1000) {  // Update every second
      last_update = now;
      this->idle_screen_.update();
    }
  }
}

void DialMenuController::dump_config() {
  ESP_LOGCONFIG(TAG, "Dial Menu Controller:");
  ESP_LOGCONFIG(TAG, "  Apps: %d", this->apps_.size());
  for (auto *app : this->apps_) {
    ESP_LOGCONFIG(TAG, "    - %s (pos: %d,%d)", 
                  app->get_name().c_str(),
                  app->get_pos_x(),
                  app->get_pos_y());
  }
}

void DialMenuController::create_lvgl_ui() {
  ESP_LOGI(TAG, "Creating LVGL UI...");
  
  // Get active screen
  this->launcher_page_ = lv_scr_act();
  
  // Set black background
  lv_obj_set_style_bg_color(this->launcher_page_, lv_color_hex(0x000000), 0);
  
  // Create input group for encoder navigation
  this->group_ = lv_group_create();
  lv_group_set_wrap(this->group_, true);
  
  // Set as default group for encoder
  lv_indev_t *indev = nullptr;
  while ((indev = lv_indev_get_next(indev)) != nullptr) {
    if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
      lv_indev_set_group(indev, this->group_);
      ESP_LOGI(TAG, "Assigned group to encoder input device");
    }
  }
  
  // Create center decoration
  this->create_center_circle();
  
  // Create app buttons (for launcher)
  for (auto *app : this->apps_) {
    this->create_app_button(app);
  }
  
  // Create app-specific UIs for apps that need them
  for (auto *app : this->apps_) {
    if (app->needs_ui()) {
      app->create_app_ui();
      ESP_LOGI(TAG, "Created UI for app: %s", app->get_name().c_str());
    }
  }
  
  ESP_LOGI(TAG, "LVGL UI created successfully");
}

void DialMenuController::create_center_circle() {
  // Decorative center circle
  lv_obj_t *center = lv_obj_create(this->launcher_page_);
  lv_obj_set_size(center, 76, 76);
  lv_obj_align(center, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(center, 38, 0);
  lv_obj_set_style_bg_color(center, lv_color_hex(0x111111), 0);
  lv_obj_set_style_border_width(center, 1, 0);
  lv_obj_set_style_border_color(center, lv_color_hex(0x333333), 0);
  lv_obj_clear_flag(center, LV_OBJ_FLAG_SCROLLABLE);
  
  // App name label - use custom font with French accents if available
  this->app_name_label_ = lv_label_create(center);
  lv_obj_align(this->app_name_label_, LV_ALIGN_CENTER, 0, -5);
  lv_obj_set_style_text_color(this->app_name_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->app_name_label_, this->get_font_14(), 0);
  if (!this->apps_.empty()) {
    lv_label_set_text(this->app_name_label_, this->apps_[0]->get_name().c_str());
  } else {
    lv_label_set_text(this->app_name_label_, "");
  }
  
  // Hint label
  this->hint_label_ = lv_label_create(center);
  lv_obj_align(this->hint_label_, LV_ALIGN_CENTER, 0, 16);
  lv_obj_set_style_text_color(this->hint_label_, lv_color_hex(0x555555), 0);
  lv_obj_set_style_text_font(this->hint_label_, this->get_font_14(), 0);
  lv_label_set_text(this->hint_label_, "Press to open");
}

void DialMenuController::create_app_button(DialApp *app) {
  // Create button
  lv_obj_t *btn = lv_btn_create(this->launcher_page_);
  app->set_lvgl_obj(btn);
  
  // Store app pointer in button's user data
  lv_obj_set_user_data(btn, app);
  
  // Position and size
  lv_obj_set_size(btn, this->button_size_, this->button_size_);
  lv_obj_align(btn, LV_ALIGN_CENTER, app->get_pos_x(), app->get_pos_y());
  
  // Style
  lv_obj_set_style_radius(btn, this->button_size_ / 2, 0);
  lv_obj_set_style_bg_color(btn, lv_color_hex(app->get_color()), 0);
  lv_obj_set_style_border_width(btn, 2, 0);
  lv_obj_set_style_border_color(btn, lv_color_hex(0x444444), 0);
  lv_obj_set_style_shadow_width(btn, 8, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_hex(app->get_color()), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
  
  // Add to group for encoder navigation
  lv_group_add_obj(this->group_, btn);
  
  // Add icon label using LVGL built-in symbols (must use built-in font for FontAwesome icons)
  lv_obj_t *icon_label = lv_label_create(btn);
  lv_obj_set_style_text_color(icon_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_14, 0);  // Built-in font has FontAwesome symbols
  lv_obj_center(icon_label);
  
  // Use LVGL symbol based on icon type, or first letter as fallback
  const char* icon = app->get_icon().c_str();
  const char* symbol = get_lvgl_symbol(icon);
  if (symbol && symbol[0] != '\0') {
    lv_label_set_text(icon_label, symbol);
    ESP_LOGD(TAG, "Created icon for button '%s'", app->get_name().c_str());
  } else if (!app->get_name().empty()) {
    // Fallback to first letter
    char first_letter[2] = {app->get_name()[0], '\0'};
    lv_label_set_text(icon_label, first_letter);
    ESP_LOGD(TAG, "Created letter '%s' for button '%s'", first_letter, app->get_name().c_str());
  }
  
  // Add event callbacks
  lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_FOCUSED, nullptr);
  lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_DEFOCUSED, nullptr);
  lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_CLICKED, nullptr);
  
  ESP_LOGD(TAG, "Created button for '%s' at (%d, %d)", 
           app->get_name().c_str(), app->get_pos_x(), app->get_pos_y());
}

void DialMenuController::button_event_cb(lv_event_t *e) {
  if (g_controller == nullptr) return;
  
  // Ignore encoder events when an app is open
  if (g_controller->app_open_) return;
  
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  DialApp *app = static_cast<DialApp *>(lv_obj_get_user_data(btn));
  
  if (app == nullptr) return;
  
  switch (code) {
    case LV_EVENT_FOCUSED:
      g_controller->on_app_focused(app->get_index());
      g_controller->update_focus_style(app, true);
      break;
    case LV_EVENT_DEFOCUSED:
      g_controller->update_focus_style(app, false);
      break;
    case LV_EVENT_CLICKED:
      g_controller->on_app_clicked(app->get_index());
      break;
    default:
      break;
  }
}

void DialMenuController::update_focus_style(DialApp *app, bool focused) {
  lv_obj_t *btn = app->get_lvgl_obj();
  if (btn == nullptr) return;
  
  if (focused) {
    // Focused style: larger, white border, stronger shadow
    lv_obj_set_size(btn, this->button_size_focused_, this->button_size_focused_);
    lv_obj_set_style_border_width(btn, 3, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_shadow_width(btn, 20, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_100, 0);
    
    // Update app name label
    if (this->app_name_label_ != nullptr) {
      lv_label_set_text(this->app_name_label_, app->get_name().c_str());
    }
  } else {
    // Default style
    lv_obj_set_size(btn, this->button_size_, this->button_size_);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x444444), 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
  }
}

void DialMenuController::select_app(int index) {
  if (this->apps_.empty()) return;
  
  // Wrap around
  while (index < 0) index += this->apps_.size();
  index = index % this->apps_.size();
  
  if (index != this->selected_index_) {
    ESP_LOGD(TAG, "Selected app %d: %s", index, this->apps_[index]->get_name().c_str());
    this->selected_index_ = index;
  }
  this->reset_idle_timer();
}

void DialMenuController::select_next() {
  this->select_app(this->selected_index_ + 1);
}

void DialMenuController::select_previous() {
  this->select_app(this->selected_index_ - 1);
}

DialApp *DialMenuController::get_selected_app() const {
  if (this->apps_.empty() || this->selected_index_ < 0 || 
      this->selected_index_ >= (int)this->apps_.size()) {
    return nullptr;
  }
  return this->apps_[this->selected_index_];
}

void DialMenuController::open_selected_app() {
  if (this->app_open_) return;
  
  DialApp *app = this->get_selected_app();
  if (app != nullptr) {
    // Only open apps that have a UI - fake apps should not be "opened"
    if (!app->needs_ui()) {
      ESP_LOGD(TAG, "App '%s' has no UI, ignoring click", app->get_name().c_str());
      return;
    }
    ESP_LOGI(TAG, "Opening app: %s", app->get_name().c_str());
    this->app_open_ = true;
    app->on_enter();
  }
}

void DialMenuController::close_current_app() {
  if (!this->app_open_) return;
  
  DialApp *app = this->get_selected_app();
  if (app != nullptr) {
    ESP_LOGI(TAG, "Closing app: %s", app->get_name().c_str());
    app->on_exit();
  }
  this->app_open_ = false;
  
  // Return to launcher
  if (this->launcher_page_ != nullptr) {
    ESP_LOGI(TAG, "Returning to launcher");
    lv_scr_load(this->launcher_page_);
    
    // Re-focus on the selected app button in the group
    if (app != nullptr && app->get_lvgl_obj() != nullptr) {
      lv_group_focus_obj(app->get_lvgl_obj());
      this->update_focus_style(app, true);
    }
  }
}

void DialMenuController::on_app_focused(int index) {
  ESP_LOGD(TAG, "App focused: %d", index);
  this->reset_idle_timer();
  this->select_app(index);
}

void DialMenuController::on_app_clicked(int index) {
  ESP_LOGI(TAG, "App clicked: %d", index);
  this->reset_idle_timer();
  
  // Ignore click if it follows a long press (button release after closing app)
  if (this->ignore_next_click_) {
    ESP_LOGD(TAG, "Ignoring app click after long press");
    this->ignore_next_click_ = false;
    return;
  }
  
  this->select_app(index);
  this->open_selected_app();
}

void DialMenuController::on_button_click() {
  ESP_LOGI(TAG, "Button click detected");
  this->reset_idle_timer();
  
  // Ignore click if it follows a long press (button release after closing app)
  if (this->ignore_next_click_) {
    ESP_LOGD(TAG, "Ignoring click after long press");
    this->ignore_next_click_ = false;
    return;
  }
  
  // If idle screen is active, wake up
  if (this->idle_active_) {
    this->wake_up();
    return;
  }
  
  if (this->app_open_) {
    // If an app is open, forward the click to the app
    DialApp *app = this->get_selected_app();
    if (app != nullptr) {
      app->on_button_press();
    }
  } else {
    // In launcher, open the selected app
    this->open_selected_app();
  }
}

void DialMenuController::on_long_press() {
  ESP_LOGI(TAG, "Long press detected");
  this->reset_idle_timer();
  
  // If idle screen is active, wake up
  if (this->idle_active_) {
    this->wake_up();
    return;
  }
  
  if (this->app_open_) {
    // If an app is open, close it and return to launcher
    this->close_current_app();
    // Set flag to ignore the next click (button release)
    this->ignore_next_click_ = true;
  }
}

void DialMenuController::on_encoder_activity() {
  this->reset_idle_timer();
  
  // If idle screen is active, wake up
  if (this->idle_active_) {
    this->wake_up();
  }
}

void DialMenuController::on_encoder_rotate(int delta) {
  this->reset_idle_timer();
  
  // If idle screen is active, wake up and don't process further
  if (this->idle_active_) {
    this->wake_up();
    return;
  }
  
  // If an app is open, forward the rotation to the app
  if (this->app_open_) {
    DialApp *app = this->get_selected_app();
    if (app != nullptr) {
      app->on_encoder_rotate(delta);
    }
  }
  // If on launcher, navigation is handled by LVGL group automatically
}

void DialMenuController::reset_idle_timer() {
  this->last_activity_time_ = millis();
}

void DialMenuController::show_idle_screen() {
  if (this->idle_active_) {
    return;
  }
  
  ESP_LOGI(TAG, "Entering idle mode");
  
  // Close any open app first
  if (this->app_open_) {
    DialApp *app = this->get_selected_app();
    if (app != nullptr) {
      app->on_exit();
    }
    this->app_open_ = false;
  }
  
  this->idle_active_ = true;
  this->idle_screen_.show();
}

void DialMenuController::wake_up() {
  if (!this->idle_active_) {
    return;
  }
  
  ESP_LOGI(TAG, "Waking up from idle");
  this->idle_active_ = false;
  this->idle_screen_.hide();
  this->reset_idle_timer();
  
  // Return to launcher
  if (this->launcher_page_ != nullptr) {
    lv_scr_load(this->launcher_page_);
  }
}

// Global function to close the current app - callable from any app
void close_current_app_global() {
  if (g_controller != nullptr) {
    g_controller->close_current_app();
  }
}

}  // namespace dial_menu
}  // namespace esphome
