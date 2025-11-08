// components/tca8418/tca8418.cpp
#include "tca8418.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace tca8418 {

static const char *const TAG = "tca8418";

// TCA8418 registers (from TI datasheet)
static const uint8_t REG_CFG         = 0x01;
static const uint8_t REG_INT_STAT    = 0x02;
static const uint8_t REG_KEY_LCK_EC  = 0x03;
static const uint8_t REG_KEY_EVENT_A = 0x04;
static const uint8_t REG_KP_GPIO1    = 0x1D;
static const uint8_t REG_KP_GPIO2    = 0x1E;
static const uint8_t REG_KP_GPIO3    = 0x1F;

void TCA8418Component::register_key_sensor(uint8_t index, binary_sensor::BinarySensor *sensor) {
  if (index >= MAX_KEYS) {
    ESP_LOGW(TAG, "Key index %u out of range (max %u)", index, MAX_KEYS - 1);
    return;
  }
  key_sensors_[index] = sensor;
}

void TCA8418Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418 keypad driver...");

  // Basic probe: read CFG register
  uint8_t cfg = 0;
  auto err = this->read_byte(REG_CFG, &cfg);
  if (err != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to communicate with TCA8418 (read CFG), I2C error=%d", err);
    this->mark_failed();
    return;
  }

  ESP_LOGD(TAG, "TCA8418 CFG initial value: 0x%02X", cfg);

  // Configure keypad matrix pins.
  // KP_GPIO1: rows R0..R7
  // KP_GPIO2: columns C0..C7
  // KP_GPIO3: columns C8..C9

  // For OMOTE we assume rows=6 (R0..R5), cols=4 (C0..C3) hard-coded.
  // You can make rows_/cols_ configurable later if desired.
  if (rows_ > 8)
    rows_ = 8;
  if (cols_ > 10)
    cols_ = 10;

  uint8_t row_mask = (rows_ > 0) ? static_cast<uint8_t>((1u << rows_) - 1u) : 0;

  uint8_t col_mask_low = 0;
  uint8_t col_mask_high = 0;
  if (cols_ > 0) {
    if (cols_ <= 8) {
      col_mask_low = static_cast<uint8_t>((1u << cols_) - 1u);
    } else {
      col_mask_low = 0xFF;  // C0..C7
      uint8_t extra = cols_ - 8;  // 1 or 2
      col_mask_high = static_cast<uint8_t>((1u << extra) - 1u);
    }
  }

  ESP_LOGD(TAG, "Config rows=%u (mask=0x%02X) cols=%u (mask_low=0x%02X, mask_high=0x%02X)",
           rows_, row_mask, cols_, col_mask_low, col_mask_high);

  // Write keypad configuration
  this->write_byte(REG_KP_GPIO1, row_mask);
  this->write_byte(REG_KP_GPIO2, col_mask_low);
  this->write_byte(REG_KP_GPIO3, col_mask_high);

  // Enable key event interrupt & typical settings.
  // Bit0 KE_IEN=1, Bit4 INT_CFG=1  => 0x11 is a common "example" config.
  this->write_byte(REG_CFG, 0x11);

  // Clear any pending interrupts
  this->write_byte(REG_INT_STAT, 0xFF);

  // Optional: configure interrupt pin as input with pull-up
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  }

  ESP_LOGCONFIG(TAG, "TCA8418 setup done");
}

void TCA8418Component::dump_config() {
  ESP_LOGCONFIG(TAG, "TCA8418 Keypad:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Rows: %u", rows_);
  ESP_LOGCONFIG(TAG, "  Cols: %u", cols_);
  if (this->interrupt_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Interrupt pin configured");
  } else {
    ESP_LOGCONFIG(TAG, "  No interrupt pin (polling mode)");
  }
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Status: FAILED");
  }
}

bool TCA8418Component::read_key_event_(uint8_t &key_event) {
  // Check how many events are in the FIFO
  uint8_t count = 0;
  auto err = this->read_byte(REG_KEY_LCK_EC, &count);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C error reading KEY_LCK_EC: %d", err);
    this->status_set_warning();
    return false;
  }

  if (count == 0) {
    return false;  // no events
  }

  err = this->read_byte(REG_KEY_EVENT_A, &key_event);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C error reading KEY_EVENT_A: %d", err);
    this->status_set_warning();
    return false;
  }

  // Clear interrupt status bits by reading & writing back
  uint8_t int_stat = 0;
  if (this->read_byte(REG_INT_STAT, &int_stat) == i2c::ERROR_OK) {
    this->write_byte(REG_INT_STAT, int_stat);
  }

  this->status_clear_warning();
  return true;
}

void TCA8418Component::process_key_event_(uint8_t ev) {
  // MSB = 1 -> key press, 0 -> release
  bool pressed = (ev & 0x80) != 0;
  uint8_t key_code = (ev & 0x7F);

  if (key_code == 0 || key_code > 80) {
    ESP_LOGV(TAG, "Ignoring out-of-range key code: 0x%02X", ev);
    return;
  }

  // Datasheet convention:
  // key_code ranges 1..80
  // zero-based: 0..79
  // row = value / 10, col = value % 10
  uint8_t zero_based = key_code - 1;
  uint8_t row = zero_based / 10;
  uint8_t col = zero_based % 10;

  ESP_LOGV(TAG, "Raw key event=0x%02X -> row=%u col=%u pressed=%d",
           ev, row, col, pressed);

  if (row >= rows_ || col >= cols_) {
    // Outside configured matrix range for this board.
    return;
  }

  uint8_t index = static_cast<uint8_t>(row * cols_ + col);
  if (index >= MAX_KEYS) {
    return;
  }

  auto *sensor = key_sensors_[index];
  if (sensor != nullptr) {
    sensor->publish_state(pressed);
  }
}

void TCA8418Component::loop() {
  const uint32_t now = millis();

  // light polling rate limit, ~100 Hz max
  if (now - this->last_poll_ < 10)
    return;
  this->last_poll_ = now;

  // For simplicity we always poll the FIFO; you could also gate on interrupt pin.
  uint8_t ev;
  uint8_t safety = 16;  // don't sit here forever if something is wrong
  while (safety-- > 0 && this->read_key_event_(ev)) {
    this->process_key_event_(ev);
  }
}

}  // namespace tca8418
}  // namespace esphome
