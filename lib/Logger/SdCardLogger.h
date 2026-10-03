#pragma once

#include <Arduino.h>
#include <cstdint>
#include <string>
#include <Preferences.h>
#include <esp_system.h>

#ifdef SD_CARD

#include <SD.h>

class SdCardLogger 
{
    public:
    struct Config
    {
        uint8_t csPin{};
        std::string logFileName{"/log"};
    };

    SdCardLogger(const Config& config);

    void init();
    void flush();

    private:
    void log(const std::string& logLevel, const std::string& tag, const std::string& message);
    void setupLogFileName(uint32_t bootCounter);
    uint32_t readAndUpdateNvsBootCounter(esp_reset_reason_t resetReason);


    static constexpr unsigned long FLUSH_INTERVAL_MS{2000};

    File m_logFile{};
    unsigned long m_lastFlushTime{};
    Config m_config{};
};

#else

class SdCardLogger 
{
    public:
    struct Config
    {
        uint8_t csPin{};
        std::string logFileName{};
    };

    SdCardLogger(const Config& config){}

    void init(){}
    void flush(){}

    private:
    static constexpr unsigned long FLUSH_INTERVAL_MS{1000};
    Config m_config{};
};

#endif