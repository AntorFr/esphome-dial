/**
 * @file homeassistant_climate.cpp
 * @brief Implementation of Home Assistant Climate component
 */

#include "homeassistant_climate.h"
#include "esphome/components/api/api_server.h"
#include "esphome/core/log.h"

namespace esphome {
namespace homeassistant_climate {

static const char *const TAG = "homeassistant_climate";

void HomeassistantClimate::setup() {
  ESP_LOGI(TAG, "Setting up Home Assistant Climate '%s'...", this->entity_id_);
  
  // Subscribe to the main state (hvac_mode)
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, optional<std::string>(), 
      [this](const std::string &state) {
        ESP_LOGD(TAG, "'%s': Got state: %s", this->entity_id_, state.c_str());
        this->parse_hvac_mode(state);
        this->received_state_ = true;
        this->publish_state();
      });
  
  // Subscribe to current_temperature attribute
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, std::string("current_temperature"),
      [this](const std::string &state) {
        ESP_LOGD(TAG, "'%s': Got current_temperature: %s", this->entity_id_, state.c_str());
        this->parse_current_temperature(state);
        this->publish_state();
      });
  
  // Subscribe to temperature (target) attribute
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, std::string("temperature"),
      [this](const std::string &state) {
        ESP_LOGD(TAG, "'%s': Got target temperature: %s", this->entity_id_, state.c_str());
        this->parse_target_temperature(state);
        this->publish_state();
      });
  
  // Subscribe to hvac_action attribute (heating, cooling, idle, off)
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, std::string("hvac_action"),
      [this](const std::string &state) {
        ESP_LOGD(TAG, "'%s': Got hvac_action: %s", this->entity_id_, state.c_str());
        this->parse_hvac_action(state);
        this->publish_state();
      });
}

void HomeassistantClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "Home Assistant Climate:");
  ESP_LOGCONFIG(TAG, "  Entity ID: '%s'", this->entity_id_);
  ESP_LOGCONFIG(TAG, "  Temperature Step: %.1f", this->temperature_step_);
  ESP_LOGCONFIG(TAG, "  Min Temperature: %.1f", this->min_temperature_);
  ESP_LOGCONFIG(TAG, "  Max Temperature: %.1f", this->max_temperature_);
}

float HomeassistantClimate::get_setup_priority() const {
  return setup_priority::AFTER_CONNECTION;
}

climate::ClimateTraits HomeassistantClimate::traits() {
  auto traits = climate::ClimateTraits();
  
  // Supported modes
  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT_COOL,
      climate::CLIMATE_MODE_AUTO,
  });
  
  // Temperature settings
  traits.set_supports_current_temperature(true);
  traits.set_supports_two_point_target_temperature(false);
  traits.set_visual_min_temperature(this->min_temperature_);
  traits.set_visual_max_temperature(this->max_temperature_);
  traits.set_visual_temperature_step(this->temperature_step_);
  
  // Actions
  traits.set_supported_custom_presets({});  // Could add preset support later
  
  return traits;
}

void HomeassistantClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = *call.get_mode();
    ESP_LOGI(TAG, "Setting HVAC mode to: %s", climate::climate_mode_to_string(mode));
    this->send_set_hvac_mode(mode);
    this->mode = mode;
  }
  
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    ESP_LOGI(TAG, "Setting target temperature to: %.1f", temp);
    this->send_set_temperature(temp);
    this->target_temperature = temp;
  }
  
  this->publish_state();
}

void HomeassistantClimate::send_set_temperature(float temperature) {
  if (!api::global_api_server->is_connected()) {
    ESP_LOGE(TAG, "No clients connected to API server");
    return;
  }
  
  api::HomeassistantServiceResponse resp;
  resp.service = "climate.set_temperature";
  resp.is_event = false;
  
  // Entity ID
  api::HomeassistantServiceMap entity_id_kv;
  entity_id_kv.key = "entity_id";
  entity_id_kv.value = this->entity_id_;
  resp.data.push_back(entity_id_kv);
  
  // Temperature
  api::HomeassistantServiceMap temp_kv;
  temp_kv.key = "temperature";
  char temp_str[16];
  snprintf(temp_str, sizeof(temp_str), "%.1f", temperature);
  temp_kv.value = temp_str;
  resp.data.push_back(temp_kv);
  
  api::global_api_server->send_homeassistant_service_call(resp);
}

void HomeassistantClimate::send_set_hvac_mode(climate::ClimateMode mode) {
  if (!api::global_api_server->is_connected()) {
    ESP_LOGE(TAG, "No clients connected to API server");
    return;
  }
  
  api::HomeassistantServiceResponse resp;
  resp.service = "climate.set_hvac_mode";
  resp.is_event = false;
  
  // Entity ID
  api::HomeassistantServiceMap entity_id_kv;
  entity_id_kv.key = "entity_id";
  entity_id_kv.value = this->entity_id_;
  resp.data.push_back(entity_id_kv);
  
  // HVAC mode
  api::HomeassistantServiceMap mode_kv;
  mode_kv.key = "hvac_mode";
  mode_kv.value = this->esphome_mode_to_ha(mode);
  resp.data.push_back(mode_kv);
  
  api::global_api_server->send_homeassistant_service_call(resp);
}

void HomeassistantClimate::parse_current_temperature(const std::string &state) {
  if (state.empty() || state == "unknown" || state == "unavailable") {
    return;
  }
  
  char *end;
  float value = strtof(state.c_str(), &end);
  if (end != state.c_str()) {
    this->current_temperature = value;
  }
}

void HomeassistantClimate::parse_target_temperature(const std::string &state) {
  if (state.empty() || state == "unknown" || state == "unavailable") {
    return;
  }
  
  char *end;
  float value = strtof(state.c_str(), &end);
  if (end != state.c_str()) {
    this->target_temperature = value;
  }
}

void HomeassistantClimate::parse_hvac_mode(const std::string &state) {
  if (state.empty() || state == "unknown" || state == "unavailable") {
    return;
  }
  
  this->mode = ha_mode_to_esphome(state);
}

void HomeassistantClimate::parse_hvac_action(const std::string &state) {
  if (state.empty() || state == "unknown" || state == "unavailable") {
    return;
  }
  
  this->action = ha_action_to_esphome(state);
}

climate::ClimateMode HomeassistantClimate::ha_mode_to_esphome(const std::string &mode) {
  if (mode == "off") {
    return climate::CLIMATE_MODE_OFF;
  } else if (mode == "heat") {
    return climate::CLIMATE_MODE_HEAT;
  } else if (mode == "cool") {
    return climate::CLIMATE_MODE_COOL;
  } else if (mode == "heat_cool" || mode == "auto") {
    return climate::CLIMATE_MODE_HEAT_COOL;
  } else if (mode == "dry") {
    return climate::CLIMATE_MODE_DRY;
  } else if (mode == "fan_only") {
    return climate::CLIMATE_MODE_FAN_ONLY;
  }
  
  ESP_LOGW(TAG, "Unknown HVAC mode: %s", mode.c_str());
  return climate::CLIMATE_MODE_OFF;
}

const char* HomeassistantClimate::esphome_mode_to_ha(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_OFF:
      return "off";
    case climate::CLIMATE_MODE_HEAT:
      return "heat";
    case climate::CLIMATE_MODE_COOL:
      return "cool";
    case climate::CLIMATE_MODE_HEAT_COOL:
      return "heat_cool";
    case climate::CLIMATE_MODE_AUTO:
      return "auto";
    case climate::CLIMATE_MODE_DRY:
      return "dry";
    case climate::CLIMATE_MODE_FAN_ONLY:
      return "fan_only";
    default:
      return "off";
  }
}

climate::ClimateAction HomeassistantClimate::ha_action_to_esphome(const std::string &action) {
  if (action == "off") {
    return climate::CLIMATE_ACTION_OFF;
  } else if (action == "heating") {
    return climate::CLIMATE_ACTION_HEATING;
  } else if (action == "cooling") {
    return climate::CLIMATE_ACTION_COOLING;
  } else if (action == "idle") {
    return climate::CLIMATE_ACTION_IDLE;
  } else if (action == "drying") {
    return climate::CLIMATE_ACTION_DRYING;
  } else if (action == "fan") {
    return climate::CLIMATE_ACTION_FAN;
  }
  
  return climate::CLIMATE_ACTION_OFF;
}

}  // namespace homeassistant_climate
}  // namespace esphome
