/**
 * UDS Scanner Implementation (Part 1)
 */

#include "uds_scanner.h"
#include "config.h"
#include <vector>

UDSScanner::UDSScanner(TWAIDriver& twai, ZoeTelemetry& telemetry)
    : twai_(twai), telemetry_(telemetry),
      last_fast_poll_ms_(0), last_medium_poll_ms_(0), last_slow_poll_ms_(0),
      waiting_for_response_(false), response_timeout_start_(0),
      consecutive_frame_sequence_(0), isotp_buffer_len_(0),
      consecutive_errors_(0), uds_disabled_due_to_errors_(false),
      request_count_(0), response_count_(0), error_count_(0) {
}

void UDSScanner::begin() {
    LOG_INFO(TAG, "Initializing UDS Scanner...");
    
#if !ENABLE_UDS_POLLING
    LOG_WARN(TAG, "UDS polling is DISABLED in config.h");
    LOG_WARN(TAG, "Only passive sniffing will be active.");
    return;
#endif
    
    // Register all known PIDs for Renault Zoe ZE50 Phase 2
    // Organized by ECU and polling frequency
    
    // === EVC (Electric Vehicle Controller) - Fast Polling ===
    registerRequest(CANId::EVC_REQ, CANId::EVC_RES, UDS::EVC::SOC_REAL,
                   "SOC Real", SOURCE_UDS_FAST, parse_SOC_Real);
    
    registerRequest(CANId::EVC_REQ, CANId::EVC_RES, UDS::EVC::HV_VOLTAGE,
                   "HV Voltage (EVC)", SOURCE_UDS_FAST, parse_HV_Voltage_EVC);
    
    registerRequest(CANId::EVC_REQ, CANId::EVC_RES, UDS::EVC::HV_CURRENT,
                   "HV Current (EVC)", SOURCE_UDS_FAST, parse_HV_Current_EVC);
    
    registerRequest(CANId::EVC_REQ, CANId::EVC_RES, UDS::EVC::MOTOR_RPM,
                   "Motor RPM", SOURCE_UDS_FAST, parse_Motor_RPM);
    
    // === EVC - Medium Polling ===
    registerRequest(CANId::EVC_REQ, CANId::EVC_RES, UDS::EVC::ENERGY_PER_SOC,
                   "Energy per SOC%", SOURCE_UDS_MEDIUM, parse_EnergyPerSOC);
    
    // === LBC (Lithium Battery Controller) - Fast Polling ===
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::BATTERY_VOLTAGE,
                   "Battery Voltage (LBC)", SOURCE_UDS_FAST, parse_Battery_Voltage_LBC);
    
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::BATTERY_CURRENT,
                   "Battery Current (LBC)", SOURCE_UDS_FAST, parse_Battery_Current_LBC);
    
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::AVAILABLE_DISCHARGE,
                   "Available Discharge Energy", SOURCE_UDS_FAST, parse_Available_Discharge);
    
    // === LBC - Medium Polling ===
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::MAX_CHARGE_POWER,
                   "Max Charge Power", SOURCE_UDS_MEDIUM, parse_Max_Charge_Power);
    
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::CELL_TEMP_MAX,
                   "Max Cell Temperature", SOURCE_UDS_MEDIUM, parse_Cell_Temp_Max);
    
    // === LBC - Slow Polling ===
    registerRequest(CANId::LBC_REQ, CANId::LBC_RES, UDS::LBC::SOH,
                   "State of Health", SOURCE_UDS_SLOW, parse_SOH);
    
    // === PEC (Power Electronic Controller) - Medium Polling ===
    registerRequest(CANId::PEC_REQ, CANId::PEC_RES, UDS::PEC::INVERTER_TEMP,
                   "Inverter Temperature", SOURCE_UDS_MEDIUM, parse_Inverter_Temp);
    
    // === BCM (Body Control Module) - Medium Polling ===
    registerRequest(CANId::BCM_REQ, CANId::BCM_RES, UDS::BCM::BATTERY_12V,
                   "12V Battery Voltage", SOURCE_UDS_MEDIUM, parse_Battery_12V);
    
    // === Multimedia - Slow Polling (static data) ===
    registerRequest(CANId::MULTIMEDIA_REQ, CANId::MULTIMEDIA_RES, UDS::MULTIMEDIA::VIN,
                   "Vehicle Identification Number", SOURCE_UDS_SLOW, parse_VIN);
    
    LOG_INFO(TAG, "Registered %d UDS requests", requests_.size());
    LOG_INFO(TAG, "UDS polling intervals: FAST=%dms, MEDIUM=%dms, SLOW=%dms",
             UDS_FAST_INTERVAL_MS, UDS_MEDIUM_INTERVAL_MS, UDS_SLOW_INTERVAL_MS);
}

void UDSScanner::registerRequest(uint32_t ecu_req, uint32_t ecu_res, uint16_t did,
                                const char* desc, DataSource poll_class,
                                UDSRequest::ParserFunc parser) {
    UDSRequest req;
    req.ecu_request_id = ecu_req;
    req.ecu_response_id = ecu_res;
    req.did = did;
    req.description = desc;
    req.poll_class = poll_class;
    req.parser = parser;
    requests_.push_back(req);
    
    LOG_DEBUG(TAG, "Registered: ECU 0x%03X, DID 0x%04X - %s", ecu_req, did, desc);
}

void UDSScanner::update() {
#if !ENABLE_UDS_POLLING
    return;  // UDS polling disabled
#endif
    
    // Check if disabled due to errors
    if (uds_disabled_due_to_errors_) {
        return;
    }
    
    uint32_t now = millis();
    
    // Fast polling
    if ((now - last_fast_poll_ms_) >= UDS_FAST_INTERVAL_MS) {
        last_fast_poll_ms_ = now;
        pollClass(SOURCE_UDS_FAST);
    }
    
    // Medium polling
    if ((now - last_medium_poll_ms_) >= UDS_MEDIUM_INTERVAL_MS) {
        last_medium_poll_ms_ = now;
        pollClass(SOURCE_UDS_MEDIUM);
    }
    
    // Slow polling
    if ((now - last_slow_poll_ms_) >= UDS_SLOW_INTERVAL_MS) {
        last_slow_poll_ms_ = now;
        pollClass(SOURCE_UDS_SLOW);
    }
}

void UDSScanner::pollClass(DataSource poll_class) {
    for (const auto& req : requests_) {
        if (req.poll_class != poll_class) continue;
        
        LOG_DEBUG(TAG, "Polling %s (0x%04X)", req.description, req.did);
        
        // Send request
        if (!sendUDSRequest(req.ecu_request_id, req.did)) {
            LOG_WARN(TAG, "Failed to send request for %s", req.description);
            error_count_++;
            consecutive_errors_++;
            
            // Safety: Disable UDS if too many errors
            if (consecutive_errors_ >= UDS_ERROR_THRESHOLD) {
                LOG_ERROR(TAG, "Too many consecutive errors (%d), disabling UDS polling!",
                         consecutive_errors_);
                uds_disabled_due_to_errors_ = true;
                return;
            }
            
            continue;
        }
        
        // Wait for response with timeout
        uint8_t response_data[64];
        uint8_t response_len = 0;
        
        if (receiveUDSResponse(req.ecu_response_id, response_data, response_len)) {
            // Parse response
            if (req.parser) {
                req.parser(telemetry_, response_data, response_len);
            }
            
            response_count_++;
            consecutive_errors_ = 0;  // Reset error counter on success
            
        } else {
            LOG_WARN(TAG, "No response for %s", req.description);
            error_count_++;
            consecutive_errors_++;
        }
        
        // Inter-request delay to avoid overwhelming the bus
        delay(UDS_INTER_REQUEST_DELAY);
    }
}

bool UDSScanner::sendUDSRequest(uint32_t ecu_req_id, uint16_t did) {
    // UDS Service 0x22: Read Data By Identifier
    // Format: [Length, Service, DID_High, DID_Low, padding...]
    
    uint8_t request[8];
    request[0] = 0x03;  // Length: 3 bytes (Service + DID)
    request[1] = UDS::SERVICE_READ_DATA_BY_ID;  // Service 0x22
    request[2] = (did >> 8) & 0xFF;  // DID high byte
    request[3] = did & 0xFF;         // DID low byte
    request[4] = 0xAA;  // Padding
    request[5] = 0xAA;
    request[6] = 0xAA;
    request[7] = 0xAA;
    
    bool success = twai_.transmit(ecu_req_id, request, 8, UDS_RESPONSE_TIMEOUT_MS);
    
    if (success) {
        request_count_++;
        telemetry_.uds_request_count++;
    }
    
    return success;
}

bool UDSScanner::receiveUDSResponse(uint32_t ecu_res_id, uint8_t* data, uint8_t& len) {
    // Wait for response with timeout
    uint32_t start = millis();
    
    while ((millis() - start) < UDS_RESPONSE_TIMEOUT_MS) {
        uint32_t rx_id;
        uint8_t rx_data[8];
        uint8_t rx_len;
        
        if (twai_.receive(rx_id, rx_data, rx_len, 10)) {
            // Check if this is the response we're waiting for
            if (rx_id == ecu_res_id) {
                // Check frame type (ISO-TP)
                uint8_t frame_type = (rx_data[0] >> 4) & 0x0F;
                
                if (frame_type == 0x0) {
                    // Single frame response
                    uint8_t payload_len = rx_data[0] & 0x0F;
                    
                    // Check for positive response (0x62) or negative (0x7F)
                    if (rx_data[1] == UDS::RESPONSE_READ_DATA_BY_ID) {
                        // Positive response: copy payload (skip length and service bytes)
                        len = payload_len - 1;  // Exclude service byte
                        memcpy(data, &rx_data[2], len);
                        telemetry_.uds_response_count++;
                        return true;
                    } else if (rx_data[1] == UDS::NEGATIVE_RESPONSE) {
                        LOG_WARN(TAG, "Negative response: Service=0x%02X, NRC=0x%02X",
                                rx_data[2], rx_data[3]);
                        telemetry_.uds_error_count++;
                        return false;
                    }
                    
                } else if (frame_type == 0x1) {
                    // First frame of multi-frame response
                    // This requires ISO-TP flow control handling
                    LOG_DEBUG(TAG, "Multi-frame response detected, sending flow control");
                    
                    // Send flow control
                    if (sendFlowControl(ecu_res_id)) {
                        // TODO: Implement full ISO-TP multi-frame assembly
                        // For now, log and return false
                        LOG_WARN(TAG, "Multi-frame responses not fully implemented yet");
                    }
                    
                    return false;
                }
            }
        }
    }
    
    // Timeout
    telemetry_.uds_error_count++;
    return false;
}

bool UDSScanner::sendFlowControl(uint32_t ecu_req_id) {
    // ISO-TP Flow Control: Continue to Send (CTS)
    uint8_t flow_control[8];
    flow_control[0] = 0x30;  // Flow control frame, CTS
    flow_control[1] = 0x00;  // Block size: 0 = send all remaining frames
    flow_control[2] = 0x00;  // Separation time: 0 ms
    flow_control[3] = 0xAA;  // Padding
    flow_control[4] = 0xAA;
    flow_control[5] = 0xAA;
    flow_control[6] = 0xAA;
    flow_control[7] = 0xAA;
    
    return twai_.transmit(ecu_req_id, flow_control, 8, 100);
}

void UDSScanner::getStatistics(uint32_t& requests, uint32_t& responses, uint32_t& errors) {
    requests = request_count_;
    responses = response_count_;
    errors = error_count_;
}

// ============================================================================
// RESPONSE PARSERS
// ============================================================================

void UDSScanner::parse_SOC_Real(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x20BE: Real SOC
    // Response: [DID_High, DID_Low, DATA...]
    // Formula: RAW * 0.01 (or single byte 0-100)
    
    if (len < 3) return;
    
    // Skip DID echo (first 2 bytes)
    uint8_t raw_soc = data[2];
    float soc_percent = raw_soc * 1.0f;  // Already in percent
    
    updateDataPoint(t.soc_real, soc_percent, SOURCE_UDS_FAST);
    
    LOG_DEBUG("UDS", "SOC Real: %.1f%%", soc_percent);
}

void UDSScanner::parse_EnergyPerSOC(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x303E: Energy per SOC percent
    // Formula: RAW * 0.01
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float energy_kwh_per_percent = raw * 0.01f;
    
    updateDataPoint(t.energy_per_soc, energy_kwh_per_percent, SOURCE_UDS_MEDIUM);
    
    LOG_DEBUG("UDS", "Energy/SOC: %.3f kWh/%%", energy_kwh_per_percent);
}

void UDSScanner::parse_HV_Voltage_EVC(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x20FE: HV Battery Voltage (EVC)
    // Formula: RAW * 0.1
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    float voltage_v = raw * 0.1f;
    
    updateDataPoint(t.battery_voltage_evc, voltage_v, SOURCE_UDS_FAST);
    
    LOG_DEBUG("UDS", "HV Voltage (EVC): %.1f V", voltage_v);
}

void UDSScanner::parse_HV_Current_EVC(ZoeTelemetry& t, const uint8_t* data, uint8_t len) {
    // DID 0x21CC: HV Battery Current (EVC)
    // Formula: (RAW - 32768) * 0.01
    // Positive = discharge, Negative = charge
    
    if (len < 4) return;
    
    uint16_t raw = (data[2] << 8) | data[3];
    int16_t signed_raw = (int16_t)raw - 32768;
    float current_a = signed_raw * 0.01f;
    
    updateDataPoint(t.battery_current_evc, current_a, SOURCE_UDS_FAST);
    
    // Calculate instantaneous power
    if (t.battery_voltage_evc.valid) {
        float power_kw = (t.battery_voltage_evc.value * current_a) / 1000.0f;
        updateDataPoint(t.instantaneous_power, power_kw, SOURCE_CALCULATED);
    }
    
    LOG_DEBUG("UDS", "HV Current (EVC): %.2f A", current_a);
}
