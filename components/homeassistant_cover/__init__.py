import esphome.codegen as cg
from esphome.components import cover
import esphome.config_validation as cv
from esphome.const import CONF_ENTITY_ID, CONF_ID, CONF_INTERNAL

DEPENDENCIES = ["api"]
CODEOWNERS = ["@AntorFr"]

homeassistant_cover_ns = cg.esphome_ns.namespace("homeassistant_cover")
HomeassistantCover = homeassistant_cover_ns.class_(
    "HomeassistantCover", cover.Cover, cg.Component
)

CONFIG_SCHEMA = cover.cover_schema(HomeassistantCover).extend(
    {
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_INTERNAL, default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)
    
    cg.add(var.set_entity_id(config[CONF_ENTITY_ID]))
