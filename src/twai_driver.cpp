/**
 * TWAI Driver Implementation
 */

#include "twai_driver.h"

TWAIDriver::TWAIDriver() 
    : initialized_(false), tx_count_(0), rx_count_(0), 
      tx_error_count_(0), rx_error_count_(0) {
}

bool TWAIDriver::begin() {
    if (initialized_) {
        LOG_WARN(TAG, "TWAI already initialized");
        return true;
    }
    
    LOG_INFO(TAG, "Initializing TWAI controller...");
    LOG_INFO(TAG, "  TX Pin: GPIO%d, RX Pin: GPIO%d", CAN_TX_PIN, CAN_RX_PIN);
    LOG_INFO(TAG, "  Speed: %d kbit/s", CAN_SPEED_KBPS);
    
    // Configure TWAI general settings
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        CAN_TX_PIN, 
        CAN_RX_PIN, 
        TWAI_MODE_NORMAL
    );
    
    // Increase queue sizes for better buffering
    g_config.tx_queue_len = 10;
    g_config.rx_queue_len = CAN_RX_QUEUE_SIZE;
    
    // Configure TWAI timing (500 kbit/s for Renault Zoe)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // Configure TWAI filter to accept all frames (promiscuous mode for sniffing)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    // Install TWAI driver
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        LOG_ERROR(TAG, "Failed to install TWAI driver: %d", ret);
        return false;
    }
    
    // Start TWAI controller
    ret = twai_start();
    if (ret != ESP_OK) {
        LOG_ERROR(TAG, "Failed to start TWAI controller: %d", ret);
        twai_driver_uninstall();
        return false;
    }
    
    initialized_ = true;
    LOG_INFO(TAG, "TWAI controller started successfully");
    
    return true;
}

void TWAIDriver::end() {
    if (!initialized_) return;
    
    LOG_INFO(TAG, "Stopping TWAI controller...");
    
    twai_stop();
    twai_driver_uninstall();
    
    initialized_ = false;
    LOG_INFO(TAG, "TWAI controller stopped");
}

bool TWAIDriver::transmit(uint32_t id, const uint8_t* data, uint8_t len, uint32_t timeout_ms) {
    if (!initialized_) {
        LOG_ERROR(TAG, "Cannot transmit: TWAI not initialized");
        return false;
    }
    
    if (len > 8) {
        LOG_ERROR(TAG, "Invalid data length: %d (max 8)", len);
        return false;
    }
    
    // Construct TWAI message
    twai_message_t message;
    message.identifier = id;
    message.data_length_code = len;
    message.flags = TWAI_MSG_FLAG_NONE;  // Standard frame, data frame
    
    // Copy data
    memcpy(message.data, data, len);
    
    // Transmit with timeout
    TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
    esp_err_t ret = twai_transmit(&message, ticks_to_wait);
    
    if (ret == ESP_OK) {
        tx_count_++;
        LOG_VERBOSE(TAG, "TX: ID=0x%03X, Len=%d", id, len);
        return true;
    } else if (ret == ESP_ERR_TIMEOUT) {
        LOG_WARN(TAG, "TX timeout for ID=0x%03X", id);
        tx_error_count_++;
        return false;
    } else {
        LOG_ERROR(TAG, "TX failed for ID=0x%03X: %d", id, ret);
        tx_error_count_++;
        return false;
    }
}

bool TWAIDriver::receive(uint32_t& id, uint8_t* data, uint8_t& len, uint32_t timeout_ms) {
    if (!initialized_) {
        return false;
    }
    
    twai_message_t message;
    TickType_t ticks_to_wait = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    
    esp_err_t ret = twai_receive(&message, ticks_to_wait);
    
    if (ret == ESP_OK) {
        // Extract message fields
        id = message.identifier;
        len = message.data_length_code;
        memcpy(data, message.data, len);
        
        rx_count_++;
        LOG_VERBOSE(TAG, "RX: ID=0x%03X, Len=%d", id, len);
        return true;
    }
    
    // No message available or error
    if (ret != ESP_ERR_TIMEOUT) {
        rx_error_count_++;
    }
    
    return false;
}

bool TWAIDriver::isOperational() {
    if (!initialized_) return false;
    
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        return (status.state == TWAI_STATE_RUNNING || status.state == TWAI_STATE_RECOVERING);
    }
    
    return false;
}

void TWAIDriver::recover() {
    if (!initialized_) return;
    
    LOG_WARN(TAG, "Initiating bus recovery...");
    
    twai_initiate_recovery();
    
    // Wait for recovery to complete
    uint32_t start = millis();
    while ((millis() - start) < 1000) {
        twai_status_info_t status;
        if (twai_get_status_info(&status) == ESP_OK) {
            if (status.state == TWAI_STATE_RUNNING) {
                LOG_INFO(TAG, "Bus recovery successful");
                return;
            }
        }
        delay(10);
    }
    
    LOG_ERROR(TAG, "Bus recovery timeout");
}

void TWAIDriver::getStatistics(uint32_t& tx_count, uint32_t& rx_count, uint32_t& tx_errors, uint32_t& rx_errors) {
    tx_count = tx_count_;
    rx_count = rx_count_;
    tx_errors = tx_error_count_;
    rx_errors = rx_error_count_;
}
