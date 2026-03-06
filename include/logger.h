/**
 * Logging System
 * Provides structured logging with severity levels
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "config.h"

// Log macros with severity levels
#if ENABLE_DEBUG_LOGGING

#define LOG_ERROR(tag, format, ...) \
    if (CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR) { \
        Serial.printf("[ERROR][%s] " format "\n", tag, ##__VA_ARGS__); \
    }

#define LOG_WARN(tag, format, ...) \
    if (CURRENT_LOG_LEVEL >= LOG_LEVEL_WARN) { \
        Serial.printf("[WARN][%s] " format "\n", tag, ##__VA_ARGS__); \
    }

#define LOG_INFO(tag, format, ...) \
    if (CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO) { \
        Serial.printf("[INFO][%s] " format "\n", tag, ##__VA_ARGS__); \
    }

#define LOG_DEBUG(tag, format, ...) \
    if (CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG) { \
        Serial.printf("[DEBUG][%s] " format "\n", tag, ##__VA_ARGS__); \
    }

#define LOG_VERBOSE(tag, format, ...) \
    if (CURRENT_LOG_LEVEL >= LOG_LEVEL_VERBOSE) { \
        Serial.printf("[VERBOSE][%s] " format "\n", tag, ##__VA_ARGS__); \
    }

#else

// Logging disabled - no-op macros
#define LOG_ERROR(tag, format, ...)
#define LOG_WARN(tag, format, ...)
#define LOG_INFO(tag, format, ...)
#define LOG_DEBUG(tag, format, ...)
#define LOG_VERBOSE(tag, format, ...)

#endif

#endif // LOGGER_H
