import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import lock

from .. import CONF_GOLMAR_UNO_ID, GolmarUnoComponent, golmar_uno_ns

DEPENDENCIES = ["golmar_uno"]

DoorLock = golmar_uno_ns.class_("DoorLock", lock.Lock)

CONFIG_SCHEMA = lock.lock_schema(DoorLock).extend(
    {
        cv.GenerateID(CONF_GOLMAR_UNO_ID): cv.use_id(GolmarUnoComponent),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOLMAR_UNO_ID])
    lck = await lock.new_lock(config)
    await cg.register_parented(lck, hub)
    cg.add(hub.set_door_lock(lck))