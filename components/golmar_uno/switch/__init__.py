import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import ENTITY_CATEGORY_NONE

from .. import CONF_GOLMAR_UNO_ID, GolmarUnoComponent, golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

UnlockDoorSwitch = golmar_uno_ns.class_(
    "UnlockDoorSwitch", cg.Component, switch.Switch
)

CONFIG_SCHEMA = switch.switch_schema(
    UnlockDoorSwitch,
    entity_category=ENTITY_CATEGORY_NONE,
).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    sw = await switch.new_switch(config)
    await cg.register_component(sw, config)
    await cg.register_parented(sw, hub)
    cg.add(hub.set_unlock_door_switch(sw))
