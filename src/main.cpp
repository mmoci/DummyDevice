#include <Arduino.h>
#include "Logger.h"
#include "SdCardLogger.h"
#include "DummyDevice.h"
#include "Secrets.h"

static const char* TAG = "MAIN";

// Initialize the SD card logger with the specified configuration
static SdCardLogger sdCardLogger{SdCardLogger::Config{.csPin = 5, .logFileName = "/log"}};
auto sdCardFlushCb = [](){ sdCardLogger.flush(); };
DummyDevice::Config dummyDeviceConfig
{
  .ssid=WIFI_SSID, 
  .password=WIFI_PASSWORD, 
  .primaryDns=IPAddress(1, 1, 1, 1), 
  .secondaryDns=IPAddress(8, 8, 8, 8)
};
static DummyDevice dummyDevice{dummyDeviceConfig};

void setup() 
{
  Serial.begin(115200);

  sdCardLogger.init();

  #if defined(ESP32) || defined(ESP_PLATFORM)
    esp_log_level_set("*", ESP_LOG_DEBUG);
  #endif

  auto resetReason = esp_reset_reason();
  LOG_INFO(TAG, "Reset reason: %d", resetReason);

  dummyDevice.init(sdCardFlushCb);
}

void loop() 
{

}