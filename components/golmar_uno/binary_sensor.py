import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
# from esphome.const import DEVICE_CLASS_DOORBELL
from . import GOLMAR_UNO, CONF_GOLMAR_UNO_ID

DEPENDENCIES = ["golmar_uno"]

CONF_CALLING_ALERT = "calling_alert"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GOLMAR_UNO),
        cv.Required(CONF_CALLING_ALERT): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_CALLING_ALERT])
    var = await binary_sensor.new_binary_sensor(config[CONF_CALLING_ALERT])
    cg.add(hub.set_calling_alert_binary_sensor(var))