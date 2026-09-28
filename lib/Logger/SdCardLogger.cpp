#include "Logger.h"
#include "SdCardLogger.h"

#ifdef SD_CARD

SdCardLogger::SdCardLogger(const Config& config)
    : m_config{config}
{}

void SdCardLogger::init()
{
    // Initialize the SD card here
    if(!SD.begin(m_config.csPin))
    {
        // Handle SD card initialization failure
        LOG_ERROR("SdCardLogger", "Failed to initialize SD card");
        return;
    }

    auto resetReason = esp_reset_reason();
    uint32_t bootCounter = readAndUpdateNvsBootCounter(resetReason);
    setupLogFileName(bootCounter);

    if(resetReason == ESP_RST_DEEPSLEEP)
    {
        LOG_INFO("SdCardLogger", "Device woke up from deep sleep");
        m_logFile = SD.open(m_config.logFileName.c_str(), FILE_APPEND);
    }
    else
    {
        LOG_INFO("SdCardLogger", "Device performing a normal boot");
        m_logFile = SD.open(m_config.logFileName.c_str(), FILE_WRITE);
    }

    if(!m_logFile)
    {
        LOG_ERROR("SdCardLogger", "Failed to open log file: %s", m_config.logFileName.c_str());
        return;
    }

    Logger::setSdCardLogHandler([this](const std::string& logLevel, const std::string& tag, const std::string& message){
        log(logLevel, tag, message);
    });
}

void SdCardLogger::flush()
{
    if(!m_logFile)
        return;
    m_logFile.flush();
    m_lastFlushTime = millis();
}

void SdCardLogger::log(const std::string& logLevel, const std::string& tag, const std::string& message)
{
    if(!m_logFile)
        return;

    char log_buf[16];
    m_logFile.printf("[%s] [%s] [%s] %s\n", logLevel.c_str(), Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag.c_str(), message.c_str());
    if(millis() > m_lastFlushTime + FLUSH_INTERVAL_MS)
        flush();
}

void SdCardLogger::setupLogFileName(uint32_t bootCounter)
{
    if(m_config.logFileName[0] != '/')
        m_config.logFileName = "/" + m_config.logFileName;

    m_config.logFileName += "_" + std::to_string(bootCounter) + ".txt";
}

uint32_t SdCardLogger::readAndUpdateNvsBootCounter(esp_reset_reason_t resetReason)
{
    Preferences preferences{};
    preferences.begin("sdlogger", false);
    uint32_t bootCounter = preferences.getUInt("bootCounter", 0);

    if(resetReason == ESP_RST_DEEPSLEEP)
    {
        // Do not increment boot counter on deep sleep wakeup
        preferences.end();
        return bootCounter;
    }
    
    preferences.putUInt("bootCounter", ++bootCounter);
    preferences.end();

    return bootCounter;
}

#endif