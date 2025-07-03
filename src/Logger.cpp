#include <Logger.hpp>

LogLevel Logger::currentLogLevel = INFO;

static const char* levelColors[] = {
  "\033[35m", // VERBOSE – purple
  "\033[36m", // DEBUG   – cyan
  "\033[34m", // INFO    – blue
  "\033[33m", // WARNING – yellow
  "\033[31m", // ERROR   – red
};
static const char* RESET = "\033[0m";
static const char* levelNames[] = {
  "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR"
};

void Logger::setLogLevel(LogLevel level) {
  currentLogLevel = level;
}

const char* Logger::levelToString(LogLevel level) {
  switch (level) {
    case VERBOSE: return "VERBOSE";
    case DEBUG: return "DEBUG";
    case INFO: return "INFO";
    case WARNING: return "WARNING";
    case ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

void Logger::log(LogLevel level, const char* tag, const char* file, const char* function, int line, const char* message, ...) {
  if (level < currentLogLevel) return;

  if (message == nullptr) {
        Serial.print(levelColors[level]);
        Serial.printf("[%s][%s:%d][%s] [NULL log message!]", tag, file, line, levelNames[level]);
        Serial.println(RESET);
        return;
    }

    char msg[256];

    va_list args;
    va_start(args, message);
    vsnprintf(msg, sizeof(msg), message, args);
    va_end(args);

    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);

    Serial.print(levelColors[level]);
    Serial.printf("[%s][%s][%s:%d][%s] %s", timeStr, tag, file, line, levelNames[level], msg);
    Serial.println(RESET);

    // Serial.print("\033[90m");  // Gray color
    // Serial.printf("   ↳ Free Heap: %d bytes", ESP.getFreeHeap());
    // Serial.println(RESET);
    // Serial.print("\033[90m");  // Gray color
    // Serial.printf("   ↳ Free Frag: %d %%", ESP.getHeapFragmentation());
    // Serial.println(RESET);
}

void Logger::log(LogLevel level, const char* tag, const char* file, const char* function, int line, const __FlashStringHelper* message) {
  if (level < currentLogLevel) return;

  if (message == nullptr) {
    Serial.print(levelColors[level]);
    Serial.printf("[%s][%s:%d][%s] [NULL flash log message!]", tag, file, line, levelNames[level]);
    Serial.println(RESET);
    return;
  }

  char msg[256];
  strncpy_P(msg, (PGM_P)message, sizeof(msg));
  msg[sizeof(msg) - 1] = '\0';

  time_t now = time(nullptr);
  struct tm* tm_info = localtime(&now);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);

  Serial.print(levelColors[level]);
  Serial.printf("[%s][%s][%s:%d][%s] %s", timeStr, tag, file, line, levelNames[level], msg);
  Serial.println(RESET);

  // Serial.print(F("\033[90m   ↳ Free Heap: "));
  // Serial.print(ESP.getFreeHeap());
  // Serial.println(F(" bytes\033[0m"));

  // Serial.print(F("\033[90m   ↳ Free Frag: "));
  // Serial.print(ESP.getHeapFragmentation());
  // Serial.println(F(" %\033[0m"));
}
