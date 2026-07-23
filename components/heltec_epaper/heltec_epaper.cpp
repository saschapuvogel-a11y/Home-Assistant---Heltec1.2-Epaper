#include "heltec_epaper.h"

#include <cstring>

#include "esphome/core/application.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heltec_epaper {

static const char *const TAG = "heltec_epaper";

void HeltecE0213A367::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heltec Wireless Paper V1.2 E0213A367...");

  this->dc_pin_->setup();
  this->dc_pin_->digital_write(false);
  this->reset_pin_->setup();
  this->reset_pin_->digital_write(false);
  this->busy_pin_->setup();
  this->power_pin_->setup();

  // GPIO45 supplies the board peripherals and is active LOW.
  // Start with VEXT disabled, then let wake_up() perform a clean sequence.
  this->power_pin_->digital_write(true);
  this->powered_ = false;
  this->sleeping_ = true;

  this->spi_setup();

  this->init_internal_(BUFFER_SIZE);
  if (this->buffer_ == nullptr) {
    ESP_LOGE(TAG, "Could not allocate %u-byte display buffer", static_cast<unsigned>(BUFFER_SIZE));
    this->mark_failed();
    return;
  }
  std::memset(this->buffer_, 0xFF, BUFFER_SIZE);

  if (!this->wake_up()) {
    ESP_LOGE(TAG, "Display initialization failed");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "E0213A367 initialized");
}

void HeltecE0213A367::power_on_() {
  if (this->powered_)
    return;

  this->power_pin_->digital_write(false);
  this->powered_ = true;
  delay(this->power_on_delay_ms_);
}

void HeltecE0213A367::power_off_() {
  // Keep interface pins at defined levels before removing VEXT.
  this->reset_pin_->digital_write(false);
  this->dc_pin_->digital_write(false);
  this->power_pin_->digital_write(true);

  this->powered_ = false;
  this->initialized_ = false;
  this->sleeping_ = true;
  this->refresh_mode_ = RefreshMode::UNKNOWN;
}

void HeltecE0213A367::hard_reset_() {
  this->reset_pin_->digital_write(false);
  delay(10);
  this->reset_pin_->digital_write(true);
  delay(10);
}

bool HeltecE0213A367::wait_until_idle_(const char *operation) {
  const uint32_t started = millis();
  while (this->busy_pin_->digital_read()) {  // BUSY is active HIGH.
    if (millis() - started > this->busy_timeout_ms_) {
      ESP_LOGE(TAG, "Timeout waiting for BUSY during %s", operation);
      return false;
    }
    App.feed_wdt();
    delay(1);
  }
  return true;
}

void HeltecE0213A367::command_(uint8_t command) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(command);
  this->disable();
}

void HeltecE0213A367::data_(uint8_t data) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_byte(data);
  this->disable();
}

void HeltecE0213A367::data_array_(const uint8_t *data, size_t length) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_array(data, length);
  this->disable();
}

bool HeltecE0213A367::initialize_panel_() {
  this->hard_reset_();
  if (!this->wait_until_idle_("hardware reset"))
    return false;

  this->command_(0x12);  // Software reset.
  if (!this->wait_until_idle_("software reset"))
    return false;

  this->refresh_mode_ = RefreshMode::UNKNOWN;
  return this->select_refresh_mode_(RefreshMode::FULL);
}

bool HeltecE0213A367::wake_up() {
  if (this->is_failed())
    return false;

  if (this->powered_ && this->initialized_ && !this->sleeping_)
    return true;

  ESP_LOGI(TAG, "Waking E0213A367 and enabling VEXT");
  this->power_on_();

  if (!this->initialize_panel_()) {
    ESP_LOGE(TAG, "E0213A367 wake-up failed");
    this->power_off_();
    return false;
  }

  this->initialized_ = true;
  this->sleeping_ = false;
  ESP_LOGI(TAG, "E0213A367 awake");
  return true;
}

bool HeltecE0213A367::sleep() {
  if (!this->powered_) {
    this->sleeping_ = true;
    return true;
  }

  ESP_LOGI(TAG, "Putting E0213A367 into deep sleep and disabling VEXT");

  if (!this->wait_until_idle_("pre-sleep")) {
    this->status_set_warning("Display pre-sleep BUSY timeout");
    return false;
  }

  // Controller deep-sleep command. A hardware reset is required after wake-up.
  this->command_(0x10);
  this->data_(0x01);
  delay(100);

  this->power_off_();
  this->status_clear_warning();
  ESP_LOGI(TAG, "E0213A367 asleep; VEXT disabled");
  return true;
}

bool HeltecE0213A367::select_refresh_mode_(RefreshMode mode) {
  if (this->refresh_mode_ == mode)
    return true;

  // The original controller sequence resets when changing waveform mode.
  this->hard_reset_();
  if (!this->wait_until_idle_("refresh-mode reset"))
    return false;

  if (mode == RefreshMode::FULL)
    this->configure_full_refresh_();
  else
    this->configure_partial_refresh_();

  if (!this->wait_until_idle_(mode == RefreshMode::FULL ? "full-refresh configuration"
                                                          : "partial-refresh configuration"))
    return false;

  this->refresh_mode_ = mode;
  return true;
}

void HeltecE0213A367::configure_full_refresh_() {
  this->command_(0x37);
  this->data_(0x00);
  this->data_(0x80);
  this->data_(0x03);
  this->data_(0x0E);

  this->command_(0x3C);
  this->data_(0x01);
}

void HeltecE0213A367::configure_partial_refresh_() {
  this->command_(0x37);
  this->data_(0x00);
  this->data_(0x80);
  this->data_(0x03);
  this->data_(0x0E);

  this->command_(0x3C);
  this->data_(0x80);
}

void HeltecE0213A367::set_full_memory_area_() {
  this->command_(0x11);
  this->data_(0x03);  // X increment, Y increment.

  this->command_(0x44);
  this->data_(0x00);
  this->data_((PANEL_WIDTH / 8) - 1);

  // E0213A367 uses one-byte Y coordinates for its 250-row RAM.
  this->command_(0x45);
  this->data_(0x00);
  this->data_(PANEL_HEIGHT - 1);

  this->command_(0x4E);
  this->data_(0x00);

  this->command_(0x4F);
  this->data_(0x00);
}

bool HeltecE0213A367::write_full_frame_() {
  this->set_full_memory_area_();

  this->command_(0x24);
  this->data_array_(this->buffer_, BUFFER_SIZE);

  // Seed the differential-update memory with the same image.
  this->command_(0x26);
  this->data_array_(this->buffer_, BUFFER_SIZE);
  return true;
}

bool HeltecE0213A367::write_partial_frame_() {
  this->set_full_memory_area_();

  // 0x26 still contains the previous frame. Put the new frame in 0x24.
  this->command_(0x24);
  this->data_array_(this->buffer_, BUFFER_SIZE);
  return true;
}

bool HeltecE0213A367::activate_(bool partial) {
  this->command_(0x22);
  this->data_(partial ? 0xFF : 0xF7);
  this->command_(0x20);
  return this->wait_until_idle_(partial ? "partial display refresh" : "full display refresh");
}

bool HeltecE0213A367::end_image_tx_quiet_() {
  this->command_(0x7F);
  return this->wait_until_idle_("partial reference-frame update");
}

void HeltecE0213A367::update() {
  if (this->is_failed())
    return;

  // A periodic or manual update may follow an explicit display sleep.
  if (!this->wake_up()) {
    this->status_set_warning("Display wake-up failed");
    return;
  }

  const uint32_t started = millis();
  this->do_update_();

  const bool full = this->full_update_every_ <= 1 ||
                    (this->update_count_ % this->full_update_every_) == 0;
  const RefreshMode wanted_mode = full ? RefreshMode::FULL : RefreshMode::PARTIAL;

  if (!this->select_refresh_mode_(wanted_mode)) {
    this->status_set_warning("Refresh-mode setup failed");
    return;
  }

  ESP_LOGI(TAG, "Performing %s E0213A367 update (%u)", full ? "full" : "partial",
           static_cast<unsigned>(this->update_count_ + 1));

  bool ok = false;
  if (full) {
    ok = this->write_full_frame_() && this->activate_(false);
  } else {
    ok = this->write_partial_frame_() && this->activate_(true);

    // After the differential refresh, copy the new image into old-image RAM
    // without triggering another visible update.
    if (ok) {
      this->set_full_memory_area_();
      this->command_(0x26);
      this->data_array_(this->buffer_, BUFFER_SIZE);
      ok = this->end_image_tx_quiet_();
    }
  }

  if (!ok) {
    this->status_set_warning("Display update failed");
    return;
  }

  this->update_count_++;
  this->status_clear_warning();
  ESP_LOGI(TAG, "Completed %s E0213A367 update in %u ms", full ? "full" : "partial",
           static_cast<unsigned>(millis() - started));
}

void HeltecE0213A367::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= VISIBLE_WIDTH || y < 0 || y >= PANEL_HEIGHT)
    return;

  const size_t byte_index = static_cast<size_t>(y) * (PANEL_WIDTH / 8) + (x / 8);
  const uint8_t mask = 0x80 >> (x & 0x07);

  // Controller RAM: 1 = white, 0 = black.
  if (color.is_on())
    this->buffer_[byte_index] &= ~mask;
  else
    this->buffer_[byte_index] |= mask;
}

void HeltecE0213A367::dump_config() {
  ESP_LOGCONFIG(TAG, "Heltec Wireless Paper E-Paper:");
  ESP_LOGCONFIG(TAG, "  Driver version: 0.3.0-rc1");
  ESP_LOGCONFIG(TAG, "  Model: E0213A367 (Wireless Paper V1.2)");
  ESP_LOGCONFIG(TAG, "  Visible dimensions: %d x %d", VISIBLE_WIDTH, PANEL_HEIGHT);
  ESP_LOGCONFIG(TAG, "  Controller RAM: %d x %d", PANEL_WIDTH, PANEL_HEIGHT);
  ESP_LOGCONFIG(TAG, "  Full update every: %u refresh(es)",
                static_cast<unsigned>(this->full_update_every_));
  ESP_LOGCONFIG(TAG, "  VEXT state: %s", this->powered_ ? "ON" : "OFF");
  ESP_LOGCONFIG(TAG, "  Panel state: %s", this->sleeping_ ? "SLEEP" : "AWAKE");
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
  LOG_PIN("  VEXT Power Pin: ", this->power_pin_);
  LOG_UPDATE_INTERVAL(this);
  LOG_SPI_DEVICE(this);
}

}  // namespace heltec_epaper
}  // namespace esphome
