/**
 * @file cover_app.cpp
 * @brief Implementation of the Cover App with multi-cover support
 */

#include "esphome/core/defines.h"

#ifdef USE_DIAL_MENU_COVER

#include "cover_app.h"

namespace esphome {
namespace dial_menu {

static const char *const TAG = "cover_app";

// Store app pointer for static callback
static CoverApp *g_current_cover_app = nullptr;

void CoverApp::add_cover(cover::Cover *cover, const std::string &name, uint32_t color) {
  CoverItem item;
  item.cover = cover;
  item.name = name;
  item.color = color;
  this->covers_.push_back(item);
  ESP_LOGD(TAG, "Added cover: %s (total: %d)", name.c_str(), this->covers_.size());
}

void CoverApp::on_enter() {
  ESP_LOGI(TAG, "Entering Cover App: %s", this->name_.c_str());
  g_current_cover_app = this;
  
  // Reset to stop action (middle button)
  this->selected_action_ = CoverAction::STOP;
  
  // Show the app page
  if (this->page_ != nullptr) {
    lv_scr_load(this->page_);
    this->update_state();
    this->update_dots();
    this->update_action_focus();
  }
}

void CoverApp::on_exit() {
  ESP_LOGI(TAG, "Exiting Cover App: %s", this->name_.c_str());
  g_current_cover_app = nullptr;
}

void CoverApp::on_button_press() {
  ESP_LOGD(TAG, "Button pressed in Cover App");
  this->execute_action();
}

void CoverApp::on_encoder_rotate(int delta) {
  ESP_LOGD(TAG, "Encoder rotated: %d", delta);
  
  // If multiple covers, rotate through them
  // If single cover or already rotating actions, change action
  if (this->covers_.size() > 1) {
    // Navigate between covers
    if (delta > 0) {
      this->next_cover();
    } else if (delta < 0) {
      this->previous_cover();
    }
  } else {
    // Navigate between actions (open/stop/close)
    if (delta > 0) {
      this->next_action();
    } else if (delta < 0) {
      this->previous_action();
    }
  }
}

void CoverApp::next_cover() {
  if (this->covers_.size() <= 1) return;
  
  this->current_index_ = (this->current_index_ + 1) % this->covers_.size();
  ESP_LOGD(TAG, "Next cover: index=%d", this->current_index_);
  this->update_state();
  this->update_dots();
}

void CoverApp::previous_cover() {
  if (this->covers_.size() <= 1) return;
  
  this->current_index_ = (this->current_index_ - 1 + this->covers_.size()) % this->covers_.size();
  ESP_LOGD(TAG, "Previous cover: index=%d", this->current_index_);
  this->update_state();
  this->update_dots();
}

void CoverApp::select_cover(int index) {
  if (index >= 0 && index < (int)this->covers_.size()) {
    this->current_index_ = index;
    this->update_state();
    this->update_dots();
  }
}

void CoverApp::next_action() {
  switch (this->selected_action_) {
    case CoverAction::OPEN:
      this->selected_action_ = CoverAction::STOP;
      break;
    case CoverAction::STOP:
      this->selected_action_ = CoverAction::CLOSE;
      break;
    case CoverAction::CLOSE:
      this->selected_action_ = CoverAction::OPEN;
      break;
  }
  ESP_LOGD(TAG, "Next action: %d", (int)this->selected_action_);
  this->update_action_focus();
}

void CoverApp::previous_action() {
  switch (this->selected_action_) {
    case CoverAction::OPEN:
      this->selected_action_ = CoverAction::CLOSE;
      break;
    case CoverAction::STOP:
      this->selected_action_ = CoverAction::OPEN;
      break;
    case CoverAction::CLOSE:
      this->selected_action_ = CoverAction::STOP;
      break;
  }
  ESP_LOGD(TAG, "Previous action: %d", (int)this->selected_action_);
  this->update_action_focus();
}

void CoverApp::execute_action() {
  switch (this->selected_action_) {
    case CoverAction::OPEN:
      this->open_cover();
      break;
    case CoverAction::STOP:
      this->stop_cover();
      break;
    case CoverAction::CLOSE:
      this->close_cover();
      break;
  }
}

void CoverApp::create_app_ui() {
  ESP_LOGI(TAG, "Creating UI for Cover App: %s (%d covers)", this->name_.c_str(), this->covers_.size());
  
  // Create a new screen/page for this app
  this->page_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(this->page_, lv_color_hex(0x000000), 0);
  
  // Get font (custom or fallback)
  const lv_font_t *font_14 = this->font_14_ ? this->font_14_->get_lv_font() : &lv_font_montserrat_14;
  
  // Cover name at top
  this->name_label_ = lv_label_create(this->page_);
  lv_obj_align(this->name_label_, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_text_color(this->name_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->name_label_, font_14, 0);
  lv_label_set_text(this->name_label_, this->name_.c_str());
  
  // Position arc in center (visual indicator of cover position)
  this->position_arc_ = lv_arc_create(this->page_);
  lv_obj_set_size(this->position_arc_, 100, 100);
  lv_obj_align(this->position_arc_, LV_ALIGN_CENTER, 0, -15);
  lv_arc_set_rotation(this->position_arc_, 135);
  lv_arc_set_bg_angles(this->position_arc_, 0, 270);
  lv_arc_set_value(this->position_arc_, 0);
  lv_obj_remove_style(this->position_arc_, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(this->position_arc_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(this->position_arc_, 8, LV_PART_MAIN);
  lv_obj_set_style_arc_width(this->position_arc_, 8, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(this->position_arc_, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_arc_color(this->position_arc_, lv_color_hex(0x03A964), LV_PART_INDICATOR);
  
  // Position percentage label inside arc
  this->position_label_ = lv_label_create(this->page_);
  lv_obj_align(this->position_label_, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_text_color(this->position_label_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(this->position_label_, &lv_font_montserrat_28, 0);
  lv_label_set_text(this->position_label_, "--");
  
  // Status label (Open/Closed/Opening/Closing)
  this->status_label_ = lv_label_create(this->page_);
  lv_obj_align(this->status_label_, LV_ALIGN_CENTER, 0, 15);
  lv_obj_set_style_text_color(this->status_label_, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_font(this->status_label_, font_14, 0);
  lv_label_set_text(this->status_label_, "");
  
  // Action buttons row at bottom
  int btn_size = 50;
  int btn_spacing = 20;
  int total_width = btn_size * 3 + btn_spacing * 2;
  int start_x = -total_width / 2 + btn_size / 2;
  int btn_y = 75;
  
  // Open button (up arrow)
  this->btn_open_ = lv_btn_create(this->page_);
  lv_obj_set_size(this->btn_open_, btn_size, btn_size);
  lv_obj_align(this->btn_open_, LV_ALIGN_CENTER, start_x, btn_y);
  lv_obj_set_style_radius(this->btn_open_, btn_size / 2, 0);
  lv_obj_set_style_bg_color(this->btn_open_, lv_color_hex(0x03A964), 0);
  lv_obj_set_style_border_width(this->btn_open_, 2, 0);
  lv_obj_set_style_border_color(this->btn_open_, lv_color_hex(0x03A964), 0);
  lv_obj_set_user_data(this->btn_open_, this);
  lv_obj_add_event_cb(this->btn_open_, btn_open_event_cb, LV_EVENT_CLICKED, nullptr);
  
  lv_obj_t *label_open = lv_label_create(this->btn_open_);
  lv_label_set_text(label_open, LV_SYMBOL_UP);
  lv_obj_center(label_open);
  lv_obj_set_style_text_color(label_open, lv_color_hex(0xFFFFFF), 0);
  
  // Stop button (square/pause)
  this->btn_stop_ = lv_btn_create(this->page_);
  lv_obj_set_size(this->btn_stop_, btn_size, btn_size);
  lv_obj_align(this->btn_stop_, LV_ALIGN_CENTER, start_x + btn_size + btn_spacing, btn_y);
  lv_obj_set_style_radius(this->btn_stop_, btn_size / 2, 0);
  lv_obj_set_style_bg_color(this->btn_stop_, lv_color_hex(0xEB8429), 0);
  lv_obj_set_style_border_width(this->btn_stop_, 2, 0);
  lv_obj_set_style_border_color(this->btn_stop_, lv_color_hex(0xEB8429), 0);
  lv_obj_set_user_data(this->btn_stop_, this);
  lv_obj_add_event_cb(this->btn_stop_, btn_stop_event_cb, LV_EVENT_CLICKED, nullptr);
  
  lv_obj_t *label_stop = lv_label_create(this->btn_stop_);
  lv_label_set_text(label_stop, LV_SYMBOL_STOP);
  lv_obj_center(label_stop);
  lv_obj_set_style_text_color(label_stop, lv_color_hex(0xFFFFFF), 0);
  
  // Close button (down arrow)
  this->btn_close_ = lv_btn_create(this->page_);
  lv_obj_set_size(this->btn_close_, btn_size, btn_size);
  lv_obj_align(this->btn_close_, LV_ALIGN_CENTER, start_x + (btn_size + btn_spacing) * 2, btn_y);
  lv_obj_set_style_radius(this->btn_close_, btn_size / 2, 0);
  lv_obj_set_style_bg_color(this->btn_close_, lv_color_hex(0xFD5C4C), 0);
  lv_obj_set_style_border_width(this->btn_close_, 2, 0);
  lv_obj_set_style_border_color(this->btn_close_, lv_color_hex(0xFD5C4C), 0);
  lv_obj_set_user_data(this->btn_close_, this);
  lv_obj_add_event_cb(this->btn_close_, btn_close_event_cb, LV_EVENT_CLICKED, nullptr);
  
  lv_obj_t *label_close = lv_label_create(this->btn_close_);
  lv_label_set_text(label_close, LV_SYMBOL_DOWN);
  lv_obj_center(label_close);
  lv_obj_set_style_text_color(label_close, lv_color_hex(0xFFFFFF), 0);
  
  // Dots indicator (pagination) - only if multiple covers
  if (this->covers_.size() > 1) {
    this->dots_container_ = lv_obj_create(this->page_);
    int container_width = this->covers_.size() * 16;
    lv_obj_set_size(this->dots_container_, container_width, 12);
    lv_obj_align(this->dots_container_, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(this->dots_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(this->dots_container_, 0, 0);
    lv_obj_set_style_pad_all(this->dots_container_, 0, 0);
    lv_obj_clear_flag(this->dots_container_, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create dots for each cover
    int dot_spacing = 16;
    int start_dot_x = (container_width - (this->covers_.size() * dot_spacing - (dot_spacing - 8))) / 2;
    for (size_t i = 0; i < this->covers_.size(); i++) {
      lv_obj_t *dot = lv_obj_create(this->dots_container_);
      lv_obj_set_size(dot, 8, 8);
      lv_obj_set_pos(dot, start_dot_x + i * dot_spacing, 2);
      lv_obj_set_style_radius(dot, 4, 0);
      lv_obj_set_style_border_width(dot, 0, 0);
      lv_obj_set_style_bg_color(dot, lv_color_hex(0x555555), 0);
      this->dots_.push_back(dot);
    }
  }
  
  // Register state callbacks for all covers
  for (size_t i = 0; i < this->covers_.size(); i++) {
    if (this->covers_[i].cover != nullptr) {
      this->covers_[i].cover->add_on_state_callback([this]() {
        // Only update if this app is currently active
        if (g_current_cover_app == this) {
          ESP_LOGD(TAG, "Cover state changed callback, refreshing UI");
          this->update_state();
        }
      });
    }
  }
  
  // Set initial state
  this->update_state();
  this->update_dots();
  this->update_action_focus();
  
  ESP_LOGI(TAG, "Cover App UI created");
}

void CoverApp::update_state() {
  if (this->covers_.empty() || this->page_ == nullptr) {
    return;
  }
  
  // Get current cover
  CoverItem &current = this->covers_[this->current_index_];
  
  if (current.cover == nullptr) {
    return;
  }
  
  // Update name label
  if (this->name_label_ != nullptr) {
    lv_label_set_text(this->name_label_, current.name.c_str());
  }
  
  // Get cover state
  float position = current.cover->position;  // 0.0 = closed, 1.0 = open
  cover::CoverOperation operation = current.cover->current_operation;
  
  // Update arc color based on cover's color
  if (this->position_arc_ != nullptr) {
    lv_obj_set_style_arc_color(this->position_arc_, lv_color_hex(current.color), LV_PART_INDICATOR);
    
    // Position is 0-1, convert to 0-100 for arc
    int arc_value = (int)(position * 100);
    lv_arc_set_value(this->position_arc_, arc_value);
  }
  
  // Update position label
  if (this->position_label_ != nullptr) {
    if (position == cover::COVER_OPEN) {
      lv_label_set_text(this->position_label_, "100%");
    } else if (position == cover::COVER_CLOSED) {
      lv_label_set_text(this->position_label_, "0%");
    } else {
      char buf[8];
      snprintf(buf, sizeof(buf), "%d%%", (int)(position * 100));
      lv_label_set_text(this->position_label_, buf);
    }
  }
  
  // Update status label
  if (this->status_label_ != nullptr) {
    lv_label_set_text(this->status_label_, this->get_state_text(operation, position));
  }
  
  ESP_LOGD(TAG, "Cover '%s' position: %.0f%%, operation: %d", 
           current.name.c_str(), position * 100, (int)operation);
}

const char* CoverApp::get_state_text(cover::CoverOperation op, float position) {
  switch (op) {
    case cover::COVER_OPERATION_OPENING:
      return "Opening...";
    case cover::COVER_OPERATION_CLOSING:
      return "Closing...";
    case cover::COVER_OPERATION_IDLE:
    default:
      if (position >= 0.99f) {
        return "Open";
      } else if (position <= 0.01f) {
        return "Closed";
      } else {
        return "Partial";
      }
  }
}

void CoverApp::update_dots() {
  if (this->dots_.empty()) return;
  
  for (size_t i = 0; i < this->dots_.size(); i++) {
    if (i == (size_t)this->current_index_) {
      lv_obj_set_style_bg_color(this->dots_[i], lv_color_hex(0xFFFFFF), 0);
    } else {
      lv_obj_set_style_bg_color(this->dots_[i], lv_color_hex(0x555555), 0);
    }
  }
}

void CoverApp::update_action_focus() {
  // Reset all buttons to normal state
  if (this->btn_open_) {
    lv_obj_set_style_border_color(this->btn_open_, lv_color_hex(0x03A964), 0);
    lv_obj_set_style_border_width(this->btn_open_, 2, 0);
  }
  if (this->btn_stop_) {
    lv_obj_set_style_border_color(this->btn_stop_, lv_color_hex(0xEB8429), 0);
    lv_obj_set_style_border_width(this->btn_stop_, 2, 0);
  }
  if (this->btn_close_) {
    lv_obj_set_style_border_color(this->btn_close_, lv_color_hex(0xFD5C4C), 0);
    lv_obj_set_style_border_width(this->btn_close_, 2, 0);
  }
  
  // Highlight selected action
  lv_obj_t *selected_btn = nullptr;
  switch (this->selected_action_) {
    case CoverAction::OPEN:
      selected_btn = this->btn_open_;
      break;
    case CoverAction::STOP:
      selected_btn = this->btn_stop_;
      break;
    case CoverAction::CLOSE:
      selected_btn = this->btn_close_;
      break;
  }
  
  if (selected_btn) {
    lv_obj_set_style_border_color(selected_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(selected_btn, 3, 0);
  }
}

void CoverApp::open_cover() {
  if (this->covers_.empty()) {
    ESP_LOGW(TAG, "No covers configured");
    return;
  }
  
  CoverItem &current = this->covers_[this->current_index_];
  if (current.cover == nullptr) {
    ESP_LOGW(TAG, "Current cover is null");
    return;
  }
  
  ESP_LOGI(TAG, "Opening cover: %s", current.name.c_str());
  auto call = current.cover->make_call();
  call.set_command_open();
  call.perform();
}

void CoverApp::close_cover() {
  if (this->covers_.empty()) {
    ESP_LOGW(TAG, "No covers configured");
    return;
  }
  
  CoverItem &current = this->covers_[this->current_index_];
  if (current.cover == nullptr) {
    ESP_LOGW(TAG, "Current cover is null");
    return;
  }
  
  ESP_LOGI(TAG, "Closing cover: %s", current.name.c_str());
  auto call = current.cover->make_call();
  call.set_command_close();
  call.perform();
}

void CoverApp::stop_cover() {
  if (this->covers_.empty()) {
    ESP_LOGW(TAG, "No covers configured");
    return;
  }
  
  CoverItem &current = this->covers_[this->current_index_];
  if (current.cover == nullptr) {
    ESP_LOGW(TAG, "Current cover is null");
    return;
  }
  
  ESP_LOGI(TAG, "Stopping cover: %s", current.name.c_str());
  auto call = current.cover->make_call();
  call.set_command_stop();
  call.perform();
}

void CoverApp::toggle_cover() {
  if (this->covers_.empty()) {
    ESP_LOGW(TAG, "No covers configured");
    return;
  }
  
  CoverItem &current = this->covers_[this->current_index_];
  if (current.cover == nullptr) {
    ESP_LOGW(TAG, "Current cover is null");
    return;
  }
  
  ESP_LOGI(TAG, "Toggling cover: %s", current.name.c_str());
  
  // Toggle logic: if mostly open -> close, if mostly closed -> open
  if (current.cover->position > 0.5f) {
    auto call = current.cover->make_call();
    call.set_command_close();
    call.perform();
  } else {
    auto call = current.cover->make_call();
    call.set_command_open();
    call.perform();
  }
}

void CoverApp::btn_open_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  CoverApp *app = static_cast<CoverApp *>(lv_obj_get_user_data(btn));
  
  if (app != nullptr) {
    app->open_cover();
  }
}

void CoverApp::btn_stop_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  CoverApp *app = static_cast<CoverApp *>(lv_obj_get_user_data(btn));
  
  if (app != nullptr) {
    app->stop_cover();
  }
}

void CoverApp::btn_close_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  CoverApp *app = static_cast<CoverApp *>(lv_obj_get_user_data(btn));
  
  if (app != nullptr) {
    app->close_cover();
  }
}

}  // namespace dial_menu
}  // namespace esphome

#endif  // USE_DIAL_MENU_COVER
