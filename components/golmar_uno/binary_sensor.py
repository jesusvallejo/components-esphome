import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import GolmarUnoComponent, CONF_GOLMAR_UNO_ID


DEPENDENCIES = ["golmar_uno"]

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(binary_sensor.BinarySensor).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    b = await binary_sensor.new_binary_sensor(config)
    cg.add(hub.set_calling_alert_binary_sensor_(b))
