import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import GolmarUnoComponent, CONF_GOLMAR_UNO_ID, golmar_uno_ns


DEPENDENCIES = ["golmar_uno"]

calling_alert_ns = golmar_uno_ns.class_("calling_alert", binary_sensor.BinarySensor)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(calling_alert_ns).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    b = await binary_sensor.new_binary_sensor(config)
    await cg.register_parented(b, hub)
    cg.add(hub.set_calling_alert_binary_sensor_(b))
