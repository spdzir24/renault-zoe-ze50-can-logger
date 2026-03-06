/**
 * TWAI (CAN) Driver for ESP32-C6
 * 
 * This module provides a clean interface to the ESP32-C6's TWAI (Two-Wire Automotive Interface)
 * controller, which is the ESP32 implementation of the CAN bus protocol.
 * 
 * IMPORTANT: ESP32-C6 uses the native ESP-IDF TWAI driver, not external libraries.
 * The Arduino framework for ESP32-C6 exposes the TWAI API through driver/twai.h
 */

#ifndef TWAI_DRIVER_H
#define TWAI_DRIVER_H

#include <Arduino.h>
#include <driver/twai.h>
#include "config.h"
#include "logger.h"

class TWAIDriver {
public:
    TWAIDriver();
    
    /**
     * Initialize TWAI controller with specified configuration
     * @return true if successful, false otherwise
     */
    bool begin();
    
    /**
     * Stop TWAI controller and release resources
     */
    void end();
    
    /**
     * Transmit a CAN message
     * @param id CAN identifier (11-bit standard)
     * @param data Pointer to data bytes
     * @param len Data length (0-8 bytes)
     * @param timeout_ms Maximum time to wait for transmission (ms)
     * @return true if transmitted successfully
     */
    bool transmit(uint32_t id, const uint8_t* data, uint8_t len, uint32_t timeout_ms = 100);
    
    /**
     * Receive a CAN message (non-blocking)
     * @param id Reference to store received CAN ID
     * @param data Buffer to store received data (must be at least 8 bytes)
     * @param len Reference to store received data length
     * @param timeout_ms Maximum time to wait for reception (0 = non-blocking)
     * @return true if message received
     */
    bool receive(uint32_t& id, uint8_t* data, uint8_t& len, uint32_t timeout_ms = 0);
    
    /**
     * Check if TWAI controller is in operational state
     */
    bool isOperational();
    
    /**
     * Recover from bus-off state
     */
    void recover();
    
    /**
     * Get bus statistics
     */
    void getStatistics(uint32_t& tx_count, uint32_t& rx_count, uint32_t& tx_errors, uint32_t& rx_errors);
    
private:
    bool initialized_;
    uint32_t tx_count_;
    uint32_t rx_count_;
    uint32_t tx_error_count_;
    uint32_t rx_error_count_;
    
    static constexpr const char* TAG = "TWAI";
};

#endif // TWAI_DRIVER_H
