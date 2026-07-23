#pragma once

#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace heltec_epaper {

class HeltecE0213A367
    : public display::DisplayBuffer,
      public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST,
                            spi::CLOCK_POLARITY_LOW,
                            spi::CLOCK_PHASE_LEADING,
                            spi::DATA_RATE_2MHZ> {
 public:
  void set_dc_pin(GPIOPin *pin) { this->dc_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }
  void set_busy_pin(GPIOPin *pin) { this->busy_pin_ = pin; }
  void set_power_pin(GPIOPin *pin) { this->power_pin_ = pin; }
  void set_power_on_delay(uint32_t delay_ms) { this->power_on_delay_ms_ = delay_ms; }
  void set_busy_timeout(uint32_t timeout_ms) { this->busy_timeout_ms_ = timeout_ms; }
  void set_full_update_every(uint32_t value) { this->full_update_every_ = value; }

  void setup() override;
  void update() override;
  void dump_config() override;

  // Public power-management methods for ESPHome lambdas.
  // The panel retains the displayed image without power.
  bool sleep();
  bool wake_up();
  bool is_sleeping() const { return this->sleeping_; }

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

 protected:
  static constexpr int PANEL_WIDTH = 128;
  static constexpr int VISIBLE_WIDTH = 122;
  static constexpr int PANEL_HEIGHT = 250;
  static constexpr size_t BUFFER_SIZE = PANEL_WIDTH * PANEL_HEIGHT / 8;

  enum class RefreshMode : uint8_t {
    UNKNOWN,
    FULL,
    PARTIAL,
  };

  int get_width_internal() override { return VISIBLE_WIDTH; }
  int get_height_internal() override { return PANEL_HEIGHT; }
  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_BINARY;
  }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void power_on_();
  void power_off_();
  void hard_reset_();
  bool wait_until_idle_(const char *operation);

  void command_(uint8_t command);
  void data_(uint8_t data);
  void data_array_(const uint8_t *data, size_t length);

  bool initialize_panel_();
  bool select_refresh_mode_(RefreshMode mode);
  void configure_full_refresh_();
  void configure_partial_refresh_();
  void set_full_memory_area_();

  bool write_full_frame_();
  bool write_partial_frame_();
  bool activate_(bool partial);
  bool end_image_tx_quiet_();

  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *busy_pin_{nullptr};
  GPIOPin *power_pin_{nullptr};

  uint32_t power_on_delay_ms_{50};
  uint32_t busy_timeout_ms_{20000};
  uint32_t full_update_every_{10};
  uint32_t update_count_{0};

  RefreshMode refresh_mode_{RefreshMode::UNKNOWN};
  bool initialized_{false};
  bool powered_{false};
  bool sleeping_{true};
};

}  // namespace heltec_epaper
}  // namespace esphome
