#include "homeassistant_cover.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace homeassistant_addon {

static const char *const TAG = "homeassistant_addon.cover";

void HomeassistantCover::setup() {
  ESP_LOGD(TAG, "Setting up HomeAssistant Cover '%s' for entity '%s'", 
           this->get_name().c_str(), this->entity_id_.c_str());
  
  // Subscribe to state changes
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, nullopt,
      [this](const std::string &state) {
        this->on_state_received(state);
      });
  
  // Subscribe to position attribute
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, std::string("current_position"),
      [this](const std::string &position) {
        this->on_position_received(position);
      });
  
  // Subscribe to tilt attribute (optional)
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, std::string("current_tilt_position"),
      [this](const std::string &tilt) {
        if (!tilt.empty() && tilt != "unavailable" && tilt != "unknown") {
          this->supports_tilt_ = true;
          auto val = parse_number<float>(tilt);
          if (val.has_value()) {
            this->tilt = val.value() / 100.0f;
            this->publish_state(false);
          }
        }
      });
}

void HomeassistantCover::on_state_received(const std::string &state) {
  ESP_LOGD(TAG, "'%s' received state: %s", this->entity_id_.c_str(), state.c_str());
  
  if (state == "open") {
    this->position = cover::COVER_OPEN;
    this->current_operation = cover::COVER_OPERATION_IDLE;
  } else if (state == "closed") {
    this->position = cover::COVER_CLOSED;
    this->current_operation = cover::COVER_OPERATION_IDLE;
  } else if (state == "opening") {
    this->current_operation = cover::COVER_OPERATION_OPENING;
  } else if (state == "closing") {
    this->current_operation = cover::COVER_OPERATION_CLOSING;
  } else if (state == "unavailable" || state == "unknown") {
    ESP_LOGW(TAG, "'%s' state is %s", this->entity_id_.c_str(), state.c_str());
    return;
  }
  
  this->publish_state(false);
}

void HomeassistantCover::on_position_received(const std::string &position_str) {
  if (position_str.empty() || position_str == "unavailable" || position_str == "unknown") {
    return;
  }
  
  this->supports_position_ = true;
  auto val = parse_number<float>(position_str);
  if (val.has_value()) {
    // HA position: 0 = closed, 100 = open
    // ESPHome position: 0.0 = closed, 1.0 = open
    this->position = val.value() / 100.0f;
    ESP_LOGD(TAG, "'%s' received position: %.0f%% -> %.2f", 
             this->entity_id_.c_str(), val.value(), this->position);
    this->publish_state(false);
  }
}

cover::CoverTraits HomeassistantCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_stop(this->supports_stop_);
  traits.set_supports_position(this->supports_position_);
  traits.set_supports_tilt(this->supports_tilt_);
  traits.set_is_assumed_state(false);
  return traits;
}

void HomeassistantCover::control(const cover::CoverCall &call) {
  api::HomeassistantServiceResponse resp;
  resp.service = "cover.";
  
  if (call.get_stop()) {
    resp.service += "stop_cover";
    resp.data.push_back(api::HomeassistantServiceMap{
        .key = "entity_id",
        .value = this->entity_id_,
    });
  } else if (call.get_position().has_value()) {
    float pos = call.get_position().value();
    
    if (pos == cover::COVER_OPEN) {
      resp.service += "open_cover";
      resp.data.push_back(api::HomeassistantServiceMap{
          .key = "entity_id",
          .value = this->entity_id_,
      });
    } else if (pos == cover::COVER_CLOSED) {
      resp.service += "close_cover";
      resp.data.push_back(api::HomeassistantServiceMap{
          .key = "entity_id",
          .value = this->entity_id_,
      });
    } else {
      // Set specific position
      resp.service += "set_cover_position";
      resp.data.push_back(api::HomeassistantServiceMap{
          .key = "entity_id",
          .value = this->entity_id_,
      });
      resp.data.push_back(api::HomeassistantServiceMap{
          .key = "position",
          .value = to_string(static_cast<int>(pos * 100)),
      });
    }
  } else if (call.get_tilt().has_value()) {
    float tilt = call.get_tilt().value();
    resp.service += "set_cover_tilt_position";
    resp.data.push_back(api::HomeassistantServiceMap{
        .key = "entity_id",
        .value = this->entity_id_,
    });
    resp.data.push_back(api::HomeassistantServiceMap{
        .key = "tilt_position",
        .value = to_string(static_cast<int>(tilt * 100)),
    });
  } else {
    ESP_LOGW(TAG, "Unknown cover control command");
    return;
  }
  
  ESP_LOGD(TAG, "Calling service: %s", resp.service.c_str());
  api::global_api_server->send_homeassistant_service_call(resp);
}

void HomeassistantCover::dump_config() {
  ESP_LOGCONFIG(TAG, "HomeAssistant Cover '%s':", this->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  Entity ID: %s", this->entity_id_.c_str());
}

}  // namespace homeassistant_addon
}  // namespace esphome
