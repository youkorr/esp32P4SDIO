#include "sensor.h"
#include "esp32p4_sdio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esp32p4_sdio {

static const char *const TAG = "esp32p4_sdio.sensor";

void ESP32P4SDIOSensor::update() {
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "Parent component not set");
    return;
  }

  size_t free_space = this->parent_->get_free_space();
  this->publish_state((float)free_space);
}

}  // namespace esp32p4_sdio
}  // namespace esphome
