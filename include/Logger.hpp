#pragma once
#include <Arduino.h>
#include <time.h>
#include <stdarg.h>

#define LOG_TAG "homelink"

enum LogLevel { VERBOSE = 0, DEBUG, INFO, WARNING, ERROR, NONE };

class Logger {
public:
  static void setLogLevel(LogLevel level);
  static void log(LogLevel level, const char* tag, const char* file, const char* function, int line, const char* message, ...);
  static void log(LogLevel level, const char* tag, const char* file, const char* function, int line, const __FlashStringHelper* msg);
  static const char* levelToString(LogLevel level);

private:
  static LogLevel currentLogLevel;
};

// Macros for simplified usage with automatic metadata
#define LOGV(message, ...) Logger::log(VERBOSE, LOG_TAG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGD(message, ...) Logger::log(DEBUG, LOG_TAG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGI(message, ...) Logger::log(INFO, LOG_TAG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGW(message, ...) Logger::log(WARNING, LOG_TAG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGE(message, ...) Logger::log(ERROR, LOG_TAG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)

#define LOGFV(msg) Logger::log(VERBOSE, LOG_TAG, __FILE__, __func__, __LINE__, F(msg))
#define LOGFD(msg) Logger::log(DEBUG,   LOG_TAG, __FILE__, __func__, __LINE__, F(msg))
#define LOGFI(msg) Logger::log(INFO,    LOG_TAG, __FILE__, __func__, __LINE__, F(msg))
#define LOGFW(msg) Logger::log(WARNING, LOG_TAG, __FILE__, __func__, __LINE__, F(msg))
#define LOGFE(msg) Logger::log(ERROR,   LOG_TAG, __FILE__, __func__, __LINE__, F(msg))

#define LOGTV(tag, message, ...) Logger::log(VERBOSE, tag, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGTD(tag, message, ...) Logger::log(DEBUG, tag, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGTI(tag, message, ...) Logger::log(INFO, tag, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGTW(tag, message, ...) Logger::log(WARNING, tag, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGTE(tag, message, ...) Logger::log(ERROR, tag, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)