/**
 * UDS Scanner Implementation (Part 2) - Remaining Parsers
 */

#include "uds_scanner.h"
#include "config.h"

void UDSScanner::parse_Motor_RPM(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x3064: Motor RPM
    // Signed 16-bit value
    
    if (len < 4) return;
    
    int16_t rpm = (int16_t)((data[2] << 8) | data[3]);
    
    updateDataPoint(t.motor_rpm, (float)rpm, SOURCE_UDS_FAST);
    
    // Calculate vehicle speed from RPM
    // Formula derived from reverse engineering: speed [km/h] = RPM * 0.012325
    float speed_kmh = rpm * RPM_TO_KMH_FACTOR;
    updateDataPoint(t.vehicle_speed, speed_kmh, SOURCE_CALCULATED);
    
    LOG_DEBUG("UDS", "Motor RPM: %d, Speed: %.1f km/h", rpm, speed_kmh);
}

void UDSScanner::parse_Battery_Voltage_LBC(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x3203: Battery Voltage (LBC internal)
    // Formula: RAW * 0.5
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float voltage_v = raw * 0.5f;
    
    updateDataPoint(t.battery_voltage_lbc, voltage_v, SOURCE_UDS_FAST);
    
    LOG_DEBUG("UDS", "Battery Voltage (LBC): %.1f V", voltage_v);
}

void UDSScanner::parse_Battery_Current_LBC(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x3204: Battery Current (LBC internal)
    // Formula: (32768 - RAW) * 0.25
    // Positive = discharge, Negative = charge
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    int16_t signed_value = 32768 - (int16_t)raw;
    float current_a = signed_value * 0.25f;
    
    updateDataPoint(t.battery_current_lbc, current_a, SOURCE_UDS_FAST);
    
    LOG_DEBUG("UDS", "Battery Current (LBC): %.2f A", current_a);
}

void UDSScanner::parse_Available_Discharge(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x502C: Available Discharge Energy
    // Formula: RAW * 0.005
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float energy_kwh = raw * 0.005f;
    
    updateDataPoint(t.available_discharge_energy, energy_kwh, SOURCE_UDS_FAST);
    
    LOG_DEBUG("UDS", "Available Discharge: %.2f kWh", energy_kwh);
}

void UDSScanner::parse_SOH(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x0101: State of Health
    // Formula: RAW / 10
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float soh_percent = raw / 10.0f;
    
    updateDataPoint(t.soh, soh_percent, SOURCE_UDS_SLOW);
    
    LOG_INFO("UDS", "State of Health: %.1f%%", soh_percent);
}

void UDSScanner::parse_Max_Charge_Power(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x3206: Maximum Charge Power
    // Formula: RAW * 0.1
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float power_kw = raw * 0.1f;
    
    updateDataPoint(t.max_charge_power, power_kw, SOURCE_UDS_MEDIUM);
    
    LOG_DEBUG("UDS", "Max Charge Power: %.1f kW", power_kw);
}

void UDSScanner::parse_Cell_Temp_Max(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x320B: Maximum Cell Temperature
    // Formula: RAW - 40
    
    if (len < 3) return;
    
    uint8_t raw = data[2];
    float temp_c = (float)raw - 40.0f;
    
    updateDataPoint(t.cell_temp_max, temp_c, SOURCE_UDS_MEDIUM);
    
    LOG_DEBUG("UDS", "Max Cell Temp: %.1f °C", temp_c);
}

void UDSScanner::parse_Inverter_Temp(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x3001: Inverter Temperature
    // Formula: RAW - 40
    
    if (len < 3) return;
    
    uint8_t raw = data[2];
    float temp_c = (float)raw - 40.0f;
    
    updateDataPoint(t.inverter_temp, temp_c, SOURCE_UDS_MEDIUM);
    
    LOG_DEBUG("UDS", "Inverter Temp: %.1f °C", temp_c);
}

void UDSScanner::parse_Battery_12V(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x2002: 12V Auxiliary Battery Voltage
    // Formula: RAW * 0.01
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float voltage_v = raw * 0.01f;
    
    updateDataPoint(t.battery_12v, voltage_v, SOURCE_UDS_MEDIUM);
    
    LOG_DEBUG("UDS", "12V Battery: %.2f V", voltage_v);
    
    // Warning if 12V battery is low
    if (voltage_v < 11.5f) {
        LOG_WARN("UDS", "12V battery voltage LOW: %.2f V (critical!)", voltage_v);
    }
}

void UDSScanner::parse_VIN(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0xF190: Vehicle Identification Number
    // ASCII string, 17 characters
    
    if (len < 17) {
        LOG_WARN("UDS", "VIN response too short: %d bytes", len);
        return;
    }
    
    // Skip DID echo (first 2 bytes), copy VIN (next 17 bytes)
    memcpy(t.vin, &data[2], 17);
    t.vin[17] = '\0';  // Null terminate
    t.vin_valid = true;
    
    LOG_INFO("UDS", "VIN: %s", t.vin);
}
