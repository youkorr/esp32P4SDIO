#include "esp32p4_sdio.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include <sys/stat.h>
#include <sys/unistd.h>
#include <dirent.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

namespace esphome {
namespace esp32p4_sdio {

static const char *const TAG = "esp32p4_sdio";

void ESP32P4SDIOComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ESP32-P4 SDIO...");
  
  // Configuration du host SDMMC
  this->host_ = SDMMC_HOST_DEFAULT();
  this->host_.max_freq_khz = this->frequency_ / 1000;
  
  // Configuration des slots
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  
  // Configuration des pins
  slot_config.gpio_cd = GPIO_NUM_NC;     // Card detect non utilisé
  slot_config.gpio_wp = GPIO_NUM_NC;     // Write protect non utilisé
  slot_config.width = 4;                 // Mode 4-bit
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  
  // Assignation des pins spécifiques ESP32-P4
  slot_config.clk = (gpio_num_t)this->clk_pin_->get_pin();
  slot_config.cmd = (gpio_num_t)this->cmd_pin_->get_pin();
  slot_config.d0 = (gpio_num_t)this->dat0_pin_->get_pin();
  slot_config.d1 = (gpio_num_t)this->dat1_pin_->get_pin();
  slot_config.d2 = (gpio_num_t)this->dat2_pin_->get_pin();
  slot_config.d3 = (gpio_num_t)this->dat3_pin_->get_pin();

  // Initialisation du host
  esp_err_t ret = sdmmc_host_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize SDMMC host: %s", esp_err_to_name(ret));
    this->mark_failed();
    return;
  }

  ret = sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize SDMMC slot: %s", esp_err_to_name(ret));
    this->mark_failed();
    return;
  }

  // Configuration du filesystem FAT
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };

  // Montage de la carte SD
  ret = esp_vfs_fat_sdmmc_mount(this->mount_point_.c_str(), &this->host_, 
                                &slot_config, &mount_config, &this->card_);
  
  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem. If you want to format the card, set format_if_mount_failed = true.");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
    }
    this->mark_failed();
    return;
  }

  this->mounted_ = true;
  ESP_LOGI(TAG, "SD card mounted successfully at %s", this->mount_point_.c_str());
  
  // Affichage des informations de la carte
  ESP_LOGI(TAG, "SD card info:");
  ESP_LOGI(TAG, "  Name: %s", this->card_->cid.name);
  ESP_LOGI(TAG, "  Type: %s", (this->card_->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC");
  ESP_LOGI(TAG, "  Speed: %s", (this->card_->csd.tr_speed > 25000000) ? "high speed" : "default speed");
  ESP_LOGI(TAG, "  Size: %lluMB", ((uint64_t) this->card_->csd.capacity) * this->card_->csd.sector_size / (1024 * 1024));
}

void ESP32P4SDIOComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ESP32-P4 SDIO:");
  ESP_LOGCONFIG(TAG, "  CLK Pin: %d", this->clk_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  CMD Pin: %d", this->cmd_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  DAT0 Pin: %d", this->dat0_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  DAT1 Pin: %d", this->dat1_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  DAT2 Pin: %d", this->dat2_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  DAT3 Pin: %d", this->dat3_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  Frequency: %.1f MHz", this->frequency_ / 1000000.0f);
  ESP_LOGCONFIG(TAG, "  Mount Point: %s", this->mount_point_.c_str());
  ESP_LOGCONFIG(TAG, "  Status: %s", this->mounted_ ? "Mounted" : "Failed");
}

void ESP32P4SDIOComponent::set_pins(InternalGPIOPin *dat0, InternalGPIOPin *dat1, 
                                   InternalGPIOPin *dat2, InternalGPIOPin *dat3,
                                   InternalGPIOPin *clk, InternalGPIOPin *cmd) {
  this->dat0_pin_ = dat0;
  this->dat1_pin_ = dat1;
  this->dat2_pin_ = dat2;
  this->dat3_pin_ = dat3;
  this->clk_pin_ = clk;
  this->cmd_pin_ = cmd;
}

size_t ESP32P4SDIOComponent::get_free_space() {
  if (!this->mounted_) {
    return 0;
  }

  FATFS *fs;
  DWORD fre_clust;
  DWORD fre_sect;
  DWORD tot_sect;

  // Obtenir les informations du système de fichiers
  esp_err_t ret = f_getfree("0:", &fre_clust, &fs);
  if (ret != FR_OK) {
    ESP_LOGE(TAG, "Failed to get filesystem info");
    return 0;
  }

  // Calculer l'espace libre
  tot_sect = (fs->n_fatent - 2) * fs->csize;
  fre_sect = fre_clust * fs->csize;

  // Convertir en MB
  return (fre_sect * fs->ssize) / (1024 * 1024);
}

bool ESP32P4SDIOComponent::read_file(const std::string &path, std::string &content) {
  if (!this->mounted_) {
    ESP_LOGE(TAG, "SD card not mounted");
    return false;
  }

  std::string full_path = this->mount_point_ + path;
  
  FILE *f = fopen(full_path.c_str(), "r");
  if (f == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for reading: %s", full_path.c_str());
    return false;
  }

  // Obtenir la taille du fichier
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  // Lire le contenu
  content.resize(file_size);
  size_t read_bytes = fread(&content[0], 1, file_size, f);
  fclose(f);

  if (read_bytes != file_size) {
    ESP_LOGE(TAG, "Failed to read complete file: %s", full_path.c_str());
    return false;
  }

  ESP_LOGD(TAG, "Read %zu bytes from file: %s", read_bytes, full_path.c_str());
  return true;
}

bool ESP32P4SDIOComponent::write_file(const std::string &path, const std::string &content) {
  if (!this->mounted_) {
    ESP_LOGE(TAG, "SD card not mounted");
    return false;
  }

  std::string full_path = this->mount_point_ + path;
  
  FILE *f = fopen(full_path.c_str(), "w");
  if (f == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for writing: %s", full_path.c_str());
    return false;
  }

  size_t written_bytes = fwrite(content.c_str(), 1, content.length(), f);
  fclose(f);

  if (written_bytes != content.length()) {
    ESP_LOGE(TAG, "Failed to write complete file: %s", full_path.c_str());
    return false;
  }

  ESP_LOGD(TAG, "Wrote %zu bytes to file: %s", written_bytes, full_path.c_str());
  return true;
}

std::string ESP32P4SDIOComponent::get_card_info() {
  if (!this->mounted_ || this->card_ == nullptr) {
    return "SD card not available";
  }

  char info[300];
  snprintf(info, sizeof(info), 
    "Name: %s, Type: %s, Speed: %s, Size: %lluMB, Free: %zuMB",
    this->card_->cid.name,
    (this->card_->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC",
    (this->card_->csd.tr_speed > 25000000) ? "High Speed" : "Default Speed",
    ((uint64_t) this->card_->csd.capacity) * this->card_->csd.sector_size / (1024 * 1024),
    this->get_free_space()
  );

  return std::string(info);
}

}  // namespace esp32p4_sdio
}  // namespace esphome
