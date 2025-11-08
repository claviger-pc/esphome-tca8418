# components/tca8418/binary_sensor.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

from . import tca8418_ns, TCA8418Component

CONF_TCA8418_ID = "tca8418_id"
CONF_KEY = "key"

TCA8418BinarySensor = tca8418_ns.class_(
    "TCA8418BinarySensor",
    binary_sensor.BinarySensor,
    cg.Parented(TCA8418Component),
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(TCA8418BinarySensor).extend(
    {
        cv.GenerateID(CONF_TCA8418_ID): cv.use_id(TCA8418Component),
        cv.Required(CONF_KEY): cv.int_range(min=0, max=63),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_TCA8418_ID])
    var = await binary_sensor.new_binary_sensor(config)

    # connect child -> parent
    cg.add(var.set_parent(hub))
    cg.add(var.set_key_index(config[CONF_KEY]))

    # register this key with the hub
    cg.add(hub.register_key_sensor(config[CONF_KEY], var))
