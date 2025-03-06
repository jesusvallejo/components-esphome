import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
# from esphome.const import DEVICE_CLASS_DOORBELL
from . import T740UNO, CONF_T740UNO_ID

DEPENDENCIES = ["t740uno"]

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    # device_class=DEVICE_CLASS_DOORBELL
).extend(
    {
        cv.GenerateID(CONF_T740UNO_ID): cv.use_id(T740UNO),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_T740UNO_ID])
    var = await binary_sensor.new_binary_sensor(config)
    cg.add(hub.set_calling_alert_binary_sensor(var))