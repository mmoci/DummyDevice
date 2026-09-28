#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

using SdCardFlushCb = std::function<void()>;

class DummyDevice 
{
    public:
    struct Config
    {
        std::string ssid{};
        std::string password{};
        IPAddress staticIp{};
        IPAddress gateway{};
        IPAddress subnet{};
        IPAddress dns{};
    };

    DummyDevice(const Config& config);

    void init(const SdCardFlushCb& sdCardFlushCb);
    void update();

    private:
    bool connectToWiFi();
    bool verifyInternetConnectivity();

    static constexpr unsigned long WIFI_SETUP_TIMEOUT = 10000; // Timeout in milliseconds for WiFi setup
    static constexpr unsigned long long DEEP_SLEEP_DURATION = 3*60*60*1000000ULL; //microseconds
    Config m_config{};
};