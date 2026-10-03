#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>
#include <array>
#include <string_view>
#include <string>

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
        IPAddress primaryDns{};
        IPAddress secondaryDns{};
    };

    DummyDevice(const Config& config);

    void init(const SdCardFlushCb& sdCardFlushCb);
    void update();

    private:
    bool connectToWiFi();
    bool verifyInternetConnectivity();
    int  getRandomTargetHostIndex();

    static constexpr unsigned long WIFI_SETUP_TIMEOUT = 10000; // Timeout in milliseconds for WiFi setup
    static constexpr unsigned long DHCP_TIMEOUT = 1500; // 1.5 seconds in milliseconds
    static constexpr unsigned long long DEEP_SLEEP_DURATION = 5*60*1000000ULL; // 5 minutes in microseconds
    static constexpr std::array<std::string_view, 4> s_targetHosts
    {
        "meteo.hr",
        "google.com",
        "facebook.com",
        "electronics.stackexchange.com"
    };
    Config m_config{};
};