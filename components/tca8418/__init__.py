# components/tca8418/__init__.py

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome import pins
from esphome.const import CONF_ID

tca8418_ns = cg.esphome_ns.namespace("tca8418")
TCA8418Component = tca8418_ns.class_("TCA8418Component", cg.Component, i2c.I2CDevice)

CONF_INTERRUPT_PIN = "interrupt_pin"

CONFIG_SCHEMA = (
    i2c.i2c_device_schema(TCA8418Component)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(TCA8418Component),
            cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        }
    )
)

async def to_code(config):
    var = await i2c.register_i2c_device(config, TCA8418Component)

    if CONF_INTERRUPT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_interrupt_pin(pin))

    cg.add_global(cg.App.register_component(var))
