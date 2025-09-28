/**
 * @file    App/Inc/st25r3911b_registers.h
 * @brief   ST25R3911B NFC Controller Register Definitions
 * @details This file contains all register addresses, bit definitions and commands for the ST25R3911B NFC controller.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_ST25R3911B_REGISTERS_H
#define INC_ST25R3911B_REGISTERS_H

#include <cstdint>

/**
 * @namespace ST25R3911B
 * @brief Contains ST25R3911B NFC controller related definitions.
 */
namespace ST25R3911B
{
    // ============================================================================
    // Register Addresses
    // ============================================================================
    
    /** @brief IO Configuration Register 1 */
    static constexpr uint8_t REG_IO_CONF1           = 0x00;
    /** @brief IO Configuration Register 2 */
    static constexpr uint8_t REG_IO_CONF2           = 0x01;
    /** @brief Operation Control Register */
    static constexpr uint8_t REG_OP_CONTROL         = 0x02;
    /** @brief Mode Definition Register */
    static constexpr uint8_t REG_MODE               = 0x03;
    /** @brief Bit Rate Definition Register */
    static constexpr uint8_t REG_BIT_RATE           = 0x04;
    /** @brief ISO14443A and NFC 106 kbps Settings */
    static constexpr uint8_t REG_ISO14443A_NFC      = 0x05;
    /** @brief ISO14443B Settings Register */
    static constexpr uint8_t REG_ISO14443B          = 0x06;
    /** @brief Stream Mode Definition Register */
    static constexpr uint8_t REG_STREAM_MODE        = 0x07;
    /** @brief Auxiliary Definition Register */
    static constexpr uint8_t REG_AUX                = 0x08;
    /** @brief Receiver Configuration Register 1 */
    static constexpr uint8_t REG_RX_CONF1           = 0x09;
    /** @brief Receiver Configuration Register 2 */
    static constexpr uint8_t REG_RX_CONF2           = 0x0A;
    /** @brief Receiver Configuration Register 3 */
    static constexpr uint8_t REG_RX_CONF3           = 0x0B;
    /** @brief Receiver Configuration Register 4 */
    static constexpr uint8_t REG_RX_CONF4           = 0x0C;
    /** @brief Power and Oscillator Control Register */
    static constexpr uint8_t REG_P2P_RX_CONF        = 0x0D;
    /** @brief Collision Display Register */
    static constexpr uint8_t REG_CORR_CONF1         = 0x0E;
    /** @brief Correlation Configuration Register 2 */
    static constexpr uint8_t REG_CORR_CONF2         = 0x0F;
    
    /** @brief Sleep Mode Control Register */
    static constexpr uint8_t REG_SLEEP_CONF         = 0x10;
    /** @brief Oscillator Control Register */
    static constexpr uint8_t REG_OSC_CONF           = 0x11;
    /** @brief Test Register 1 */
    static constexpr uint8_t REG_TEST1              = 0x12;
    /** @brief Test Register 2 */
    static constexpr uint8_t REG_TEST2              = 0x13;
    /** @brief IO Configuration Register 3 */
    static constexpr uint8_t REG_IO_CONF3           = 0x14;
    /** @brief IO Configuration Register 4 */
    static constexpr uint8_t REG_IO_CONF4           = 0x15;
    /** @brief Measurement Configuration Register */
    static constexpr uint8_t REG_MEAS_CONF          = 0x16;
    /** @brief Antenna Configuration Register */
    static constexpr uint8_t REG_ANT_CONF           = 0x17;
    /** @brief Timer Configuration Register 1 */
    static constexpr uint8_t REG_TIM_CONF1          = 0x18;
    /** @brief Timer Configuration Register 2 */
    static constexpr uint8_t REG_TIM_CONF2          = 0x19;
    /** @brief Regulator Configuration Register */
    static constexpr uint8_t REG_REGULATOR_CONF     = 0x1A;
    /** @brief Field Threshold Register */
    static constexpr uint8_t REG_FIELD_THRESHOLD    = 0x1B;
    /** @brief Regulator Display Register */
    static constexpr uint8_t REG_REGULATOR_DISPLAY  = 0x1C;
    /** @brief RSSI Display Register 1 */
    static constexpr uint8_t REG_RSSI_DISPLAY1      = 0x1D;
    /** @brief RSSI Display Register 2 */  
    static constexpr uint8_t REG_RSSI_DISPLAY2      = 0x1E;
    /** @brief Gain Reduction State Register */
    static constexpr uint8_t REG_GAIN_RED_STATE     = 0x1F;
    
    /** @brief Capacitance Display Register 1 */
    static constexpr uint8_t REG_CAP_SENSOR_DISPLAY = 0x20;
    /** @brief Auxiliary Display Register */
    static constexpr uint8_t REG_AUX_DISPLAY        = 0x21;
    /** @brief Wake-up Timer Control Register 1 */
    static constexpr uint8_t REG_WUP_TIMER_CONTROL1 = 0x22;
    /** @brief Wake-up Timer Control Register 2 */
    static constexpr uint8_t REG_WUP_TIMER_CONTROL2 = 0x23;
    /** @brief Amplitude Measurement Configuration Register */
    static constexpr uint8_t REG_AMPLITUDE_MEAS_CONF = 0x24;
    /** @brief Phase Measurement Configuration Register */
    static constexpr uint8_t REG_PHASE_MEAS_CONF    = 0x25;
    /** @brief Capacitance Measurement Configuration Register */
    static constexpr uint8_t REG_CAPACITANCE_MEAS_CONF = 0x26;
    /** @brief IC Identity Register */
    static constexpr uint8_t REG_IC_IDENTITY         = 0x27;
    
    // ============================================================================
    // FIFO and Test Registers
    // ============================================================================
    
    /** @brief FIFO RX Status Register 1 */
    static constexpr uint8_t REG_FIFO_RX_STATUS1    = 0x28;
    /** @brief FIFO RX Status Register 2 */
    static constexpr uint8_t REG_FIFO_RX_STATUS2    = 0x29;
    /** @brief Collision Display Register */
    static constexpr uint8_t REG_COLLISION_DISPLAY  = 0x2A;
    /** @brief Number of Transmitted Bytes Register 1 */
    static constexpr uint8_t REG_NUM_TX_BYTES1      = 0x2B;
    /** @brief Number of Transmitted Bytes Register 2 */
    static constexpr uint8_t REG_NUM_TX_BYTES2      = 0x2C;
    /** @brief NFCIP Bit Rate Detection Display */
    static constexpr uint8_t REG_NFCIP_BIT_RATE     = 0x2D;
    /** @brief A/D Converter Output Register */
    static constexpr uint8_t REG_AD_CONVERTER_OUTPUT = 0x2E;
    /** @brief Antenna Calibration Display Register */
    static constexpr uint8_t REG_ANT_CAL_DISPLAY    = 0x2F;
    
    /** @brief Antenna Calibration Target Register */
    static constexpr uint8_t REG_ANT_CAL_TARGET     = 0x30;
    /** @brief Antenna Calibration Configuration Register */
    static constexpr uint8_t REG_ANT_CAL_CONF       = 0x31;
    /** @brief Measurement Display Register */
    static constexpr uint8_t REG_MEAS_DISPLAY       = 0x32;
    /** @brief Power Reduction Register */
    static constexpr uint8_t REG_POWER_RED          = 0x33;
    /** @brief EMD Suppression Configuration Register */
    static constexpr uint8_t REG_EMD_SUP_CONF       = 0x34;
    /** @brief Subc Startup Configuration Register */
    static constexpr uint8_t REG_SUBC_START_CONF    = 0x35;
    
    // ============================================================================
    // Interrupt Registers
    // ============================================================================
    
    /** @brief Main Interrupt Register */
    static constexpr uint8_t REG_IRQ_MAIN           = 0x36;
    /** @brief Timer and NFC Interrupt Register */
    static constexpr uint8_t REG_IRQ_TIMER_NFC      = 0x37;
    /** @brief Error and Wake-up Interrupt Register */
    static constexpr uint8_t REG_IRQ_ERROR_WUP      = 0x38;
    /** @brief Target Interrupt Register */
    static constexpr uint8_t REG_IRQ_TARGET         = 0x39;
    
    /** @brief Main Interrupt Mask Register */
    static constexpr uint8_t REG_IRQ_MASK_MAIN      = 0x3A;
    /** @brief Timer and NFC Interrupt Mask Register */
    static constexpr uint8_t REG_IRQ_MASK_TIMER_NFC = 0x3B;
    /** @brief Error and Wake-up Interrupt Mask Register */
    static constexpr uint8_t REG_IRQ_MASK_ERROR_WUP = 0x3C;
    /** @brief Target Interrupt Mask Register */
    static constexpr uint8_t REG_IRQ_MASK_TARGET    = 0x3D;
    
    // ============================================================================
    // FIFO Access
    // ============================================================================
    
    /** @brief FIFO Load Register */
    static constexpr uint8_t REG_FIFO_LOAD          = 0x3E;
    /** @brief FIFO Data Register */
    static constexpr uint8_t REG_FIFO_DATA          = 0x3F;
    
    // ============================================================================
    // Direct Commands (0xC0 - 0xFF)
    // ============================================================================
    
    /** @brief Set Default Command */
    static constexpr uint8_t CMD_SET_DEFAULT        = 0xC1;
    /** @brief Clear FIFO Command */
    static constexpr uint8_t CMD_CLEAR_FIFO         = 0xC2;
    /** @brief Transmit with CRC Command */
    static constexpr uint8_t CMD_TRANSMIT_WITH_CRC  = 0xC4;
    /** @brief Transmit without CRC Command */
    static constexpr uint8_t CMD_TRANSMIT_WITHOUT_CRC = 0xC5;
    /** @brief Transmit REQA Command */
    static constexpr uint8_t CMD_TRANSMIT_REQA      = 0xC6;
    /** @brief Transmit WUPA Command */
    static constexpr uint8_t CMD_TRANSMIT_WUPA      = 0xC7;
    /** @brief NFC Initial Field On Command */
    static constexpr uint8_t CMD_INITIAL_RF_COLLISION = 0xC8;
    /** @brief NFC Response RF Collision Avoidance Command */
    static constexpr uint8_t CMD_RESPONSE_RF_COLLISION_N = 0xC9;
    /** @brief Goto Sleep Command */
    static constexpr uint8_t CMD_GOTO_SLEEP         = 0xCA;
    /** @brief Goto Sleep with Wake-up Timer Command */
    static constexpr uint8_t CMD_GOTO_SLEEP_WU      = 0xCB;
    /** @brief Mask Receive Data Command */
    static constexpr uint8_t CMD_MASK_RECEIVE_DATA  = 0xD0;
    /** @brief Unmask Receive Data Command */
    static constexpr uint8_t CMD_UNMASK_RECEIVE_DATA = 0xD1;
    /** @brief AM Modulation Command */
    static constexpr uint8_t CMD_AM_MOD_STATE_CHANGE = 0xD2;
    /** @brief Measure Amplitude Command */
    static constexpr uint8_t CMD_MEASURE_AMPLITUDE  = 0xD3;
    /** @brief Reset RX Gain Command */
    static constexpr uint8_t CMD_RESET_RXGAIN       = 0xD5;
    /** @brief Adjust Regulators Command */
    static constexpr uint8_t CMD_ADJUST_REGULATORS  = 0xD6;
    /** @brief Calibrate Antenna Command */
    static constexpr uint8_t CMD_CALIBRATE_ANTENNA  = 0xD7;
    /** @brief Measure Phase Command */
    static constexpr uint8_t CMD_MEASURE_PHASE      = 0xD8;
    /** @brief Clear RSSI Command */
    static constexpr uint8_t CMD_CLEAR_RSSI         = 0xD9;
    /** @brief Transparent Mode Command */
    static constexpr uint8_t CMD_TRANSPARENT_MODE   = 0xDC;
    /** @brief Calibrate C-Sensor Command */
    static constexpr uint8_t CMD_CALIBRATE_C_SENSOR = 0xDD;
    /** @brief Measure Capacitance Command */
    static constexpr uint8_t CMD_MEASURE_CAPACITANCE = 0xDE;
    /** @brief Measure Power Supply Command */
    static constexpr uint8_t CMD_MEASURE_VDD        = 0xDF;
    /** @brief Start GP Timer Command */
    static constexpr uint8_t CMD_START_GP_TIMER     = 0xE0;
    /** @brief Start Wake-up Timer Command */
    static constexpr uint8_t CMD_START_WUP_TIMER    = 0xE1;
    /** @brief Start Mask-Receive Timer Command */
    static constexpr uint8_t CMD_START_MASK_RECEIVE_TIMER = 0xE2;
    /** @brief Start No-Response Timer Command */
    static constexpr uint8_t CMD_START_NO_RESPONSE_TIMER = 0xE3;
    /** @brief Test Clearsight Command */
    static constexpr uint8_t CMD_TEST_CLEARSIGHT    = 0xE4;
    /** @brief Test Access FIFO Command */
    static constexpr uint8_t CMD_TEST_ACCESS_FIFO   = 0xE5;
    /** @brief Load PPROM Command */
    static constexpr uint8_t CMD_LOAD_PPROM         = 0xE6;
    /** @brief Space-B Command */
    static constexpr uint8_t CMD_SPACE_B_ACCESS     = 0xFB;
    /** @brief Test Access Register Command */
    static constexpr uint8_t CMD_TEST_ACCESS        = 0xFC;
    /** @brief Load Configuration Command */
    static constexpr uint8_t CMD_LOAD_CONFIG        = 0xFD;
    /** @brief Crop Configuration Command */
    static constexpr uint8_t CMD_CROP_CONFIG        = 0xFE;
    
    // ============================================================================
    // Bit Definitions - Mode Register (0x03)
    // ============================================================================
    
    /** @brief Transmitter Enable */
    static constexpr uint8_t MODE_TARG_EN           = 0x80;
    /** @brief Target Mode Enable */
    static constexpr uint8_t MODE_TARG              = 0x40;
    /** @brief OM (Operation Mode) Mask */
    static constexpr uint8_t MODE_OM_MASK           = 0x3C;
    /** @brief OM - NFC Mode */
    static constexpr uint8_t MODE_OM_NFC            = 0x00;
    /** @brief OM - ISO14443A Mode */
    static constexpr uint8_t MODE_OM_ISO14443A      = 0x04;
    /** @brief OM - ISO14443B Mode */
    static constexpr uint8_t MODE_OM_ISO14443B      = 0x08;
    /** @brief OM - FeliCa Mode */
    static constexpr uint8_t MODE_OM_FELICA         = 0x0C;
    /** @brief OM - Subcarrier Stream Mode */
    static constexpr uint8_t MODE_OM_SUBCARRIER     = 0x10;
    /** @brief NFC Mode - NRTx Bit */
    static constexpr uint8_t MODE_NFCIP1_NRTX       = 0x02;
    /** @brief Transmitter Enable Bit */
    static constexpr uint8_t MODE_TR_EN             = 0x01;
    
    // ============================================================================
    // Bit Definitions - Operation Control Register (0x02)
    // ============================================================================
    
    /** @brief Receiver Enable */
    static constexpr uint8_t OP_CONTROL_RX_EN       = 0x80;
    /** @brief RX Chain Gain Reduction */
    static constexpr uint8_t OP_CONTROL_RX_CRCR     = 0x40;
    /** @brief RX Multiple Enable */
    static constexpr uint8_t OP_CONTROL_RX_MAN      = 0x20;
    /** @brief Collision Detector Enable */
    static constexpr uint8_t OP_CONTROL_TX_CRC      = 0x10;
    /** @brief CRC Error Handling */
    static constexpr uint8_t OP_CONTROL_CRC_EN      = 0x08;
    /** @brief RF Collision Avoidance Enable */
    static constexpr uint8_t OP_CONTROL_RFA_EN      = 0x04;
    /** @brief External Field Detector Enable */
    static constexpr uint8_t OP_CONTROL_EFD_EN      = 0x02;
    /** @brief Oscillator Enable */
    static constexpr uint8_t OP_CONTROL_EN          = 0x01;
    
    // ============================================================================
    // Bit Definitions - Interrupt Registers
    // ============================================================================
    
    // Main Interrupt Register (0x36)
    /** @brief Oscillator Stable Interrupt */
    static constexpr uint8_t IRQ_MAIN_OSC           = 0x80;
    /** @brief FIFO Water Level Interrupt */
    static constexpr uint8_t IRQ_MAIN_FWL           = 0x40;
    /** @brief RX Complete Interrupt */
    static constexpr uint8_t IRQ_MAIN_RXS           = 0x20;
    /** @brief RX Start Interrupt */
    static constexpr uint8_t IRQ_MAIN_RXE           = 0x10;
    /** @brief TX Complete Interrupt */
    static constexpr uint8_t IRQ_MAIN_TXE           = 0x08;
    /** @brief Collision Interrupt */
    static constexpr uint8_t IRQ_MAIN_COL           = 0x04;
    /** @brief Oscillator Frequency Stable Interrupt */
    static constexpr uint8_t IRQ_MAIN_NRE           = 0x02;
    /** @brief FIFO Overflow Interrupt */
    static constexpr uint8_t IRQ_MAIN_EOF           = 0x01;
    
    // Timer and NFC Interrupt Register (0x37)
    /** @brief DCT Interrupt */
    static constexpr uint8_t IRQ_TIMER_DCT          = 0x80;
    /** @brief NFC Target Activation Interrupt */
    static constexpr uint8_t IRQ_TIMER_NFC_T        = 0x40;
    /** @brief NFC Initiator Interrupt */
    static constexpr uint8_t IRQ_TIMER_NFC_I        = 0x20;
    /** @brief General Purpose Timer Interrupt */
    static constexpr uint8_t IRQ_TIMER_GPT          = 0x10;
    /** @brief Mask Receive Timer Interrupt */
    static constexpr uint8_t IRQ_TIMER_MRT          = 0x08;
    /** @brief No Response Timer Interrupt */
    static constexpr uint8_t IRQ_TIMER_NRT          = 0x04;
    /** @brief Wake Up Timer Interrupt */
    static constexpr uint8_t IRQ_TIMER_WUT          = 0x02;
    /** @brief Wake Up Amplitude Interrupt */
    static constexpr uint8_t IRQ_TIMER_WUA          = 0x01;

    // ============================================================================
    // FIFO Constants
    // ============================================================================
    
    /** @brief FIFO Size in bytes */
    static constexpr uint8_t FIFO_SIZE              = 96;
    /** @brief FIFO Water Level */
    static constexpr uint8_t FIFO_WATER_LEVEL       = 64;
    
    // ============================================================================
    // SPI Communication Constants
    // ============================================================================
    
    /** @brief SPI Read Command Mask */
    static constexpr uint8_t SPI_CMD_READ           = 0x40;
    /** @brief SPI Write Command Mask */
    static constexpr uint8_t SPI_CMD_WRITE          = 0x00;
    /** @brief SPI Direct Command Mask */
    static constexpr uint8_t SPI_CMD_DIRECT         = 0xC0;
    
    // ============================================================================
    // IC Identity Values
    // ============================================================================
    
    /** @brief Expected IC Identity Value */
    static constexpr uint8_t IC_IDENTITY_VALUE      = 0x09;
    /** @brief IC Type Mask */
    static constexpr uint8_t IC_TYPE_MASK           = 0x1F;
    /** @brief IC Revision Mask */
    static constexpr uint8_t IC_REV_MASK            = 0xE0;
    
} // namespace ST25R3911B

#endif /* INC_ST25R3911B_REGISTERS_H */