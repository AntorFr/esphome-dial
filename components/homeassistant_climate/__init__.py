"""
homeassistant_climate component for ESPHome
Import and control Home Assistant climate entities from ESPHome.

This allows ESPHome devices to control thermostats, HVAC systems, etc. from Home Assistant.

Example:
  homeassistant_climate:
    - id: living_room_thermostat
      entity_id: climate.living_room

Features:
  - Read current temperature
  - Read/set target temperature
  - Read/set HVAC mode (off, heat, cool, auto, etc.)
  - Read current action (heating, cooling, idle)
  - Support for temperature step configuration
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
    CONF_ENTITY_ID,
)

CODEOWNERS = ["@antorfr"]
DEPENDENCIES = ["api"]

# Configuration keys
CONF_TEMPERATURE_STEP = "temperature_step"
CONF_MIN_TEMPERATURE = "min_temperature"
CONF_MAX_TEMPERATURE = "max_temperature"

# Namespace
homeassistant_climate_ns = cg.esphome_ns.namespace("homeassistant_climate")
HomeassistantClimate = homeassistant_climate_ns.class_(
    "HomeassistantClimate", climate.Climate, cg.Component
)

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(HomeassistantClimate).extend(
        {
            cv.Required(CONF_ENTITY_ID): cv.entity_id,
            cv.Optional(CONF_TEMPERATURE_STEP, default=0.5): cv.float_range(min=0.1, max=5.0),
            cv.Optional(CONF_MIN_TEMPERATURE, default=7.0): cv.float_,
            cv.Optional(CONF_MAX_TEMPERATURE, default=35.0): cv.float_,
        }
    ).extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    
    cg.add(var.set_entity_id(config[CONF_ENTITY_ID]))
    cg.add(var.set_temperature_step(config[CONF_TEMPERATURE_STEP]))
    cg.add(var.set_min_temperature(config[CONF_MIN_TEMPERATURE]))
    cg.add(var.set_max_temperature(config[CONF_MAX_TEMPERATURE]))
