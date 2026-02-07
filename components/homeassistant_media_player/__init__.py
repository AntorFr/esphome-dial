import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_ENTITY_ID

DEPENDENCIES = ["api"]
CODEOWNERS = ["@dial-menu"]

homeassistant_media_player_ns = cg.esphome_ns.namespace("homeassistant_media_player")
HomeassistantMediaPlayer = homeassistant_media_player_ns.class_(
    "HomeassistantMediaPlayer", cg.Component
)

CONF_VOLUME_STEP = "volume_step"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HomeassistantMediaPlayer),
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_VOLUME_STEP, default=0.05): cv.float_range(min=0.01, max=0.2),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cg.add(var.set_entity_id(config[CONF_ENTITY_ID]))
    cg.add(var.set_volume_step(config[CONF_VOLUME_STEP]))
