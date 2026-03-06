/**
 * Telemetry Output Implementation
 */

#include "telemetry_output.h"

TelemetryOutput::TelemetryOutput(ZoeTelemetry& telemetry)
    : telemetry_(telemetry), last_output_ms_(0) {
}

void TelemetryOutput::begin() {
    LOG_INFO(TAG, "Telemetry output initialized");
    LOG_INFO(TAG, "Protocol: JSON Lines (one JSON object per line)");
    LOG_INFO(TAG, "Baudrate: %d", UART_BAUDRATE);
}

void TelemetryOutput::addDataPoint(JsonObject& obj, const char* key, const DataPoint& dp,
                                   uint32_t current_time_ms, int decimals) {
    if (isDataValid(dp, current_time_ms, DATA_STALE_THRESHOLD_MS)) {
        obj[key] = serialized(String(dp.value, decimals));
    }
}

void TelemetryOutput::output(bool force) {
    uint32_t now = millis();
    
    // Create JSON document
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    
    // Add message type and timestamp
    root["type"] = "telemetry";
    root["timestamp_ms"] = now;
    
    // === Battery State ===
    JsonObject battery_state = root["battery_state"].to<JsonObject>();
    addDataPoint(battery_state, "soc_real_percent", telemetry_.soc_real, now, 1);
    addDataPoint(battery_state, "soh_percent", telemetry_.soh, now, 1);
    
    // === Battery Electrical ===
    JsonObject battery_elec = root["battery_electrical"].to<JsonObject>();
    addDataPoint(battery_elec, "voltage_evc_v", telemetry_.battery_voltage_evc, now, 1);
    addDataPoint(battery_elec, "voltage_lbc_v", telemetry_.battery_voltage_lbc, now, 1);
    addDataPoint(battery_elec, "current_evc_a", telemetry_.battery_current_evc, now, 2);
    addDataPoint(battery_elec, "current_lbc_a", telemetry_.battery_current_lbc, now, 2);
    
    // === Energy ===
    JsonObject energy = root["energy"].to<JsonObject>();
    addDataPoint(energy, "available_discharge_kwh", telemetry_.available_discharge_energy, now, 2);
    addDataPoint(energy, "available_coarse_kwh", telemetry_.available_energy_coarse, now, 1);
    addDataPoint(energy, "energy_per_soc_kwh_per_percent", telemetry_.energy_per_soc, now, 3);
    
    // === Power ===
    JsonObject power = root["power"].to<JsonObject>();
    addDataPoint(power, "instantaneous_kw", telemetry_.instantaneous_power, now, 2);
    addDataPoint(power, "max_charge_kw", telemetry_.max_charge_power, now, 1);
    
    // === Temperatures ===
    JsonObject temps = root["temperatures"].to<JsonObject>();
    addDataPoint(temps, "cell_max_c", telemetry_.cell_temp_max, now, 1);
    addDataPoint(temps, "inverter_c", telemetry_.inverter_temp, now, 1);
    
    // === Motion ===
    JsonObject motion = root["motion"].to<JsonObject>();
    addDataPoint(motion, "motor_rpm", telemetry_.motor_rpm, now, 0);
    addDataPoint(motion, "speed_kmh", telemetry_.vehicle_speed, now, 1);
    
    // === Auxiliary ===
    JsonObject aux = root["auxiliary"].to<JsonObject>();
    addDataPoint(aux, "battery_12v_v", telemetry_.battery_12v, now, 2);
    
    // === Vehicle Info ===
    if (telemetry_.vin_valid) {
        root["vin"] = telemetry_.vin;
    }
    
    // === Cell Voltages (if available) ===
    // Only include if at least some cells have valid data
    bool has_cell_data = false;
    for (int i = 0; i < 96; i++) {
        if (telemetry_.cell_voltages[i].valid) {
            has_cell_data = true;
            break;
        }
    }
    
    if (has_cell_data) {
        JsonArray cells = root["cell_voltages_v"].to<JsonArray>();
        for (int i = 0; i < 96; i++) {
            if (isDataValid(telemetry_.cell_voltages[i], now, DATA_STALE_THRESHOLD_MS)) {
                cells.add(serialized(String(telemetry_.cell_voltages[i].value, 3)));
            } else {
                cells.add(nullptr);  // null for missing/stale data
            }
        }
    }
    
    // Serialize and output
    serializeJson(doc, Serial);
    Serial.println();  // New line to complete JSON Lines format
    
    last_output_ms_ = now;
}

void TelemetryOutput::outputHeartbeat() {
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    
    root["type"] = "heartbeat";
    root["timestamp_ms"] = millis();
    root["uptime_ms"] = millis();
    root["passive_frames"] = telemetry_.passive_frame_count;
    root["uds_requests"] = telemetry_.uds_request_count;
    root["uds_responses"] = telemetry_.uds_response_count;
    root["uds_errors"] = telemetry_.uds_error_count;
    
    serializeJson(doc, Serial);
    Serial.println();
}

void TelemetryOutput::outputStatistics() {
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    
    root["type"] = "statistics";
    root["timestamp_ms"] = millis();
    root["passive_frame_count"] = telemetry_.passive_frame_count;
    root["uds_request_count"] = telemetry_.uds_request_count;
    root["uds_response_count"] = telemetry_.uds_response_count;
    root["uds_error_count"] = telemetry_.uds_error_count;
    
    // Calculate success rate
    if (telemetry_.uds_request_count > 0) {
        float success_rate = (telemetry_.uds_response_count * 100.0f) / telemetry_.uds_request_count;
        root["uds_success_rate_percent"] = serialized(String(success_rate, 1));
    }
    
    serializeJson(doc, Serial);
    Serial.println();
}
