#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace tca8418 {

/// 18 pins total: ROW0-7 (0..7), COL0-9 (8..17)
static const uint8_t TCA8418_NUM_PINS = 18;

// Register addresses (from TI TCA8418 datasheet)
static const uint8_t REG_CFG          = 0x01;
static const uint8_t REG_GPI_EM       = 0x02;
static const uint8_t REG_GPI_INT_EN   = 0x03;
static const uint8_t REG_GPIO_DIR0    = 0x0E;  // direction bits for P0..P7
static const uint8_t REG_GPIO_DIR1    = 0x0F;  // direction bits for P8..P15
static const uint8_t REG_GPIO_DIR2    = 0x10;  // direction bits for P16..P17 (only 2 bits used)
static const uint8_t REG_GPIO_DAT_STAT0 = 0x11;
static const uint8_t REG_GPIO_DAT_STAT1 = 0x12;
static const uint8_t REG_GPIO_DAT_STAT2 = 0x13;
static const uint8_t REG_GPIO_DAT_OUT0  = 0x14;
static const uint8_t REG_GPIO_DAT_OUT1  = 0x15;
static const uint8_t REG_GPIO_DAT_OUT2  = 0x16;
static const uint8_t REG_KP_GPIO_PULL1  = 0x0B;
static const uint8_t REG_KP_GPIO_PULL2  = 0x0C;
static const uint8_t REG_KP_GPIO_PULL3  = 0x0D;

class TCA8418Component;

/// A single pin on the TCA8418, implementing the GPIOPin interface so that
/// other ESPHome components (matrix_keypad, gpio sensors, etc.) can use it.
class TCA8418GPIOPin : public GPIOPin {
 public:
  TCA8418GPIOPin(TCA8418Component *parent, uint8_t pin) : parent_(parent), pin_(pin) {}

  void setup() override;
  void pin_mode(gpio::Flags flags) override;
  bool digital_read() override;
  void digital_write(bool value) override;
  std::string dump_summary() const override;

 protected:
  TCA8418Component *parent_;
  uint8_t pin_;  // 0..17
};

/// The I2C hub for the TCA8418.
class TCA8418Component : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;

  friend class TCA8418GPIOPin;

  // Low-level helpers used by TCA8418GPIOPin
  void set_pin_direction(uint8_t pin, bool output);
  void set_pin_pullup(uint8_t pin, bool enabled);
  bool read_pin(uint8_t pin);
  void write_pin(uint8_t pin, bool value);

 protected:
  // Convenience to read/modify/write a single bit in a 3-byte bitmap (0..17)
  void update_bitfield(uint8_t base_reg, uint8_t pin, bool value);
  bool read_bitfield(uint8_t base_reg, uint8_t pin);
};

}  // namespace tca8418
}  // namespace esphome
