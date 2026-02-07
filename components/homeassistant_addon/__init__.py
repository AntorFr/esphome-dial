"""
homeassistant_addon component for ESPHome
Provides cover, climate, and media_player entities imported from Home Assistant.

These entity types are not natively available in ESPHome's homeassistant platform.

Example:
  homeassistant_addon:
    covers:
      - id: my_gate
        entity_id: cover.front_gate
    climates:
      - id: my_thermostat
        entity_id: climate.living_room
    media_players:
      - id: my_speaker
        entity_id: media_player.living_room
"""
import esphome.codegen as cg
from esphome.components import cover, climate
import esphome.config_validation as cv
from esphome.const import CONF_ENTITY_ID, CONF_ID, CONF_INTERNAL

DEPENDENCIES = ["api"]
CODEOWNERS = ["@AntorFr"]
MULTI_CONF = True

# Namespace
homeassistant_addon_ns = cg.esphome_ns.namespace("homeassistant_addon")

# Classes
HomeassistantCover = homeassistant_addon_ns.class_(
    "HomeassistantCover", cover.Cover, cg.Component
)
HomeassistantClimate = homeassistant_addon_ns.class_(
    "HomeassistantClimate", climate.Climate, cg.Component
)
HomeassistantMediaPlayer = homeassistant_addon_ns.class_(
    "HomeassistantMediaPlayer", cg.Component
)

# Configuration keys
CONF_COVERS = "covers"
CONF_CLIMATES = "climates"
CONF_MEDIA_PLAYERS = "media_players"
CONF_TEMPERATURE_STEP = "temperature_step"
CONF_MIN_TEMPERATURE = "min_temperature"
CONF_MAX_TEMPERATURE = "max_temperature"
CONF_VOLUME_STEP = "volume_step"

# Cover schema
COVER_SCHEMA = cover.cover_schema(HomeassistantCover).extend(
    {
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_INTERNAL, default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)

# Climate schema
CLIMATE_SCHEMA = climate.climate_schema(HomeassistantClimate).extend(
    {
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_INTERNAL, default=True): cv.boolean,
        cv.Optional(CONF_TEMPERATURE_STEP, default=0.5): cv.float_range(min=0.1, max=2.0),
        cv.Optional(CONF_MIN_TEMPERATURE, default=7.0): cv.float_range(min=-20, max=50),
        cv.Optional(CONF_MAX_TEMPERATURE, default=35.0): cv.float_range(min=-20, max=50),
    }
).extend(cv.COMPONENT_SCHEMA)

# Media player schema
MEDIA_PLAYER_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HomeassistantMediaPlayer),
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_INTERNAL, default=True): cv.boolean,
        cv.Optional(CONF_VOLUME_STEP, default=0.05): cv.float_range(min=0.01, max=0.2),
    }
).extend(cv.COMPONENT_SCHEMA)

# Main schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_COVERS): cv.ensure_list(COVER_SCHEMA),
        cv.Optional(CONF_CLIMATES): cv.ensure_list(CLIMATE_SCHEMA),
        cv.Optional(CONF_MEDIA_PLAYERS): cv.ensure_list(MEDIA_PLAYER_SCHEMA),
    }
)


async def to_code(config):
    # Process covers
    for conf in config.get(CONF_COVERS, []):
        var = await cover.new_cover(conf)
        await cg.register_component(var, conf)
        cg.add(var.set_entity_id(conf[CONF_ENTITY_ID]))
    
    # Process climates
    for conf in config.get(CONF_CLIMATES, []):
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await climate.register_climate(var, conf)
        
        cg.add(var.set_entity_id(conf[CONF_ENTITY_ID]))
        cg.add(var.set_temperature_step(conf[CONF_TEMPERATURE_STEP]))
        cg.add(var.set_min_temperature(conf[CONF_MIN_TEMPERATURE]))
        cg.add(var.set_max_temperature(conf[CONF_MAX_TEMPERATURE]))
    
    # Process media players
    for conf in config.get(CONF_MEDIA_PLAYERS, []):
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        
        cg.add(var.set_entity_id(conf[CONF_ENTITY_ID]))
        cg.add(var.set_volume_step(conf[CONF_VOLUME_STEP]))
