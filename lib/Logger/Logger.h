#pragma once

#include <functional>
#include <string>
#include <cstdio>
#include <utility>

using LogHandler = std::function<void(const std::string& logLevel, const std::string& tag, const std::string& message)>;

#if defined(ESP32) || defined(ESP_PLATFORM)
    // ESP32 / ESP-IDF: use native logging — runtime level control available

    #include <esp_log.h>

    #define LOG_ERROR(tag, format, args...)   do { \
        char log_buf[16]; \
        esp_log_write(ESP_LOG_ERROR, tag, "[ERROR] [%s] %s: " format "\n", Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag, ##args); \
        Logger::dispatch("ERROR", tag, format, ##args); \
    } while(0)

    #define LOG_WARN(tag, format, args...)    do { \
        char log_buf[16]; \
        esp_log_write(ESP_LOG_WARN, tag, "[WARN] [%s] %s: " format "\n", Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag, ##args); \
        Logger::dispatch("WARN", tag, format, ##args); \
    } while(0)

    #define LOG_INFO(tag, format, args...)    do { \
        char log_buf[16]; \
        esp_log_write(ESP_LOG_INFO, tag, "[INFO] [%s] %s: " format "\n", Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag, ##args); \
        Logger::dispatch("INFO", tag, format, ##args); \
    } while(0)

    #define LOG_DEBUG(tag, format, args...)   do { \
        char log_buf[16]; \
        esp_log_write(ESP_LOG_DEBUG, tag, "[DEBUG] [%s] %s: " format "\n", Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag, ##args); \
        Logger::dispatch("DEBUG", tag, format, ##args); \
    } while(0)

    #define LOG_VERBOSE(tag, format, args...) do { \
        char log_buf[16]; \
        esp_log_write(ESP_LOG_VERBOSE, tag, "[VERBOSE] [%s] %s: " format "\n", Logger::log_fmt_time(log_buf, sizeof(log_buf), millis()), tag, ##args); \
        Logger::dispatch("VERBOSE", tag, format, ##args); \
    } while(0)
    #endif

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

    template<typename... Args>
    inline void dispatch(const char* logLevel, const char* tag, const char* format, Args... args)
    {
        if (!sdCardLogHandler)
        {
            return;
        }

        char log_buf[256];
        snprintf(log_buf, sizeof(log_buf), format, args...);
        sdCardLogHandler(logLevel, tag, log_buf);
    }
}