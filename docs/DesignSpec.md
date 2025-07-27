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
- Step-by-step process of NFC reading
- Data transmission to software

## 5. Integration with Existing Software
- Interface description
- Data format and protocol

## 6. Future Extensions
- Possible improvements
- Additional features

## 7. References
- Datasheets
- Standards
- Related documentation