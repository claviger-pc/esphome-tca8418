# components/tca8418/binary_sensor.py

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

from . import tca8418_ns, TCA8418Component

CONF_TCA8418_ID = "tca8418_id"
CONF_KEY = "key"

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend(
    {
        cv.GenerateID(CONF_TCA8418_ID): cv.use_id(TCA8418Component),
        cv.Required(CONF_KEY): cv.int_range(min=0, max=63),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_TCA8418_ID])
    var = await binary_sensor.new_binary_sensor(config)

    key = config[CONF_KEY]
    cg.add(hub.register_key_sensor(key, var))
