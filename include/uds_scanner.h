/**
 * UDS Scanner for Renault Zoe ZE50 Phase 2
 * 
 * Implements UDS (Unified Diagnostic Services) Service 0x22 (ReadDataByIdentifier)
 * with ISO-TP (ISO 15765-2) multi-frame support for requesting diagnostic data
 * from ECUs behind the Security Gateway.
 * 
 * IMPORTANT: This functionality is DISABLED by default (see config.h).
 * The Zoe ZE50 Security Gateway is sensitive to timing and bus load.
 * Enable only if you understand the implications and risks.
 */

#ifndef UDS_SCANNER_H
#define UDS_SCANNER_H

#include <Arduino.h>
#include "zoe_data_model.h"
#include "twai_driver.h"
#include "logger.h"

// ISO-TP Frame types
enum class ISOTPFrameType : uint8_t {
    SINGLE_FRAME = 0x00,     // Bits 7-4 = 0x0, Bits 3-0 = length
    FIRST_FRAME = 0x10,      // Bits 7-4 = 0x1, Bits 3-0 = high nibble of length
    CONSECUTIVE_FRAME = 0x20, // Bits 7-4 = 0x2, Bits 3-0 = sequence number
    FLOW_CONTROL = 0x30      // Bits 7-4 = 0x3
};

// UDS request descriptor
struct UDSRequest {
    uint32_t ecu_request_id;   // CAN ID for request
    uint32_t ecu_response_id;  // CAN ID for response
    uint16_t did;              // Data Identifier (PID)
    const char* description;   // Human-readable description
    DataSource poll_class;     // Polling class (FAST, MEDIUM, SLOW)
    
    // Response parsing function pointer
    using ParserFunc = void (*)(ZoeTelemetry&, const uint8_t*, uint8_t);
    ParserFunc parser;
};

class UDSScanner {
public:
    UDSScanner(TWAIDriver& twai, ZoeTelemetry& telemetry);
    
    /**
     * Initialize UDS scanner (register all PIDs)
     */
    void begin();
    
    /**
     * Execute scheduled UDS polling based on intervals
     * Call this from main loop
     */
    void update();
    
    /**
     * Manually request a specific DID from an ECU
     * @return true if request sent successfully
     */
    bool requestDID(uint32_t ecu_req_id, uint32_t ecu_res_id, uint16_t did);
    
    /**
     * Get statistics
     */
    void getStatistics(uint32_t& requests, uint32_t& responses, uint32_t& errors);
    
private:
    TWAIDriver& twai_;
    ZoeTelemetry& telemetry_;
    
    // Request registry
    std::vector<UDSRequest> requests_;
    
    // Timing
    uint32_t last_fast_poll_ms_;
    uint32_t last_medium_poll_ms_;
    uint32_t last_slow_poll_ms_;
    
    // ISO-TP state
    bool waiting_for_response_;
    uint32_t response_timeout_start_;
    uint8_t consecutive_frame_sequence_;
    uint8_t isotp_buffer_[256];
    uint8_t isotp_buffer_len_;
    
    // Error tracking
    uint32_t consecutive_errors_;
    bool uds_disabled_due_to_errors_;
    
    // Statistics
    uint32_t request_count_;
    uint32_t response_count_;
    uint32_t error_count_;
    
    // Helper functions
    void registerRequest(uint32_t ecu_req, uint32_t ecu_res, uint16_t did, 
                        const char* desc, DataSource poll_class,
                        UDSRequest::ParserFunc parser);
    
    void pollClass(DataSource poll_class);
    bool sendUDSRequest(uint32_t ecu_req_id, uint16_t did);
    bool receiveUDSResponse(uint32_t ecu_res_id, uint8_t* data, uint8_t& len);
    void processISOTP(uint32_t id, const uint8_t* data, uint8_t len);
    bool sendFlowControl(uint32_t ecu_req_id);
    
    // Response parsers (static functions)
    static void parse_SOC_Real(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_EnergyPerSOC(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_HV_Voltage_EVC(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_HV_Current_EVC(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Motor_RPM(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Battery_Voltage_LBC(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Battery_Current_LBC(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Available_Discharge(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_SOH(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Max_Charge_Power(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Cell_Temp_Max(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Inverter_Temp(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_Battery_12V(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    static void parse_VIN(ZoeTelemetry& t, const uint8_t* data, uint8_t len);
    
    static constexpr const char* TAG = "UDS";
};

#endif // UDS_SCANNER_H
