import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from .. import GolmarUnoComponent, CONF_GOLMAR_UNO_ID,golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

unlock_door_button_ns = golmar_uno_ns.class_("unlock_door_button", button.Button)

CONFIG_SCHEMA = button.button_schema(unlock_door_button_ns).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    b = await button.new_button(config)
    await cg.register_parented(b, hub)
    cg.add(hub.set_unlock_door_button_(b))





    