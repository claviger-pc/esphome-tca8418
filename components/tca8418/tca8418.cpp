#include "tca8418.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tca8418 {

static const char *const TAG = "tca8418";

// ------- TCA8418Component implementation -------

void TCA8418Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418...");

  // Basic config:
  //  - disable keypad scan engine (CFG bit[0] = KE=0) so we may use pure GPIO
  //  - leave de-bounce/etc off for now
  uint8_t cfg = 0x00;
  this->write_register(REG_CFG, &cfg, 1);

  // Set all pins as inputs by default.
  uint8_t dir0 = 0x00;  // 0=input, 1=output
  uint8_t dir1 = 0x00;
  uint8_t dir2 = 0x00;
  this->write_register(REG_GPIO_DIR0, &dir0, 1);
  this->write_register(REG_GPIO_DIR1, &dir1, 1);
  this->write_register(REG_GPIO_DIR2, &dir2, 1);

  // Enable pull-ups on all pins by default (you can override in pin_mode)
  uint8_t pull1 = 0xFF;
  uint8_t pull2 = 0xFF;
  uint8_t pull3 = 0x03;  // only P16, P17 used in this reg
  this->write_register(REG_KP_GPIO_PULL1, &pull1, 1);
  this->write_register(REG_KP_GPIO_PULL2, &pull2, 1);
  this->write_register(REG_KP_GPIO_PULL3, &pull3, 1);
}

void TCA8418Component::dump_config() {
  ESP_LOGCONFIG(TAG, "TCA8418:");
  LOG_I2C_DEVICE(this);
}

void TCA8418Component::update_bitfield(uint8_t base_reg, uint8_t pin, bool value) {
  // base_reg is REG_GPIO_DIR0/1/2 or REG_GPIO_DAT_OUT0/1/2 etc.
  uint8_t reg = base_reg;
  uint8_t index = pin;
  if (index >= 8 && index < 16) {
    reg = base_reg + 1;
    index -= 8;
  } else if (index >= 16) {
    reg = base_reg + 2;
    index -= 16;
  }

  uint8_t buf;
  if (!this->read_register(reg, &buf, 1)) {
    ESP_LOGW(TAG, "Failed to read reg 0x%02X", reg);
    return;
  }
  if (value)
    buf |= (1 << index);
  else
    buf &= ~(1 << index);

  this->write_register(reg, &buf, 1);
}

bool TCA8418Component::read_bitfield(uint8_t base_reg, uint8_t pin) {
  uint8_t reg = base_reg;
  uint8_t index = pin;
  if (index >= 8 && index < 16) {
    reg = base_reg + 1;
    index -= 8;
  } else if (index >= 16) {
    reg = base_reg + 2;
    index -= 16;
  }

  uint8_t buf;
  if (!this->read_register(reg, &buf, 1)) {
    ESP_LOGW(TAG, "Failed to read reg 0x%02X", reg);
    return false;
  }
  return (buf & (1 << index)) != 0;
}

void TCA8418Component::set_pin_direction(uint8_t pin, bool output) {
  this->update_bitfield(REG_GPIO_DIR0, pin, output);
}

void TCA8418Component::set_pin_pullup(uint8_t pin, bool enabled) {
  this->update_bitfield(REG_KP_GPIO_PULL1, pin, enabled);
}

bool TCA8418Component::read_pin(uint8_t pin) {
  return this->read_bitfield(REG_GPIO_DAT_STAT0, pin);
}

void TCA8418Component::write_pin(uint8_t pin, bool value) {
  this->update_bitfield(REG_GPIO_DAT_OUT0, pin, value);
}

// ------- TCA8418GPIOPin implementation -------

void TCA8418GPIOPin::setup() {
  // nothing special here; pin_mode() will be called by ESPHome
}

void TCA8418GPIOPin::pin_mode(gpio::Flags flags) {
  const bool is_output = flags.output;
  const bool pullup = flags.pullup;

  this->parent_->set_pin_direction(this->pin_, is_output);
  this->parent_->set_pin_pullup(this->pin_, pullup);
}

bool TCA8418GPIOPin::digital_read() {
  return this->parent_->read_pin(this->pin_);
}

void TCA8418GPIOPin::digital_write(bool value) {
  this->parent_->write_pin(this->pin_, value);
}

std::string TCA8418GPIOPin::dump_summary() const {
  char buf[32];
  snprintf(buf, sizeof(buf), "TCA8418 Pin %u", this->pin_);
  return std::string(buf);
}

}  // namespace tca8418
}  // namespace esphome
