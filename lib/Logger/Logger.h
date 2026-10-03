#pragma once

#include <Arduino.h>
#include <functional>
#include <string>
#include <cstdio>
#include <utility>

#if defined(ESP32) || defined(ESP_PLATFORM)
#include <esp_log.h>
#endif

using LogHandler = std::function<void(const std::string& logLevel, const std::string& tag, const std::string& message)>;

#define LOG_ERROR(tag, format, args...)   do { \
    Logger::dispatch("ERROR", tag, format, ##args); \
} while(0)

#define LOG_WARN(tag, format, args...)    do { \
    Logger::dispatch("WARN", tag, format, ##args); \
} while(0)

#define LOG_INFO(tag, format, args...)    do { \
    Logger::dispatch("INFO", tag, format, ##args); \
} while(0)

#define LOG_DEBUG(tag, format, args...)   do { \
    Logger::dispatch("DEBUG", tag, format, ##args); \
} while(0)

#define LOG_VERBOSE(tag, format, args...) do { \
    Logger::dispatch("VERBOSE", tag, format, ##args); \
} while(0)


namespace Logger 
{
    inline LogHandler sdCardLogHandler{nullptr};

    inline void setSdCardLogHandler(LogHandler handler)
    {
        sdCardLogHandler = std::move(handler);
    }

    // Format raw milliseconds as HH:MM:SS.mmm for readability
    inline char* log_fmt_time(char* buf, std::size_t buf_size, unsigned long ms)
    {
        unsigned long s = ms / 1000;
        unsigned long m = s / 60;
        unsigned long h = m / 60;
        snprintf(buf, buf_size, "%02lu:%02lu:%02lu.%03lu", h, m % 60, s % 60, ms % 1000);
        return buf;
    }

    #if defined(ESP32) || defined(ESP_PLATFORM)
        inline const char* logLevelToString(esp_log_level_t logLevel)
        {
            switch (logLevel)
            {
                case ESP_LOG_ERROR:   return "ERROR";
                case ESP_LOG_WARN:    return "WARN";
                case ESP_LOG_INFO:    return "INFO";
                case ESP_LOG_DEBUG:   return "DEBUG";
                case ESP_LOG_VERBOSE: return "VERBOSE";
                default:              return "UNKNOWN";
            }
        }

        inline esp_log_level_t logLevelFromString(const std::string& logLevel)
        {
            if (logLevel == "ERROR")   return ESP_LOG_ERROR;
            if (logLevel == "WARN")    return ESP_LOG_WARN;
            if (logLevel == "INFO")    return ESP_LOG_INFO;
            if (logLevel == "DEBUG")   return ESP_LOG_DEBUG;
            if (logLevel == "VERBOSE") return ESP_LOG_VERBOSE;
            return ESP_LOG_NONE;
        }
    #endif

    template<typename... Args>
    inline void dispatch(const std::string& logLevel, const char* tag, const char* format, Args... args)
    {
        const char* level = logLevel.c_str();
        
        char log_time[16];
        char log_message[256]{};
        snprintf(log_message, sizeof(log_message), format, args...);
        const char* formattedTime = Logger::log_fmt_time(log_time, sizeof(log_time), millis());

        #if defined(ESP32) || defined(ESP_PLATFORM)
            esp_log_write(Logger::logLevelFromString(logLevel), tag, "[%s] [%s] %s: %s\n", level, formattedTime, tag, log_message);
        #else
            Serial.printf("[%s] [%s] %s: %s\n", level, formattedTime, tag, log_message);
        #endif

        if (sdCardLogHandler)
        {
            sdCardLogHandler(level, tag, log_message);
        }
    }
}