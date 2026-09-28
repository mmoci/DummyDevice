#include "DummyDevice.h"
#include <HTTPClient.h>

static const char* TAG = "DummyDevice";

DummyDevice::DummyDevice(const Config& config) : m_config(config) 
{}

void DummyDevice::init(const SdCardFlushCb& sdCardFlushCb)
{
    if(connectToWiFi())
    {
        ESP_LOGI(TAG, "Successfully connected to WiFi: %s", m_config.ssid.data());
  
        // Try to connect to the internet by making an HTTP request to a known URL
        if ( verifyInternetConnectivity() )
            ESP_LOGI(TAG, "Connected successfully to internet");
        else
            ESP_LOGW(TAG, "Failed to connect to internet");
    }
    else
    {
        ESP_LOGW(TAG, "Failed to connect to WiFi: %s", m_config.ssid.data());
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // enter deep sleep mode
    ESP_LOGI(TAG, "Entering deep sleep mode...");
    
    if(sdCardFlushCb)
        sdCardFlushCb();

    // Give some time for the log message to be sent before deep sleep
    delay(100);

    esp_deep_sleep(DEEP_SLEEP_DURATION);
}

void DummyDevice::update()
{}

bool DummyDevice::connectToWiFi()
{
    ESP_LOGI(TAG, "Connecting to WiFi: %s", m_config.ssid.data());

    WiFi.mode(WIFI_STA);

    if(m_config.staticIp != IPAddress{})
        WiFi.config(m_config.staticIp, m_config.gateway, m_config.subnet, m_config.dns);
    
    WiFi.begin(m_config.ssid.c_str(), m_config.password.c_str());

    unsigned long timeout{millis() + WIFI_SETUP_TIMEOUT};
    while (WiFi.status() != WL_CONNECTED && millis() < timeout) 
    {
        Serial.print(".");
        delay(500);
    }

    return WiFi.status() == WL_CONNECTED;
}

bool DummyDevice::verifyInternetConnectivity()
{
    static constexpr const char* CONNECTIVITY_CHECK_URL{"http://connectivitycheck.gstatic.com/generate_204"};

    HTTPClient http{};
    http.setTimeout(5000);
    http.begin(CONNECTIVITY_CHECK_URL);
    int httpCode = http.GET();
    http.end();

    if(httpCode <= 0)
    {
        ESP_LOGW(TAG, "Connectivity HTTP request failed, code: %d", httpCode);
        return false;
    }

    const bool success{httpCode == HTTP_CODE_NO_CONTENT || httpCode == HTTP_CODE_OK};
    if(!success)
        ESP_LOGW(TAG, "Connectivity endpoint returned unexpected status: %d", httpCode);

    return success;
}