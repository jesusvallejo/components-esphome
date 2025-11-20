import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
# from esphome.const import DEVICE_CLASS_DOORBELL
from . import GOLMAR_UNO, CONF_GOLMAR_UNO_ID,golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

GOLMAR_UNO = golmar_uno_ns.class_("GolmarUnoBinarySensor", binary_sensor.BinarySensor)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    # device_class=DEVICE_CLASS_DOORBELL
).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GOLMAR_UNO),
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    await cg.register_parented(var, hub)
    cg.add(hub.set_calling_alert_binary_sensor(var))