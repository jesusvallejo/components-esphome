import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from .. import T740UNO, CONF_T740UNO_ID,t740uno_ns

DEPENDENCIES = ["t740uno"]

OpenDoorButton = t740uno_ns.class_("OpenDoorButton", button.Button)

CONFIG_SCHEMA = button.button_schema(OpenDoorButton).extend(
    {
        cv.GenerateID(CONF_T740UNO_ID): cv.use_id(T740UNO),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_T740UNO_ID])
    b = await button.new_button(config)
    await cg.register_parented(b,config[CONF_T740UNO_ID])
    cg.add(hub.set_open_door_button(b))