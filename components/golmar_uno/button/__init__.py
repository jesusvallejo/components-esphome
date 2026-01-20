import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import ENTITY_CATEGORY_NONE

from .. import CONF_GOLMAR_UNO_ID, GolmarUnoComponent, golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

UnlockDoorButton = golmar_uno_ns.class_("UnlockDoorButton", button.Button)

CONFIG_SCHEMA = button.button_schema(
    UnlockDoorButton,
    entity_category=ENTITY_CATEGORY_NONE,
).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    btn = await button.new_button(config)
    await cg.register_parented(btn, hub)
    cg.add(hub.set_unlock_door_button(btn))