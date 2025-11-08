// components/tca8418/tca8418.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace tca8418 {

class TCA8418Component : public Component, public i2c::I2CDevice {
 public:
  void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }

  void register_key_sensor(uint8_t index, binary_sensor::BinarySensor *sensor);

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  static const uint8_t MAX_KEYS = 64;

  binary_sensor::BinarySensor *key_sensors_[MAX_KEYS] = {nullptr};

  // For now we assume a fixed 6x4 matrix (0..23) for OMOTE;
  // you can make this configurable later if you want.
  uint8_t rows_{6};
  uint8_t cols_{4};
  InternalGPIOPin *interrupt_pin_{nullptr};
  uint32_t last_poll_{0};

  bool read_key_event_(uint8_t &key_event);
  void process_key_event_(uint8_t key_event);
};

}  // namespace tca8418
}  // namespace esphome
