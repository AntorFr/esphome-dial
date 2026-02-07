#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/api/api_server.h"

namespace esphome {
namespace homeassistant_cover {

class HomeassistantCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }
  
  void set_entity_id(const std::string &entity_id) { entity_id_ = entity_id; }
  const std::string &get_entity_id() const { return entity_id_; }
  
  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;
  
  void on_state_received(const std::string &state);
  void on_position_received(const std::string &position_str);
  
  std::string entity_id_;
  
  // Traits detected from HA
  bool supports_position_{false};
  bool supports_tilt_{false};
  bool supports_stop_{true};
};

}  // namespace homeassistant_cover
}  // namespace esphome
