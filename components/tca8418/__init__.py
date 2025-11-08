# components/tca8418/__init__.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, gpio
from esphome.const import CONF_ID

tca8418_ns = cg.esphome_ns.namespace("tca8418")
TCA8418Component = tca8418_ns.class_("TCA8418Component", cg.Component, i2c.I2CDevice)

CONF_ROWS = "rows"
CONF_COLS = "cols"
CONF_INTERRUPT_PIN = "interrupt_pin"

CONFIG_SCHEMA = (
    i2c.i2c_device_schema(TCA8418Component)
    .extend(
        {
            cv.Optional(CONF_ROWS, default=6): cv.int_range(min=1, max=8),
            cv.Optional(CONF_COLS, default=4): cv.int_range(min=1, max=10),
            cv.Optional(CONF_INTERRUPT_PIN): cv.internal_gpio_input_pin_schema,
        }
    )
)


async def to_code(config):
    var = await i2c.register_i2c_device(config, TCA8418Component)

    cg.add(var.set_rows(config[CONF_ROWS]))
    cg.add(var.set_cols(config[CONF_COLS]))

    if CONF_INTERRUPT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_interrupt_pin(pin))
