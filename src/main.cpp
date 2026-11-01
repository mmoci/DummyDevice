#include <Arduino.h>
#include "Logger.h"
#include "SdCardLogger.h"
#include "DummyDevice.h"
#include "Secrets.h"
#include "Config.h"

static const char* TAG = "MAIN";

// Initialize the SD card logger with the specified configuration
static SdCardLogger sdCardLogger{SdCardLogger::Config{.csPin = SD_CS, .logFileName = "/log"}};
auto sdCardFlushCb = [](){ sdCardLogger.flush(); };

// Initialize the configuration for the dummy device
DummyDevice::Config dummyDeviceConfig
{
  .ssid=WIFI_SSID_1, 
  .password=WIFI_PASSWORD_1, 
  .primaryDns=IPAddress(1, 1, 1, 1), 
  .secondaryDns=IPAddress(8, 8, 8, 8)
};
static DummyDevice dummyDevice{dummyDeviceConfig};

void setup() 
{
  pinMode(BOOT_PIN, INPUT_PULLUP);
  const bool stayAwake = (digitalRead(BOOT_PIN) == LOW);
  
  Serial.begin(115200);

  if (esp_reset_reason() != ESP_RST_DEEPSLEEP)
    delay(3000);

  sdCardLogger.init();

  #if defined(ESP32) || defined(ESP_PLATFORM)
    esp_log_level_set("*", ESP_LOG_DEBUG);
  #endif

  auto resetReason = esp_reset_reason();
  LOG_INFO(TAG, "Reset reason: %d", resetReason);

  dummyDevice.init(sdCardFlushCb, stayAwake);
}

void loop() 
{}