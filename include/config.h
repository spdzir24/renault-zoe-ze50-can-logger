/**
 * Configuration Header for Renault Zoe ZE50 CAN Logger
 * RejsaCAN v6.x ESP32-C6
 * 
 * This file contains all compile-time configuration flags and constants.
 * Modify these values to customize behavior without changing core logic.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// HARDWARE CONFIGURATION - RejsaCAN v6.x ESP32-C6
// ============================================================================

// CAN/TWAI Pin Configuration
// Based on RejsaCAN v6.x schematic with SN65HVD230 transceiver
#define CAN_TX_PIN   GPIO_NUM_0   // ESP32-C6 GPIO0 -> SN65HVD230 TXD
#define CAN_RX_PIN   GPIO_NUM_1   // ESP32-C6 GPIO1 -> SN65HVD230 RXD

// Status LED (WS2812B RGB LED on RejsaCAN)
#define STATUS_LED_PIN   GPIO_NUM_8

// CAN Bus Configuration - Renault Zoe uses 500 kbit/s
#define CAN_SPEED_KBPS   500

// ============================================================================
// UART/SERIAL CONFIGURATION
// ============================================================================

// UART baudrate for telemetry output
#define UART_BAUDRATE    115200

// Protocol selection for UART output
#define UART_PROTOCOL_JSONLINES  1  // JSON Lines format (default)
#define UART_PROTOCOL_DEBUG      0  // Human-readable debug format

// Heartbeat interval (milliseconds)
#define HEARTBEAT_INTERVAL_MS    5000

// ============================================================================
// FEATURE ENABLE/DISABLE FLAGS
// ============================================================================

// Passive CAN sniffing (always safe, should remain enabled)
#define ENABLE_PASSIVE_SNIFFING  1

// UDS active polling (DISABLED by default due to Security Gateway sensitivity)
// WARNING: Enable only if you understand the implications!
// The Zoe ZE50 Phase 2 has a strict Security Gateway that may react
// negatively to incorrect UDS timing or excessive bus load.
#define ENABLE_UDS_POLLING       0

// Debug logging to Serial (separate from telemetry)
#define ENABLE_DEBUG_LOGGING     1

// Statistics reporting (bus load, frame counts, etc.)
#define ENABLE_STATISTICS        1

// ============================================================================
// UDS POLLING INTERVALS (only relevant if ENABLE_UDS_POLLING = 1)
// ============================================================================

// Fast polling: critical driving data (e.g., SOC, current, voltage)
#define UDS_FAST_INTERVAL_MS     1000   // 1 second

// Medium polling: moderate frequency data (e.g., temperatures, power limits)
#define UDS_MEDIUM_INTERVAL_MS   5000   // 5 seconds

// Slow polling: static/rarely changing data (e.g., VIN, SOH)
#define UDS_SLOW_INTERVAL_MS     60000  // 60 seconds

// UDS timeout and retry configuration
#define UDS_RESPONSE_TIMEOUT_MS  200    // Maximum wait for ECU response
#define UDS_MAX_RETRIES          2      // Retry attempts before marking failed
#define UDS_INTER_REQUEST_DELAY  50     // Delay between consecutive requests (ms)

// ============================================================================
// DATA PROCESSING
// ============================================================================

// Maximum age of passive data before considered stale (milliseconds)
#define DATA_STALE_THRESHOLD_MS  10000

// Buffer sizes
#define CAN_RX_QUEUE_SIZE        50     // CAN receive queue depth
#define TELEMETRY_BUFFER_SIZE    512    // JSON serialization buffer

// ============================================================================
// VEHICLE-SPECIFIC CONSTANTS - RENAULT ZOE ZE50 PHASE 2
// ============================================================================

// Battery configuration
#define BATTERY_NOMINAL_CAPACITY_KWH  54.7  // Gross capacity
#define BATTERY_USABLE_CAPACITY_KWH   52.0  // Usable capacity
#define BATTERY_CELL_COUNT            192   // 96s2p configuration (12 modules × 16 cells)
#define BATTERY_MODULE_COUNT          12

// Calculation constants derived from reverse engineering
#define RPM_TO_KMH_FACTOR            0.012325  // Motor RPM to vehicle speed

// ============================================================================
// LOGGING LEVELS
// ============================================================================

#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4
#define LOG_LEVEL_VERBOSE 5

// Current log level (set to LOG_LEVEL_INFO for production)
#define CURRENT_LOG_LEVEL  LOG_LEVEL_DEBUG

// ============================================================================
// SAFETY LIMITS
// ============================================================================

// Maximum CAN messages per second to transmit (UDS polling rate limiter)
#define MAX_CAN_TX_PER_SECOND    50

// Error threshold before disabling UDS polling automatically
#define UDS_ERROR_THRESHOLD      10  // Consecutive errors

#endif // CONFIG_H
