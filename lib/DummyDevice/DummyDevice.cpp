#include "DummyDevice.h"
#include "Logger.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

static const char* TAG = "DummyDevice";

DummyDevice::DummyDevice(const Config& config) : m_config(config) 
{}

void DummyDevice::init(const SdCardFlushCb& sdCardFlushCb)
{
    if(connectToWiFi())
    {
        LOG_INFO(TAG, "Successfully connected to WiFi: %s", m_config.ssid.data());
  
        // Try to connect to the internet by making an HTTP request to a known URL
        if ( !verifyInternetConnectivity() )
            LOG_WARN(TAG, "Failed to connect to internet");
    }
    else
    {
        LOG_WARN(TAG, "Failed to connect to WiFi: %s", m_config.ssid.data());
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // enter deep sleep mode
    LOG_INFO(TAG, "Entering deep sleep mode...");
    
    if(sdCardFlushCb)
        sdCardFlushCb();

    // Ensure serial logs are completely sent out over hardware lines
    Serial.flush();
    delay(100);

    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION); // DEEP_SLEEP_DURATION in microseconds (e.g., 60 * 1000000ULL)
    esp_deep_sleep_start();
}

void DummyDevice::update()
{}

bool DummyDevice::connectToWiFi()
{
    LOG_INFO(TAG, "Connecting to WiFi: %s", m_config.ssid.data());

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);

    if (m_config.staticIp != IPAddress{})
        WiFi.config(m_config.staticIp, m_config.gateway, m_config.subnet, m_config.primaryDns, m_config.secondaryDns);
    
    WiFi.begin(m_config.ssid.c_str(), m_config.password.c_str());

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start < WIFI_SETUP_TIMEOUT)) 
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(300));

        // Print network diagnostics
        LOG_INFO(TAG, "Wi-Fi Connected! Local IP: %s", WiFi.localIP().toString().c_str());
        return true;
    }

    return false;
}

bool DummyDevice::verifyInternetConnectivity()
{
    const std::string_view targetHost = s_targetHosts[getRandomTargetHostIndex()];
    
    // Construct full HTTPS URL dynamically from the host string_view
    const std::string targetUrl = "https://" + std::string(targetHost) + "/";

    // STEP 1: Verify Router DNS Proxy / Resolution
    IPAddress resolvedIP;
    const uint32_t dnsStart = millis();
    
    // Explicitly pass .data() (null-terminated C-string pointer)
    const int dnsResult = WiFi.hostByName(targetHost.data(), resolvedIP);
    const uint32_t dnsTime = millis() - dnsStart;

    if (dnsResult != 1)
    {
        LOG_WARN(TAG, "[FAIL] DNS lookup failed for %s (Took %lu ms).", targetHost.data(), dnsTime);
        
        // Fallback: If domain lookup failed, verify direct IP connectivity (e.g. Google DNS 8.8.8.8) 
        // to diagnose whether cellular WAN is down or ONLY DNS proxy is frozen.
        IPAddress pingIP(8, 8, 8, 8);
        LOG_INFO(TAG, "Testing raw IP connectivity...");
        
        WiFiClient client;
        if (client.connect(pingIP, 53)) 
        {
            LOG_INFO(TAG, "[IP OK] WAN is active, but DNS proxy failed!");
            client.stop();
        }
        return false;
    }

    LOG_INFO(TAG, "[DNS OK] Resolved %s -> %s in %lu ms", 
             targetHost.data(), resolvedIP.toString().c_str(), dnsTime);

    // STEP 2: Simulate Real User HTTPS Web Session
    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Skip certificate validation for performance/memory

    HTTPClient http{};
    http.setTimeout(7000); // 7s timeout accommodates 4G cellular latency

    LOG_INFO(TAG, "Simulating user HTTPS GET request to %s...", targetUrl.c_str());

    if (!http.begin(secureClient, targetUrl.c_str()))
    {
        LOG_WARN(TAG, "[FAIL] Failed to initialize HTTPS client connection for %s", targetHost.data());
        return false;
    }

    // Add realistic Browser Headers
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    http.addHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    http.addHeader("Connection", "close");

    const uint32_t httpStart = millis();
    const int httpCode = http.GET();
    const uint32_t httpTime = millis() - httpStart;
    http.end();

    if (httpCode <= 0)
    {
        LOG_WARN(TAG, "[FAIL] HTTPS request to %s failed, code: %d (%s) after %lu ms", 
                 targetHost.data(), httpCode, http.errorToString(httpCode).c_str(), httpTime);
        return false;
    }

    // Accept HTTP 200 OK or HTTP 301/302 Redirects
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND)
    {
        LOG_INFO(TAG, "[SUCCESS] Reached %s in %lu ms (HTTP Status: %d)", 
                 targetHost.data(), httpTime, httpCode);
        return true;
    }

    LOG_WARN(TAG, "[FAIL] Target %s returned unexpected HTTP status: %d", targetHost.data(), httpCode);
    return false;
}

int DummyDevice::getRandomTargetHostIndex()
{
    return esp_random() % s_targetHosts.size();
}