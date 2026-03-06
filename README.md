# Renault Zoe ZE50 Phase 2 CAN Logger

**Professional CAN-Bus telemetry logger for Renault Zoe ZE50 Phase 2 on RejsaCAN v6.x hardware with ESP32-C6**

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange.svg)](https://platformio.org/)
[![ESP32-C6](https://img.shields.io/badge/ESP32-C6-blue.svg)](https://www.espressif.com/en/products/socs/esp32-c6)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

## 💡 Project Overview

This project provides a **production-ready, modular firmware** for logging and monitoring telemetry data from a **Renault Zoe ZE50 Phase 2** electric vehicle via the OBD2 port. It is specifically designed for the **RejsaCAN v6.x** hardware platform featuring the **ESP32-C6-MINI-1-N4** microcontroller with integrated TWAI (CAN) interface and **SN65HVD230** CAN transceiver.

### Key Features

✅ **Passive CAN Sniffing** - Safe, read-only monitoring of CAN bus frames  
✅ **UDS Diagnostic Support** - Optional active polling of ECU diagnostic data (ISO 15765-2)  
✅ **Security Gateway Aware** - Respects Zoe ZE50 Phase 2 SGW limitations  
✅ **JSON Lines Output** - Machine-readable UART telemetry for downstream processing  
✅ **Future-Ready Architecture** - Prepared for UART bridge to SIM70xx/GPS module  
✅ **Comprehensive Data Model** - Battery, motor, temperatures, cell voltages, SOC, SOH  
✅ **Robust Error Handling** - Automatic bus recovery, timeout protection, retry logic  
✅ **Highly Configurable** - Compile-time flags for all major features  

---

## 🛠️ Hardware Requirements

### Target Platform

- **Board**: RejsaCAN v6.x
- **Microcontroller**: ESP32-C6-MINI-1-N4 (4MB Flash)
- **CAN Transceiver**: SN65HVD230DR (dual, high-speed CAN)
- **Connection**: OBD2 port (under steering wheel)

### Pin Configuration (RejsaCAN v6.x)

| Function | ESP32-C6 GPIO | Connection |
|----------|---------------|------------|
| CAN TX   | GPIO0         | SN65HVD230 TXD |
| CAN RX   | GPIO1         | SN65HVD230 RXD |
| Status LED | GPIO8       | WS2812B RGB LED |

### Vehicle

- **Make/Model**: Renault Zoe
- **Generation**: ZE50 (Phase 2, 52 kWh battery)
- **Years**: 2019 onwards (with C1A platform and Security Gateway)
- **Bus Speed**: 500 kbit/s

---

## 🏛️ Architecture Overview

### Design Philosophy

The firmware follows a **modular, layered architecture** designed for:

1. **Separation of Concerns** - Hardware abstraction, business logic, and I/O are cleanly separated
2. **Testability** - Each module can be tested independently
3. **Extensibility** - New PIDs, frames, and output formats can be added with minimal changes
4. **Safety** - Passive mode is default; active UDS polling is opt-in and rate-limited
5. **Future-Proofing** - UART protocol designed for seamless integration with external modules

### Module Structure

```
renault-zoe-ze50-can-logger/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── config.h             # Central configuration (EDIT THIS)
│   ├── logger.h             # Debug logging macros
│   ├── zoe_data_model.h     # Telemetry data structures and CAN IDs
│   ├── twai_driver.h        # ESP32-C6 TWAI/CAN driver
│   ├── passive_decoder.h    # Passive frame decoder
│   ├── uds_scanner.h        # UDS diagnostic scanner
│   └── telemetry_output.h   # UART telemetry output
├── src/
│   ├── main.cpp             # Main application logic
│   ├── twai_driver.cpp      # TWAI implementation
│   ├── passive_decoder.cpp  # Passive decoder implementation
│   ├── uds_scanner.cpp      # UDS core logic
│   ├── uds_scanner_parsers.cpp  # UDS response parsers
│   └── telemetry_output.cpp # Telemetry output implementation
└── README.md             # This file
```

### Data Flow

```
                   ╭─────────────────╮
                   │  Renault Zoe    │
                   │  OBD2 Port      │
                   │  (CAN 500k)     │
                   ╰────────┬───────╯
                            │
                   ╭────────┴───────╮
                   │  SN65HVD230    │
                   │  Transceiver   │
                   ╰────────┬───────╯
                            │
          ╭─────────────┴─────────────╮
          │     ESP32-C6 TWAI       │
          │    (Hardware CAN)      │
          ╰─────────┬─────────────╯
                   │
       ╭───────────┼───────────╮
       │                           │
   ╭───┴───╮                  ╭───┴───╮
   │ Passive │                  │  UDS   │
   │ Decoder │                  │Scanner│
   ╰───┬───╯                  ╰───┬───╯
       │                           │
       └───────────┬───────────┘
                   │
          ╭────────┴────────╮
          │  Telemetry Data   │
          │  Model (Unified)  │
          ╰────────┬────────╯
                   │
          ╭────────┴────────╮
          │ JSON Lines UART  │
          │ Output (115200)  │
          ╰────────┬────────╯
                   │
          ╭────────┴────────╮
          │ Future: UART     │
          │ Bridge to        │
          │ SIM70xx + GPS    │
          ╰─────────────────╯
```

---

## 🚀 Quick Start

### Prerequisites

1. **PlatformIO** installed (VS Code extension or CLI)
2. **RejsaCAN v6.x board** with ESP32-C6
3. **USB-C cable** for programming and serial monitor
4. **OBD2 cable** for vehicle connection
5. **Renault Zoe ZE50 Phase 2** vehicle

### Installation

1. **Clone repository**:
   ```bash
   git clone https://github.com/spdzir24/renault-zoe-ze50-can-logger.git
   cd renault-zoe-ze50-can-logger
   ```

2. **Open in VS Code with PlatformIO**:
   - File → Open Folder → Select project directory
   - PlatformIO will auto-detect `platformio.ini`

3. **Review configuration** (optional but recommended):
   - Edit `include/config.h`
   - Check pin assignments, intervals, enable/disable features

4. **Build firmware**:
   ```bash
   pio run
   ```

5. **Upload to board**:
   ```bash
   pio run --target upload
   ```

6. **Open serial monitor**:
   ```bash
   pio device monitor
   ```
   Or use VS Code's built-in serial monitor.

### First Run

1. **Connect RejsaCAN to vehicle OBD2 port**
2. **Turn on vehicle ignition** (do not start motor if not needed)
3. **Connect USB cable** to computer
4. **Open serial monitor** at 115200 baud
5. **Observe output**:
   - Initialization messages
   - Passive CAN frames being decoded
   - JSON Lines telemetry every 1 second
   - Heartbeat messages every 5 seconds

---

## ⚙️ Configuration

### Primary Configuration File: `include/config.h`

All major settings are centralized in `config.h`. Edit this file to customize behavior:

#### Hardware Configuration

```cpp
#define CAN_TX_PIN   GPIO_NUM_0   // TWAI TX pin
#define CAN_RX_PIN   GPIO_NUM_1   // TWAI RX pin
#define CAN_SPEED_KBPS   500      // Zoe uses 500 kbit/s
```

#### Feature Flags

```cpp
#define ENABLE_PASSIVE_SNIFFING  1  // Safe, always recommended
#define ENABLE_UDS_POLLING       0  // DISABLED by default (Security Gateway!)
#define ENABLE_DEBUG_LOGGING     1  // Serial debug output
#define ENABLE_STATISTICS        1  // Periodic statistics
```

#### UDS Polling Intervals (only if `ENABLE_UDS_POLLING = 1`)

```cpp
#define UDS_FAST_INTERVAL_MS     1000   // Critical data (SOC, voltage, current)
#define UDS_MEDIUM_INTERVAL_MS   5000   // Moderate data (temps, limits)
#define UDS_SLOW_INTERVAL_MS     60000  // Static data (VIN, SOH)
```

#### Safety Limits

```cpp
#define UDS_RESPONSE_TIMEOUT_MS  200    // ECU response timeout
#define UDS_MAX_RETRIES          2      // Retry attempts
#define UDS_ERROR_THRESHOLD      10     // Auto-disable after N errors
#define MAX_CAN_TX_PER_SECOND    50     // Rate limiter
```

### Enabling UDS Polling

**⚠️ WARNING**: The Renault Zoe ZE50 Phase 2 has a **Security Gateway** that filters and controls access to ECUs. Incorrect UDS timing or excessive bus load can trigger gateway protection mechanisms.

**Only enable UDS polling if you understand the risks!**

To enable:

1. Edit `include/config.h`:
   ```cpp
   #define ENABLE_UDS_POLLING  1
   ```

2. Rebuild and upload:
   ```bash
   pio run --target upload
   ```

3. Monitor serial output for errors:
   - If you see "Too many consecutive errors, disabling UDS polling!", the system has automatically disabled UDS for safety
   - Reduce polling intervals or disable specific PIDs if needed

---

## 📊 Monitored Data Points

### Passive Frames (Always Safe)

| Frame ID | Description | Data | Typical Update Rate |
|----------|-------------|------|--------------------|
| `0x427`  | Available battery energy | kWh (coarse) | ~1 Hz |
| `0x5D7`  | Cell voltages (multiplexed) | 96 cell groups, V | Sequential scan |

### UDS Diagnostic Data (Optional)

Organized by **polling class** and **ECU**:

#### Fast Polling (1 second interval)

**EVC (0x7E2 / 0x7EA)**:
- `0x20BE` - Real State of Charge (%, 0.01 resolution)
- `0x20FE` - HV Battery Voltage (V, 0.1 resolution)
- `0x21CC` - HV Battery Current (A, 0.01 resolution, offset 32768)
- `0x3064` - Motor RPM (signed 16-bit)

**LBC (0x7E4 / 0x7EC)**:
- `0x3203` - Battery Voltage internal (V, 0.5 resolution)
- `0x3204` - Battery Current internal (A, 0.25 resolution, offset 32768)
- `0x502C` - Available Discharge Energy (kWh, 0.005 resolution)

#### Medium Polling (5 second interval)

**EVC**:
- `0x303E` - Energy per SOC% (kWh/%, 0.01 resolution)

**LBC**:
- `0x3206` - Maximum Charge Power (kW, 0.1 resolution)
- `0x320B` - Maximum Cell Temperature (°C, offset -40)

**PEC (0x7E3 / 0x7EB)**:
- `0x3001` - Inverter Temperature (°C, offset -40)

**BCM (0x771 / 0x779)**:
- `0x2002` - 12V Auxiliary Battery Voltage (V, 0.01 resolution)

#### Slow Polling (60 second interval)

**LBC**:
- `0x0101` - State of Health (%, 0.1 resolution)

**Multimedia (0x7B5 / 0x7BD)**:
- `0xF190` - Vehicle Identification Number (VIN, 17 ASCII chars)

---

## 📝 UART Protocol

### JSON Lines Format

The system outputs telemetry via USB/UART in **JSON Lines** format:
- **One JSON object per line**
- **Newline-terminated** (`\n`)
- **Machine-readable** for easy parsing
- **Human-inspectable** for debugging

### Message Types

#### 1. Telemetry (`type: "telemetry"`)

Output every 1 second with current vehicle data:

```json
{
  "type": "telemetry",
  "timestamp_ms": 123456,
  "battery_state": {
    "soc_real_percent": "87.5",
    "soh_percent": "98.2"
  },
  "battery_electrical": {
    "voltage_evc_v": "385.2",
    "current_evc_a": "-12.34"
  },
  "energy": {
    "available_discharge_kwh": "45.67"
  },
  "power": {
    "instantaneous_kw": "-4.75"
  },
  "motion": {
    "speed_kmh": "50.2",
    "motor_rpm": "4073"
  }
}
```

#### 2. Heartbeat (`type: "heartbeat"`)

Output every 5 seconds to indicate system is alive:

```json
{
  "type": "heartbeat",
  "timestamp_ms": 123456,
  "uptime_ms": 123456,
  "passive_frames": 5432,
  "uds_requests": 120,
  "uds_responses": 118,
  "uds_errors": 2
}
```

#### 3. Statistics (`type: "statistics"`)

Output every 60 seconds with performance metrics:

```json
{
  "type": "statistics",
  "timestamp_ms": 123456,
  "passive_frame_count": 5432,
  "uds_request_count": 120,
  "uds_response_count": 118,
  "uds_error_count": 2,
  "uds_success_rate_percent": "98.3"
}
```

### Parsing Example (Python)

```python
import serial
import json

ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
    line = ser.readline().decode('utf-8').strip()
    try:
        data = json.loads(line)
        if data['type'] == 'telemetry':
            soc = data['battery_state'].get('soc_real_percent')
            if soc:
                print(f"SOC: {soc}%")
    except json.JSONDecodeError:
        pass  # Debug log line, not JSON
```

---

## 🚧 Known Limitations & Risks

### Security Gateway (SGW) Behavior

**Critical Understanding**: The Zoe ZE50 Phase 2 Security Gateway is **NOT a transparent bridge**. It:

1. **Filters** all traffic between OBD2 port and internal CAN buses
2. **Whitelists** only specific UDS requests
3. **Rate-limits** diagnostic queries
4. **Blocks** most passive broadcast frames (only 0x427 and 0x5D7 survive reliably)
5. **Terminates** diagnostic sessions on protocol violations

### Tested Observations

✅ **Works reliably**:
- Passive sniffing of 0x427 and 0x5D7
- UDS requests at 1-5 second intervals
- Single-frame UDS responses

⚠️ **May cause issues**:
- UDS polling faster than 500ms
- Sending non-whitelisted PIDs (gateway ignores or NRC)
- Multi-frame responses with slow flow control

❌ **Does NOT work**:
- Direct access to HEV-CAN broadcast frames (filtered by SGW)
- Write operations (Service 0x2E, 0x31, etc.) → blocked by SGW
- Extended diagnostic sessions without authentication

### Comparison to Zoe Phase 1 (ZE40)

| Feature | Phase 1 (ZE40) | Phase 2 (ZE50) |
|---------|----------------|----------------|
| Passive frames | ~100+ visible | Only 2-3 visible |
| UDS access | Direct ECU access | Through SGW filter |
| Frame 0x5D7 | ABS data | Cell voltages (repurposed) |
| Diagnostic freedom | High | Restricted |

### Recommended Usage

1. **Start with passive mode only** (`ENABLE_UDS_POLLING = 0`)
2. **Test in a safe environment** (not while driving initially)
3. **Enable UDS cautiously** and monitor for errors
4. **If UDS errors occur**, system will auto-disable after 10 consecutive failures
5. **Respect bus timing** – never poll faster than necessary

---

## 🔧 Troubleshooting

### No CAN Traffic Received

**Symptoms**: `passive_frame_count` stays at 0

**Possible Causes**:
1. Vehicle ignition not on
2. Incorrect pin configuration
3. CAN transceiver not powered
4. OBD2 cable issue

**Solutions**:
- Verify vehicle is in ACC or ON mode
- Check `config.h` pin assignments match hardware
- Measure voltage on CAN_H and CAN_L (should be ~2.5V idle, differential signal when active)

### UDS Requests Timeout

**Symptoms**: `uds_error_count` increasing, "No response" warnings

**Possible Causes**:
1. Security Gateway blocking requests
2. Incorrect ECU address or DID
3. Polling too fast
4. Bus timing issues

**Solutions**:
- Increase `UDS_RESPONSE_TIMEOUT_MS` to 300-500ms
- Reduce polling frequency (increase intervals)
- Check if specific PID is whitelisted (see Zoe documentation)
- System will auto-disable UDS after 10 errors for safety

### 12V Battery Warning

**Symptoms**: Log shows "12V battery voltage LOW"

**Cause**: Auxiliary 12V battery voltage below 11.5V

**Action**:
- This is a **critical warning** from the vehicle
- Charge or replace 12V battery soon
- Low 12V can cause erratic behavior, including SGW issues

### Compilation Errors

**Missing `driver/twai.h`**:
- Ensure platform is set to `espressif32` with latest version
- ESP32-C6 support requires ESP-IDF 5.0+

**ArduinoJson errors**:
- Verify `lib_deps` in `platformio.ini` includes ArduinoJson v7.x
- Run `pio lib install` to fetch dependencies

---

## 🔮 Future Expansion: UART Bridge Module

### Planned Architecture

The current firmware outputs telemetry via UART in JSON Lines format to **prepare for a second ESP32 module** that will handle:

1. **Cellular connectivity** (SIM70xx modem)
2. **GPS positioning** (integrated or external)
3. **MQTT broker connection** (e.g., for Home Assistant, ABRP, InfluxDB)
4. **Remote data logging**

### Integration Points

**Hardware**:
- Connect second ESP32's UART RX to RejsaCAN's UART TX (GPIO pin TBD)
- Common ground between modules
- Independent power regulation

**Software**:
- Second ESP32 parses JSON Lines from UART
- Enriches data with GPS coordinates and timestamp
- Publishes to MQTT broker (e.g., `zoe/telemetry`)
- No changes needed to RejsaCAN firmware!

**Example MQTT Topic Structure**:
```
zoe/telemetry        → Full telemetry JSON
zoe/location         → GPS lat/lon
zoe/battery/soc      → Individual data points
zoe/battery/voltage
zoe/status           → System health
```

This **separation of concerns** keeps the CAN interface module focused on vehicle communication, while the bridge module handles internet connectivity.

---

## 📚 References & Credits

### Hardware

- **RejsaCAN**: [github.com/MagnusThome/RejsaCAN-ESP32](https://github.com/MagnusThome/RejsaCAN-ESP32)
- **ESP32-C6 Datasheet**: [Espressif Documentation](https://www.espressif.com/en/products/socs/esp32-c6)

### Reverse Engineering

- **Renault Zoe ZE50 CAN-Bus Referenztabelle** (project-internal documentation)
- **CanZE Project**: [github.com/fesch/CanZE](https://github.com/fesch/CanZE) - Android app for Zoe Phase 1/2
- **GoingElectric Forum**: [Zoe discussion threads](https://www.goingelectric.de/forum/viewforum.php?f=211)

### Technical Standards

- **UDS**: ISO 14229 (Unified Diagnostic Services)
- **ISO-TP**: ISO 15765-2 (Transport Protocol for CAN)
- **CAN**: ISO 11898 (Controller Area Network)

---

## 📜 License

MIT License - See `LICENSE` file for details.

---

## ❤️ Acknowledgments

- **Magnus Thomé** for the RejsaCAN hardware platform
- **CanZE community** for extensive Zoe reverse engineering
- **Espressif** for ESP32-C6 and excellent documentation
- **Renault Zoe owners** sharing diagnostic knowledge

---

## ❓ Support & Contributing

For issues, questions, or contributions:

1. **GitHub Issues**: [Report bugs or request features](https://github.com/spdzir24/renault-zoe-ze50-can-logger/issues)
2. **Pull Requests**: Contributions welcome!
3. **Discussions**: Share your findings and improvements

**Before reporting an issue**, please:
- Check this README thoroughly
- Review `config.h` settings
- Enable debug logging and provide serial output
- Specify hardware version and vehicle year/model

---

**Happy logging! 🚗⚡📊**
