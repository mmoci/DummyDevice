#pragma once

// SD card pin configuration based on the target ESP32 variant
#if defined(ESP32) || defined(ESP_PLATFORM)
  #if CONFIG_IDF_TARGET_ESP32C3
    static constexpr int SD_SCK = 4, SD_MISO = 5, SD_MOSI = 6, SD_CS = 7;
    static constexpr int BOOT_PIN = 9;
  #else
    static constexpr int SD_SCK = 18, SD_MISO = 19, SD_MOSI = 23, SD_CS = 5;
    static constexpr int BOOT_PIN = 0;
  #endif
#endif