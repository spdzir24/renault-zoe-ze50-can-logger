/**
 * Passive Decoder Implementation
 */

#include "passive_decoder.h"
#include "config.h"

PassiveDecoder::PassiveDecoder(ZoeTelemetry& telemetry) 
    : telemetry_(telemetry), cell_voltage_index_(0) {
}

bool PassiveDecoder::processFrame(uint32_t id, const uint8_t* data, uint8_t len) {
    // Dispatch to appropriate decoder based on CAN ID
    switch (id) {
        case CANId::AVAILABLE_ENERGY:
            decode_0x427_AvailableEnergy(data, len);
            return true;
            
        case CANId::CELL_VOLTAGES:
            decode_0x5D7_CellVoltages(data, len);
            return true;
            
        default:
            // Frame not recognized - this is normal, many frames on bus
            return false;
    }
}

void PassiveDecoder::decode_0x427_AvailableEnergy(const uint8_t* data, uint8_t len) {
    // Frame 0x427: Available battery energy (coarse)
    // Resolution: 0.1 kWh
    // Byte mapping depends on exact position - typically bytes 2-3 or similar
    // This is a known passive frame that survives the Security Gateway
    
    if (len < 4) {
        LOG_WARN(TAG, "0x427: Insufficient data length: %d", len);
        return;
    }
    
    // Based on reverse engineering: typically a 16-bit value
    // Exact byte position may vary - this is a common pattern:
    // Bytes 2-3: Available energy in 0.1 kWh units
    uint16_t raw_value = (data[2] << 8) | data[3];
    float energy_kwh = raw_value * 0.1f;
    
    updateDataPoint(telemetry_.available_energy_coarse, energy_kwh, SOURCE_PASSIVE);
    
    LOG_DEBUG(TAG, "0x427: Available Energy = %.1f kWh (raw: %u)", energy_kwh, raw_value);
}

void PassiveDecoder::decode_0x5D7_CellVoltages(const uint8_t* data, uint8_t len) {
    // Frame 0x5D7: Cell voltages (multiplexed)
    // 
    // IMPORTANT: In Zoe Phase 1, this was used for ABS data.
    // In Zoe ZE50 Phase 2, this frame was repurposed for cell voltages!
    // 
    // The frame uses multiplexing: one byte indicates which cell group
    // is being transmitted, and the remaining bytes contain voltage data.
    // 
    // Zoe ZE50: 192 cells in 96s2p configuration
    // -> 96 parallel groups are monitored
    
    if (len < 8) {
        LOG_WARN(TAG, "0x5D7: Insufficient data length: %d", len);
        return;
    }
    
    // Byte 0: Multiplexer index (which cell group: 0-95)
    uint8_t cell_index = data[0];
    
    if (cell_index >= 96) {
        LOG_WARN(TAG, "0x5D7: Invalid cell index: %d", cell_index);
        return;
    }
    
    // Bytes 1-2: Cell voltage (exact encoding depends on reverse engineering)
    // Common encoding: 16-bit value, unit: mV or with specific scaling
    // Based on Zoe documentation, typical range: 3.0V - 4.2V per cell
    // Resolution is usually 0.001V or 0.01V
    
    // Example decoding (adjust based on actual observations):
    // Assuming bytes 1-2 contain voltage in mV
    uint16_t raw_voltage_mv = (data[1] << 8) | data[2];
    float voltage_v = raw_voltage_mv * 0.001f;  // Convert mV to V
    
    // Sanity check: Cell voltage should be in reasonable range
    if (voltage_v < 2.5f || voltage_v > 4.5f) {
        LOG_WARN(TAG, "0x5D7: Cell %d voltage out of range: %.3f V (raw: %u)", 
                 cell_index, voltage_v, raw_voltage_mv);
        return;
    }
    
    updateDataPoint(telemetry_.cell_voltages[cell_index], voltage_v, SOURCE_PASSIVE);
    
    LOG_VERBOSE(TAG, "0x5D7: Cell[%d] = %.3f V", cell_index, voltage_v);
    
    // Track multiplexing progress for debugging
    if (cell_index == 95) {
        LOG_DEBUG(TAG, "0x5D7: Complete cell voltage scan received (0-95)");
    }
}
