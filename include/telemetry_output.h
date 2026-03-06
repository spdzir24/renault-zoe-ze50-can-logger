/**
 * Telemetry Output System
 * 
 * Provides UART output of telemetry data in machine-readable format.
 * Default protocol: JSON Lines (one JSON object per line)
 * 
 * This format is designed for easy parsing by a secondary ESP32 module
 * that will handle MQTT/GPS/cellular connectivity in future expansion.
 */

#ifndef TELEMETRY_OUTPUT_H
#define TELEMETRY_OUTPUT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "zoe_data_model.h"
#include "config.h"
#include "logger.h"

class TelemetryOutput {
public:
    TelemetryOutput(ZoeTelemetry& telemetry);
    
    /**
     * Initialize telemetry output system
     */
    void begin();
    
    /**
     * Output current telemetry snapshot
     * @param force Force output even if data hasn't changed significantly
     */
    void output(bool force = false);
    
    /**
     * Output heartbeat message (system alive indicator)
     */
    void outputHeartbeat();
    
    /**
     * Output statistics
     */
    void outputStatistics();
    
private:
    ZoeTelemetry& telemetry_;
    uint32_t last_output_ms_;
    
    // Helper to add data point to JSON if valid
    void addDataPoint(JsonObject& obj, const char* key, const DataPoint& dp, 
                     uint32_t current_time_ms, int decimals = 2);
    
    static constexpr const char* TAG = "TELEMETRY";
};

#endif // TELEMETRY_OUTPUT_H
