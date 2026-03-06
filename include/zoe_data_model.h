/**
 * Renault Zoe ZE50 Phase 2 Data Model
 * 
 * This file defines the complete telemetry data structure and CAN identifiers
 * for the Renault Zoe ZE50 Phase 2, based on reverse engineering documentation.
 */

#ifndef ZOE_DATA_MODEL_H
#define ZOE_DATA_MODEL_H

#include <Arduino.h>
#include <stdint.h>

// ============================================================================
// DATA SOURCE TYPES
// ============================================================================

enum DataSource {
    SOURCE_UNKNOWN = 0,
    SOURCE_PASSIVE,      // Passively sniffed from CAN bus
    SOURCE_UDS_FAST,     // UDS poll with fast interval
    SOURCE_UDS_MEDIUM,   // UDS poll with medium interval
    SOURCE_UDS_SLOW,     // UDS poll with slow/static interval
    SOURCE_CALCULATED    // Derived from other values
};

// ============================================================================
// CAN IDENTIFIERS - ECU ADDRESSES
// ============================================================================

// Standard 11-bit CAN identifiers for diagnosis
namespace CANId {
    // Diagnostic request/response pairs
    constexpr uint32_t EVC_REQ = 0x7E2;   // Electric Vehicle Controller Request
    constexpr uint32_t EVC_RES = 0x7EA;   // EVC Response
    
    constexpr uint32_t LBC_REQ = 0x7E4;   // Lithium Battery Controller Request
    constexpr uint32_t LBC_RES = 0x7EC;   // LBC Response
    
    constexpr uint32_t PEC_REQ = 0x7E3;   // Power Electronic Controller Request
    constexpr uint32_t PEC_RES = 0x7EB;   // PEC Response
    
    constexpr uint32_t BCM_REQ = 0x771;   // Body Control Module Request
    constexpr uint32_t BCM_RES = 0x779;   // BCM Response
    
    constexpr uint32_t ABS_REQ = 0x760;   // ABS/ESC Request
    constexpr uint32_t ABS_RES = 0x768;   // ABS Response
    
    constexpr uint32_t MULTIMEDIA_REQ = 0x7B5;  // Multimedia/Infotainment Request
    constexpr uint32_t MULTIMEDIA_RES = 0x7BD;  // Multimedia Response
    
    // Passive broadcast frames (free frames)
    constexpr uint32_t AVAILABLE_ENERGY = 0x427;  // Available battery energy (coarse)
    constexpr uint32_t CELL_VOLTAGES = 0x5D7;     // Cell voltages (multiplexed, ZE50 specific)
}

// ============================================================================
// UDS SERVICE IDENTIFIERS AND DATA IDENTIFIERS (DIDs)
// ============================================================================

namespace UDS {
    // Service IDs
    constexpr uint8_t SERVICE_READ_DATA_BY_ID = 0x22;
    constexpr uint8_t RESPONSE_OFFSET = 0x40;  // Response = Request + 0x40
    
    // Positive response: 0x62 (0x22 + 0x40)
    constexpr uint8_t RESPONSE_READ_DATA_BY_ID = 0x62;
    
    // Negative response
    constexpr uint8_t NEGATIVE_RESPONSE = 0x7F;
    
    // Data Identifiers (DIDs) - organized by ECU
    namespace EVC {
        constexpr uint16_t SOC_REAL = 0x20BE;           // Real State of Charge (%)
        constexpr uint16_t ENERGY_PER_SOC = 0x303E;     // Energy per SOC percent (kWh/%)
        constexpr uint16_t HV_VOLTAGE = 0x20FE;         // High Voltage battery voltage (V)
        constexpr uint16_t HV_CURRENT = 0x21CC;         // High Voltage battery current (A)
        constexpr uint16_t MOTOR_RPM = 0x3064;          // Motor RPM (also used for speed calc)
    }
    
    namespace LBC {
        constexpr uint16_t BATTERY_VOLTAGE = 0x3203;    // Battery voltage internal (V)
        constexpr uint16_t BATTERY_CURRENT = 0x3204;    // Battery current internal (A)
        constexpr uint16_t AVAILABLE_DISCHARGE = 0x502C; // Available discharge energy (kWh)
        constexpr uint16_t SOH = 0x0101;                // State of Health (%)
        constexpr uint16_t MAX_CHARGE_POWER = 0x3206;   // Maximum charge power (kW)
        constexpr uint16_t CELL_TEMP_MAX = 0x320B;      // Maximum cell temperature (°C)
    }
    
    namespace PEC {
        constexpr uint16_t INVERTER_TEMP = 0x3001;      // Inverter temperature (°C)
    }
    
    namespace BCM {
        constexpr uint16_t BATTERY_12V = 0x2002;        // 12V auxiliary battery voltage (V)
    }
    
    namespace MULTIMEDIA {
        constexpr uint16_t VIN = 0xF190;                // Vehicle Identification Number (ASCII)
    }
}

// ============================================================================
// TELEMETRY DATA STRUCTURE
// ============================================================================

// Timestamp for data freshness tracking
struct DataPoint {
    float value;
    uint32_t timestamp_ms;  // millis() when data was received
    DataSource source;
    bool valid;             // false if never received or stale
    
    DataPoint() : value(0.0f), timestamp_ms(0), source(SOURCE_UNKNOWN), valid(false) {}
};

// Main telemetry structure
struct ZoeTelemetry {
    // === Battery State ===
    DataPoint soc_real;                  // Real State of Charge (%, 0-100)
    DataPoint soc_display;               // Display SOC (calculated with buffer)
    DataPoint soh;                       // State of Health (%, 0-100)
    
    // === Battery Electrical ===
    DataPoint battery_voltage_evc;       // HV battery voltage from EVC (V)
    DataPoint battery_voltage_lbc;       // HV battery voltage from LBC (V)
    DataPoint battery_current_evc;       // HV battery current from EVC (A, + = discharge)
    DataPoint battery_current_lbc;       // HV battery current from LBC (A)
    
    // === Energy ===
    DataPoint energy_per_soc;            // Energy per 1% SOC (kWh/%)
    DataPoint available_discharge_energy; // Available discharge energy (kWh)
    DataPoint available_energy_coarse;   // From passive frame 0x427 (kWh)
    
    // === Power ===
    DataPoint max_charge_power;          // Maximum charge power limit (kW)
    DataPoint instantaneous_power;       // Calculated: V * I (kW)
    
    // === Temperatures ===
    DataPoint cell_temp_max;             // Maximum cell temperature (°C)
    DataPoint inverter_temp;             // Inverter temperature (°C)
    
    // === Motion ===
    DataPoint motor_rpm;                 // Motor rotational speed (rpm)
    DataPoint vehicle_speed;             // Calculated from RPM (km/h)
    
    // === Auxiliary ===
    DataPoint battery_12v;               // 12V auxiliary battery (V)
    
    // === Cell Voltages (passive, multiplexed) ===
    // Zoe ZE50: 192 cells in 96s2p, but we monitor 96 parallel groups
    DataPoint cell_voltages[96];         // Individual cell/group voltages (V)
    
    // === Static Data ===
    char vin[18];                        // Vehicle Identification Number (17 chars + null)
    bool vin_valid;
    
    // === Metadata ===
    uint32_t last_update_ms;             // Last time any data was updated
    uint32_t passive_frame_count;        // Total passive frames received
    uint32_t uds_request_count;          // Total UDS requests sent
    uint32_t uds_response_count;         // Total successful UDS responses
    uint32_t uds_error_count;            // Total UDS errors
    
    ZoeTelemetry() : vin_valid(false), last_update_ms(0), 
                     passive_frame_count(0), uds_request_count(0),
                     uds_response_count(0), uds_error_count(0) {
        memset(vin, 0, sizeof(vin));
    }
};

// ============================================================================
// DATA VALIDITY HELPERS
// ============================================================================

inline bool isDataValid(const DataPoint& dp, uint32_t current_time_ms, uint32_t stale_threshold_ms) {
    if (!dp.valid) return false;
    return (current_time_ms - dp.timestamp_ms) < stale_threshold_ms;
}

inline void updateDataPoint(DataPoint& dp, float value, DataSource source) {
    dp.value = value;
    dp.timestamp_ms = millis();
    dp.source = source;
    dp.valid = true;
}

inline void invalidateDataPoint(DataPoint& dp) {
    dp.valid = false;
}

#endif // ZOE_DATA_MODEL_H
