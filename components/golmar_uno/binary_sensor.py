import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
# from esphome.const import DEVICE_CLASS_DOORBELL
from . import GolmarUnoComponent, CONF_GOLMAR_UNO_ID,golmar_uno_ns


DEPENDENCIES = ["golmar_uno"]

incoming_call = golmar_uno_ns.class_("incoming_call", binary_sensor.BinarySensor)

CONFIG_SCHEMA = binary_sensor.binary_sensor(incoming_call).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    b = await binary_sensor.new_binary_sensor(config)
    await cg.register_parented(b, hub)
    cg.add(hub.set_incoming_call(b))
