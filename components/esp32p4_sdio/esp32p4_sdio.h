#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

namespace esphome {
namespace esp32p4_sdio {

class ESP32P4SDIOComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::BUS; }
  
  void set_pins(InternalGPIOPin *dat0, InternalGPIOPin *dat1, 
                InternalGPIOPin *dat2, InternalGPIOPin *dat3,
                InternalGPIOPin *clk, InternalGPIOPin *cmd);
  void set_frequency(uint32_t frequency) { this->frequency_ = frequency; }
  void set_mount_point(const std::string &mount_point) { this->mount_point_ = mount_point; }
  void set_slot(uint8_t slot) { this->slot_ = slot; }
  
  bool is_mounted() const { return this->mounted_; }
  size_t get_free_space();
  bool read_file(const std::string &path, std::string &content);
  bool write_file(const std::string &path, const std::string &content);
  std::string get_card_info();

 protected:
  InternalGPIOPin *dat0_pin_{nullptr};
  InternalGPIOPin *dat1_pin_{nullptr};
  InternalGPIOPin *dat2_pin_{nullptr};
  InternalGPIOPin *dat3_pin_{nullptr};
  InternalGPIOPin *clk_pin_{nullptr};
  InternalGPIOPin *cmd_pin_{nullptr};
  
  uint32_t frequency_{20000000};
  std::string mount_point_{"/sdcard"};
  uint8_t slot_{1};
  bool mounted_{false};
  
  sdmmc_card_t *card_{nullptr};
  sdmmc_host_t host_;
};

}  // namespace esp32p4_sdio
}  // namespace esphome


