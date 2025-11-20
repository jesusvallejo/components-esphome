import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
# from esphome.const import DEVICE_CLASS_DOORBELL
from . import GOLMAR_UNO, CONF_GOLMAR_UNO_ID

DEPENDENCIES = ["golmar_uno"]

binary_sensor_ns = cg.esphome_ns.namespace("binary_sensor")
binary_sensor = binary_sensor.binary_sensor_ns.class_("binary_sensor")
CONF_BINARY_SENSOR = "binary_sensor"

CONFIG_SCHEMA = (
    {
        cv.GenerateID(): cv.declare_id(binary_sensor),
        cv.Required(CONF_BINARY_SENSOR): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    var = await binary_sensor.new_binary_sensor(config)
    cg.add(hub.set_calling_alert_binary_sensor(var))