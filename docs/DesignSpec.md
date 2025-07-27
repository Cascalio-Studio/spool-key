# NFC Reader Design Specification

## 1. Introduction
- Purpose of the document
- Project overview

## 2. Hardware Description
### Hardware Platform
For this project, the Waveshare ST25R3911B NFC Board will be used ([product link](https://www.waveshare.com/product/iot-communication/short-range-wireless/nfc-rf/st25r3911b-nfc-board.htm)). This board integrates the ST25R3911B NFC reader IC and is equipped with an STM32 clone microcontroller, which manages communication and control of the NFC functions. The board provides a compact and cost-effective solution for NFC tag reading and supports SPI and USB interfaces for external communication.

The initial development approach is to create custom firmware for the onboard STM32 clone microcontroller, leveraging its compatibility with standard STM32 development tools and libraries. This allows direct control over the NFC reader and peripheral components, such as LEDs, buttons, and buzzer. If programming the clone MCU proves to be unreliable or limited, the board design allows for replacement with a pin-compatible STM32G0B1 microcontroller. This ensures full compatibility with STM32 development environments and provides access to advanced features, better support, and long-term maintainability.

The hardware platform is chosen for its flexibility, availability, and proven performance in NFC applications. The ST25R3911B IC is well-supported and offers robust NFC tag reading capabilities, while the STM32 family is known for its reliability and extensive ecosystem.

### Parts List
### Parts List
- Waveshare ST25R3911B NFC Board: Main board integrating NFC reader and microcontroller
- STM32 clone microcontroller (on board): Handles NFC communication and peripheral control
- (Optional) STM32G0B1 Microcontroller (pin-compatible replacement): Ensures compatibility and advanced features if clone MCU is insufficient
- Winbond W25Q32 SPI NOR Flash (4MB, SOIC-8): External memory for data logging and configuration storage
- CUI Inc. CPT-1205C Passive Piezo Buzzer (3.3V): Provides audio feedback for user actions and status
- Push Button (Start/Stop Reading): User input to initiate or stop NFC tag reading
- Push Button (Reset): Hardware reset for the system
- Status LEDs: Visual indication of power, operation, and error states
- USB Connector (for PC communication): Interface for data transfer and firmware updates

Communication interfaces (SPI, USB, GPIO) will be defined according to the board's capabilities and project requirements. The design allows for easy expansion and integration of additional peripherals if needed.

## 4. Data Flow & Operation Sequence

## 3. Firmware Architecture
### Overview
The firmware will enable the NFC reader device to communicate with the Spool-Coder software via USB using the CDC (virtual COM port) protocol. Its main responsibilities are to handle NFC tag reading/writing, manage user input (buttons), provide status indication (LEDs, buzzer), and exchange data with the PC.

### Main Modules
- **USB CDC Communication Module:** Implements USB CDC (virtual COM port) for data exchange with the PC. Receives commands from Spool-Coder and sends responses/data.
- **NFC Control Module:** Interfaces with the ST25R3911B IC to read/write NFC tags. Handles tag detection, data extraction, and programming.
- **User Interface Module:** Manages buttons (start/stop, reset), LEDs, and buzzer for feedback and status.
- **Data Management Module:** Handles temporary storage of NFC data and manages external flash for logs/configuration if needed.
- **Firmware Update Module:** Supports firmware updates via USB for maintainability and future improvements.

### Operation Sequence
1. Device powers up, initializes hardware and USB CDC connection.
2. Waits for commands from Spool-Coder (e.g., "read tag", "write tag").
3. On "read tag":
   - Activates NFC reader, scans for tag.
   - Reads tag data, sends it to PC via USB CDC.
   - Indicates status via LED/buzzer.
4. On "write tag":
   - Receives new data from PC.
   - Programs NFC tag.
   - Confirms operation to PC and user.
5. Handles errors, user input, and status indication throughout.

### Firmware Flowchart

```
[Start]
   |
   v
[Initialize Hardware & USB CDC]
   |
   v
[Wait for Command from PC]
   |
   v
+-----------------------------+
|  Command Received?          |
+-----------------------------+
   |         | 
   |         v
   |      [No] <-------------------+
   |         |                     |
   v         v                     |
[Yes]   [Idle/Status Indication]   |
   |                               |
   v                               |
[Parse Command]                    |
   |                               |
   v                               |
+-----------------------------+    |
| Command Type?               |    |
+-----------------------------+    |
   |         |         |      |    |
   v         v         v      v    |
[READ_TAG][WRITE_TAG][GET_STATUS][RESET]
   |         |         |      |
   v         v         v      v
[Activate   [Receive   [Send  [Reset
 NFC Reader] Data from Status] Device]
   |         PC]       |      |
   v         |         v      v
[Read Tag   [Write     [Send  [Restart
 Data]      Tag Data]  Response] Firmware]
   |         |         |      |
   v         v         v      v
[Send Data  [Confirm   [Idle/Status Indication]
 to PC]     to PC]
   |         |
   v         v
[Idle/Status Indication]
   |
   v
[Wait for Next Command]
```

### Communication Protocol
The communication protocol between the firmware and Spool-Coder must be defined. It should be simple and robust, supporting commands such as `READ_TAG`, `WRITE_TAG`, `GET_STATUS`, and `RESET`. The firmware will act as a transparent bridge, passing data between the NFC tag and the Spool-Coder software. Encoding and decoding of Bambulab spool data is handled entirely by the software; the device only transmits raw data.

### Supported Features
- USB CDC (virtual COM port) for PC communication
- Firmware update via USB
- Data pass-through for Bambulab spools
- Status indication and user input handling

## 5. Integration with Existing Software

### USB CDC Communication Protocol
The device communicates with the Spool-Coder software via USB CDC (virtual COM port) using a simple, robust, binary protocol. Each message consists of a fixed header, command code, payload length, payload data, and checksum for error detection.

#### Message Structure
| Byte Offset | Field           | Description                                 |
|-------------|-----------------|---------------------------------------------|
| 0           | Start Byte      | 0xAA (indicates start of message)           |
| 1           | Command Code    | 1 byte (see below)                          |
| 2           | Payload Length  | 1 byte (number of payload bytes)            |
| 3..N        | Payload         | N bytes (depends on command)                |
| N+1         | Checksum        | 1 byte (sum of all previous bytes modulo 256)|

#### Command Codes
- 0x01: READ_TAG
- 0x02: WRITE_TAG
- 0x03: GET_STATUS
- 0x04: RESET

#### Example: Read Tag Command
Suppose the host wants to read a tag. The message sent from PC to device:

| Byte | Value | Description           |
|------|-------|----------------------|
| 0    | 0xAA  | Start Byte           |
| 1    | 0x01  | Command: READ_TAG    |
| 2    | 0x00  | Payload Length: 0    |
| 3    | 0xAB  | Checksum: 0xAA+0x01+0x00 = 0xAB |

Device responds with tag data (example, 8 bytes):

| Byte | Value | Description           |
|------|-------|----------------------|
| 0    | 0xAA  | Start Byte           |
| 1    | 0x01  | Command: READ_TAG    |
| 2    | 0x08  | Payload Length: 8    |
| 3-10 | ...   | Tag Data (8 bytes)   |
| 11   | XX    | Checksum: sum of all previous bytes modulo 256 |

#### Example Data Exchange
#### Example Frame Format Table


#### Protocol Frame Format

| Field           | Offset | Size      | Example Value | Description                                 |
|-----------------|--------|-----------|--------------|---------------------------------------------|
| Start Byte      | 0      | 1 byte    | 0xAA         | Indicates start of message                  |
| Command Code    | 1      | 1 byte    | 0x01         | READ_TAG command                            |
| Payload Length  | 2      | 1 byte    | 0x08         | Number of bytes in Data field               |
| Data            | 3      | N bytes   | 0xDE,0xAD... | Tag data (example, N=8)                     |
| Checksum        | 3+N    | 1 byte    | 0xXX         | Sum of all previous bytes modulo 256        |

**Frame Example (READ_TAG response, N=8):**

| Offset | Value | Description           |
|--------|-------|----------------------|
| 0      | 0xAA  | Start Byte           |
| 1      | 0x01  | Command: READ_TAG    |
| 2      | 0x08  | Payload Length: 8    |
| 3      | 0xDE  | Tag Data byte 1      |
| 4      | 0xAD  | Tag Data byte 2      |
| 5      | 0xBE  | Tag Data byte 3      |
| 6      | 0xEF  | Tag Data byte 4      |
| 7      | 0x01  | Tag Data byte 5      |
| 8      | 0x02  | Tag Data byte 6      |
| 9      | 0x03  | Tag Data byte 7      |
| 10     | 0x04  | Tag Data byte 8      |
| 11     | 0xXX  | Checksum             |

**Full Frame (hex):**
`AA 01 08 DE AD BE EF 01 02 03 04 XX`

PC sends: [0xAA, 0x01, 0x00, 0xAB]
Device responds: [0xAA, 0x01, 0x08, 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04, 0xXX]
Where 0xDEADBEEF01020304 is the tag data, and 0xXX is the checksum.

#### Notes
- All multi-byte values are sent in big-endian format.
- The checksum is a simple sum of all previous bytes modulo 256.
- The protocol is extensible for future commands and payloads.

## 6. Future Extensions
- Possible improvements
- Additional features

## 7. References
- Datasheets
- Standards
- Related documentation