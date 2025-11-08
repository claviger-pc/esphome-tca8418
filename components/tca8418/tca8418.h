// components/tca8418/tca8418.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace tca8418 {

class TCA8418BinarySensor;

class TCA8418Component : public Component, public i2c::I2CDevice {
 public:
  void set_rows(uint8_t rows) { rows_ = rows; }
  void set_cols(uint8_t cols) { cols_ = cols; }
  void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }

  void register_key_sensor(uint8_t index, TCA8418BinarySensor *sensor);

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  static const uint8_t MAX_KEYS = 64;

  TCA8418BinarySensor *key_sensors_[MAX_KEYS] = {nullptr};

  uint8_t rows_{0};
  uint8_t cols_{0};
  InternalGPIOPin *interrupt_pin_{nullptr};
  uint32_t last_poll_{0};

  bool read_key_event_(uint8_t &key_event);
  void process_key_event_(uint8_t key_event);
};

class TCA8418BinarySensor : public binary_sensor::BinarySensor,
                            public Parented<TCA8418Component> {
 public:
  void set_key_index(uint8_t key) { key_index_ = key; }
  uint8_t get_key_index() const { return key_index_; }

 protected:
  friend class TCA8418Component;
  uint8_t key_index_{0};
};

}  // namespace tca8418
}  // namespace esphome
