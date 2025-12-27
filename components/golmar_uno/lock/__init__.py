import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import lock

from .. import GolmarUnoComponent, CONF_GOLMAR_UNO_ID,golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

door_lock_ns = golmar_uno_ns.class_("door_lock", lock.Lock)

CONFIG_SCHEMA = lock.lock_schema(door_lock_ns).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    b = await lock.new_lock(config)
    await cg.register_parented(b, hub)
    cg.add(hub.set_door_lock_(b))





    