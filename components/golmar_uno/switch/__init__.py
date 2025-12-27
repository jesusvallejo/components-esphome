import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from .. import GolmarUnoComponent, CONF_GOLMAR_UNO_ID, golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

unlock_door_switch_ns = golmar_uno_ns.class_("unlock_door_switch", switch.Switch)

CONFIG_SCHEMA = switch.switch_schema(unlock_door_switch_ns).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    sw = await switch.new_switch(config)
    await cg.register_parented(sw, hub)
    cg.add(hub.set_unlock_door_switch_(sw))
