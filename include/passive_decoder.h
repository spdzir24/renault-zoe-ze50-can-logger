/**
 * Passive CAN Frame Decoder for Renault Zoe ZE50 Phase 2
 * 
 * This module decodes passively received CAN frames that pass through
 * the Security Gateway without requiring active UDS requests.
 * 
 * Key frames:
 * - 0x427: Available battery energy (coarse resolution)
 * - 0x5D7: Cell voltages (multiplexed, ZE50-specific)
 */

#ifndef PASSIVE_DECODER_H
#define PASSIVE_DECODER_H

#include <Arduino.h>
#include "zoe_data_model.h"
#include "logger.h"

class PassiveDecoder {
public:
    PassiveDecoder(ZoeTelemetry& telemetry);
    
    /**
     * Process a received CAN frame
     * @param id CAN identifier
     * @param data Frame data bytes
     * @param len Data length
     * @return true if frame was recognized and decoded
     */
    bool processFrame(uint32_t id, const uint8_t* data, uint8_t len);
    
private:
    ZoeTelemetry& telemetry_;
    
    // Decoder functions for specific frames
    void decode_0x427_AvailableEnergy(const uint8_t* data, uint8_t len);
    void decode_0x5D7_CellVoltages(const uint8_t* data, uint8_t len);
    
    // Cell voltage multiplexing state
    uint8_t cell_voltage_index_;
    
    static constexpr const char* TAG = "PASSIVE";
};

#endif // PASSIVE_DECODER_H
