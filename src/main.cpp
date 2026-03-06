/**
 * Renault Zoe ZE50 Phase 2 CAN Logger
 * Main Application Entry Point
 * 
 * Hardware: RejsaCAN v6.x with ESP32-C6-MINI-1-N4
 * Target Vehicle: Renault Zoe ZE50 Phase 2
 * Connection: OBD2 port under steering wheel
 * 
 * This application provides:
 * 1. Passive CAN sniffing (always active, safe)
 * 2. Optional UDS diagnostic polling (disabled by default)
 * 3. UART telemetry output in JSON Lines format
 * 4. Future-ready architecture for UART bridge to SIM70xx/GPS module
 * 
 * Author: Generated for RejsaCAN ESP32-C6 project
 * Date: March 2026
 */

#include <Arduino.h>
#include "config.h"
#include "logger.h"
#include "zoe_data_model.h"
#include "twai_driver.h"
#include "passive_decoder.h"
#include "uds_scanner.h"
#include "telemetry_output.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// Central telemetry data structure
ZoeTelemetry g_telemetry;

// TWAI/CAN driver
TWAIDriver g_twai;

// Passive frame decoder
PassiveDecoder g_passive_decoder(g_telemetry);

// UDS scanner (optional)
UDSScanner g_uds_scanner(g_twai, g_telemetry);

// Telemetry output
TelemetryOutput g_telemetry_output(g_telemetry);

// ============================================================================
// TIMING VARIABLES
// ============================================================================

uint32_t last_heartbeat_ms = 0;
uint32_t last_telemetry_output_ms = 0;
uint32_t last_statistics_ms = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize USB Serial for logging and telemetry output
    Serial.begin(UART_BAUDRATE);
    
    // Wait for USB serial connection (max 3 seconds)
    uint32_t start = millis();
    while (!Serial && (millis() - start) < 3000) {
        delay(10);
    }
    
    // Startup banner
    Serial.println();
    Serial.println("======================================================");
    Serial.println("  Renault Zoe ZE50 Phase 2 CAN Logger");
    Serial.println("  Hardware: RejsaCAN v6.x ESP32-C6");
    Serial.println("  Firmware Version: 1.0.0-prototype");
    Serial.println("======================================================");
    Serial.println();
    
    LOG_INFO("MAIN", "Starting initialization...");
    
    // Log configuration
    LOG_INFO("MAIN", "Configuration:");
    LOG_INFO("MAIN", "  Passive sniffing: %s", ENABLE_PASSIVE_SNIFFING ? "ENABLED" : "DISABLED");
    LOG_INFO("MAIN", "  UDS polling: %s", ENABLE_UDS_POLLING ? "ENABLED" : "DISABLED");
    LOG_INFO("MAIN", "  CAN speed: %d kbit/s", CAN_SPEED_KBPS);
    LOG_INFO("MAIN", "  UART baudrate: %d", UART_BAUDRATE);
    
    // Initialize TWAI/CAN interface
    LOG_INFO("MAIN", "Initializing TWAI/CAN interface...");
    if (!g_twai.begin()) {
        LOG_ERROR("MAIN", "Failed to initialize TWAI! System halted.");
        while (1) {
            delay(1000);
        }
    }
    LOG_INFO("MAIN", "TWAI interface ready");
    
    // Initialize passive decoder
    LOG_INFO("MAIN", "Initializing passive decoder...");
    // PassiveDecoder has no begin() method, it's ready to use
    LOG_INFO("MAIN", "Passive decoder ready");
    
    // Initialize UDS scanner
#if ENABLE_UDS_POLLING
    LOG_INFO("MAIN", "Initializing UDS scanner...");
    g_uds_scanner.begin();
    LOG_INFO("MAIN", "UDS scanner ready");
#else
    LOG_INFO("MAIN", "UDS scanner DISABLED (passive mode only)");
#endif
    
    // Initialize telemetry output
    LOG_INFO("MAIN", "Initializing telemetry output...");
    g_telemetry_output.begin();
    LOG_INFO("MAIN", "Telemetry output ready");
    
    // Initialization complete
    LOG_INFO("MAIN", "Initialization complete!");
    LOG_INFO("MAIN", "System operational. Monitoring CAN bus...");
    Serial.println();
    
    // Send initial heartbeat
    g_telemetry_output.outputHeartbeat();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    uint32_t now = millis();
    
    // ========================================================================
    // 1. PROCESS INCOMING CAN FRAMES (Passive Sniffing)
    // ========================================================================
    
#if ENABLE_PASSIVE_SNIFFING
    uint32_t rx_id;
    uint8_t rx_data[8];
    uint8_t rx_len;
    
    // Process all available frames in queue (non-blocking)
    while (g_twai.receive(rx_id, rx_data, rx_len, 0)) {
        // Update frame counter
        g_telemetry.passive_frame_count++;
        
        // Try to decode frame
        g_passive_decoder.processFrame(rx_id, rx_data, rx_len);
        
        // Update last activity timestamp
        g_telemetry.last_update_ms = now;
    }
#endif
    
    // ========================================================================
    // 2. UDS POLLING (if enabled)
    // ========================================================================
    
#if ENABLE_UDS_POLLING
    g_uds_scanner.update();
#endif
    
    // ========================================================================
    // 3. TELEMETRY OUTPUT
    // ========================================================================
    
    // Output telemetry every 1 second
    if ((now - last_telemetry_output_ms) >= 1000) {
        last_telemetry_output_ms = now;
        g_telemetry_output.output(false);
    }
    
    // ========================================================================
    // 4. HEARTBEAT
    // ========================================================================
    
    if ((now - last_heartbeat_ms) >= HEARTBEAT_INTERVAL_MS) {
        last_heartbeat_ms = now;
        g_telemetry_output.outputHeartbeat();
    }
    
    // ========================================================================
    // 5. STATISTICS (every 60 seconds)
    // ========================================================================
    
#if ENABLE_STATISTICS
    if ((now - last_statistics_ms) >= 60000) {
        last_statistics_ms = now;
        g_telemetry_output.outputStatistics();
        
        // Also log to debug console
        LOG_INFO("STATS", "Passive frames: %u", g_telemetry.passive_frame_count);
        LOG_INFO("STATS", "UDS requests: %u, responses: %u, errors: %u",
                 g_telemetry.uds_request_count,
                 g_telemetry.uds_response_count,
                 g_telemetry.uds_error_count);
    }
#endif
    
    // ========================================================================
    // 6. BUS ERROR RECOVERY
    // ========================================================================
    
    // Check TWAI status and recover if needed
    if (!g_twai.isOperational()) {
        LOG_ERROR("MAIN", "TWAI bus error detected, attempting recovery...");
        g_twai.recover();
    }
    
    // Small delay to prevent tight loop and reduce CPU load
    delay(1);
}
