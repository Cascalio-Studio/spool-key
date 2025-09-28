/**
 * @file    App/Src/st25r3911b.cpp
 * @brief   ST25R3911B NFC Controller Implementation
 * @details This file contains the implementation of the ST25R3911B NFC controller class.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

/**
 * @include necessary headers
 */
#include "st25r3911b.h"
#include "FreeRTOS.h"
#include "task.h"

namespace NFC
{
    // ============================================================================
    // Constructor and Destructor
    // ============================================================================

    ST25R3911B::ST25R3911B(const NFCConfig& config)
        : _config(config)
        , _initialized(false)
        , _currentProtocol(NFCProtocol::NFC_A)
        , _fieldState(NFCField::OFF)
        , _interruptPending(false)
    {
        // Set interrupt callback if GPIO interrupt is available
        // Note: Lambda callbacks not supported with function pointers - callback will be set externally
    }

    ST25R3911B::~ST25R3911B()
    {
        Deinitialize();
    }

    // ============================================================================
    // Initialization and Configuration
    // ============================================================================

    NFCStatus ST25R3911B::Initialize(void)
    {
        if (_initialized) {
            return NFCStatus::OK;
        }

        if (!_config.spiMaster || !_config.spiMaster->IsInitialized()) {
            return NFCStatus::NOT_INITIALIZED;
        }

        // Reset the controller
        NFCStatus status = Reset();
        if (status != NFCStatus::OK) {
            return status;
        }

        // Verify IC identity
        uint8_t identity;
        status = GetIdentity(identity);
        if (status != NFCStatus::OK) {
            return status;
        }

        if ((identity & ::ST25R3911B::IC_TYPE_MASK) != ::ST25R3911B::IC_IDENTITY_VALUE) {
            return NFCStatus::ERROR;
        }

        // Configure default settings
        status = configureDefaults();
        if (status != NFCStatus::OK) {
            return status;
        }

        // Set default protocol
        status = SetProtocol(_config.defaultProtocol);
        if (status != NFCStatus::OK) {
            return status;
        }

        _initialized = true;
        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::Deinitialize(void)
    {
        if (!_initialized) {
            return NFCStatus::OK;
        }

        // Turn off RF field
        SetField(NFCField::OFF);

        // Clear all interrupt masks
        SetInterruptMasks(0x00, 0x00, 0x00);

        // Execute set default command
        ExecuteCommand(::ST25R3911B::CMD_SET_DEFAULT);

        _initialized = false;
        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::Reset(void)
    {
        // Execute set default command to reset all registers
        NFCStatus status = ExecuteCommand(::ST25R3911B::CMD_SET_DEFAULT);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Wait for oscillator to stabilize
        vTaskDelay(pdMS_TO_TICKS(10));

        // Clear FIFO
        status = ClearFifo();
        if (status != NFCStatus::OK) {
            return status;
        }

        // Clear all interrupts
        return ClearInterrupts(0xFF, 0xFF, 0xFF);
    }

    NFCStatus ST25R3911B::GetIdentity(uint8_t& identity)
    {
        return ReadRegister(::ST25R3911B::REG_IC_IDENTITY, identity);
    }

    // ============================================================================
    // Field and Protocol Operations
    // ============================================================================

    NFCStatus ST25R3911B::SetField(NFCField field)
    {
        NFCStatus status;
        
        if (field == NFCField::ON) {
            // Enable oscillator and transmitter
            status = ModifyRegister(::ST25R3911B::REG_OP_CONTROL, 
                                  ::ST25R3911B::OP_CONTROL_EN, 
                                  ::ST25R3911B::OP_CONTROL_EN);
            if (status != NFCStatus::OK) {
                return status;
            }

            // Enable transmitter in mode register
            status = ModifyRegister(::ST25R3911B::REG_MODE, 
                                  ::ST25R3911B::MODE_TR_EN, 
                                  ::ST25R3911B::MODE_TR_EN);
            if (status != NFCStatus::OK) {
                return status;
            }

            // Wait for field to stabilize
            vTaskDelay(pdMS_TO_TICKS(5));
        } else {
            // Disable transmitter
            status = ModifyRegister(::ST25R3911B::REG_MODE, 
                                  ::ST25R3911B::MODE_TR_EN, 
                                  0x00);
            if (status != NFCStatus::OK) {
                return status;
            }

            // Disable oscillator if not needed
            status = ModifyRegister(::ST25R3911B::REG_OP_CONTROL, 
                                  ::ST25R3911B::OP_CONTROL_EN, 
                                  0x00);
        }

        _fieldState = field;
        return status;
    }

    NFCStatus ST25R3911B::GetField(NFCField& field)
    {
        uint8_t regValue;
        NFCStatus status = ReadRegister(::ST25R3911B::REG_MODE, regValue);
        if (status != NFCStatus::OK) {
            return status;
        }

        field = (regValue & ::ST25R3911B::MODE_TR_EN) ? NFCField::ON : NFCField::OFF;
        _fieldState = field;
        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::SetProtocol(NFCProtocol protocol)
    {
        NFCStatus status = configureProtocol(protocol);
        if (status == NFCStatus::OK) {
            _currentProtocol = protocol;
        }
        return status;
    }

    // ============================================================================
    // Low-Level Register Operations
    // ============================================================================

    NFCStatus ST25R3911B::ReadRegister(uint8_t reg, uint8_t& value)
    {
        if (!_initialized || !isValidRegister(reg)) {
            return NFCStatus::INVALID_PARAM;
        }

        std::vector<uint8_t> txData = { static_cast<uint8_t>(reg | ::ST25R3911B::SPI_CMD_READ), 0x00 };
        std::vector<uint8_t> rxData;

        _config.spiMaster->SelectSlave();
        SPI::SPIStatus spiStatus = _config.spiMaster->TransmitReceive(txData, rxData);
        _config.spiMaster->DeselectSlave();

        if (spiStatus != SPI::SPIStatus::OK || rxData.size() < 2) {
            return convertSpiStatus(spiStatus);
        }

        value = rxData[1];
        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::WriteRegister(uint8_t reg, uint8_t value)
    {
        if (!_initialized || !isValidRegister(reg)) {
            return NFCStatus::INVALID_PARAM;
        }

        std::vector<uint8_t> txData = { static_cast<uint8_t>(reg | ::ST25R3911B::SPI_CMD_WRITE), value };

        _config.spiMaster->SelectSlave();
        SPI::SPIStatus spiStatus = _config.spiMaster->Transmit(txData);
        _config.spiMaster->DeselectSlave();

        return convertSpiStatus(spiStatus);
    }

    NFCStatus ST25R3911B::ReadRegisters(uint8_t startReg, std::vector<uint8_t>& data, uint8_t length)
    {
        if (!_initialized || !isValidRegister(startReg) || length == 0) {
            return NFCStatus::INVALID_PARAM;
        }

        data.clear();
        data.reserve(length);

        for (uint8_t i = 0; i < length; ++i) {
            uint8_t value;
            NFCStatus status = ReadRegister(startReg + i, value);
            if (status != NFCStatus::OK) {
                return status;
            }
            data.push_back(value);
        }

        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::WriteRegisters(uint8_t startReg, const std::vector<uint8_t>& data)
    {
        if (!_initialized || !isValidRegister(startReg) || data.empty()) {
            return NFCStatus::INVALID_PARAM;
        }

        for (size_t i = 0; i < data.size(); ++i) {
            NFCStatus status = WriteRegister(startReg + static_cast<uint8_t>(i), data[i]);
            if (status != NFCStatus::OK) {
                return status;
            }
        }

        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::ExecuteCommand(uint8_t cmd)
    {
        if (!_initialized || !isValidCommand(cmd)) {
            return NFCStatus::INVALID_PARAM;
        }

        std::vector<uint8_t> txData = { cmd };

        _config.spiMaster->SelectSlave();
        SPI::SPIStatus spiStatus = _config.spiMaster->Transmit(txData);
        _config.spiMaster->DeselectSlave();

        return convertSpiStatus(spiStatus);
    }

    NFCStatus ST25R3911B::ModifyRegister(uint8_t reg, uint8_t mask, uint8_t value)
    {
        uint8_t regValue;
        NFCStatus status = ReadRegister(reg, regValue);
        if (status != NFCStatus::OK) {
            return status;
        }

        regValue = (regValue & ~mask) | (value & mask);
        return WriteRegister(reg, regValue);
    }

    // ============================================================================
    // FIFO Operations
    // ============================================================================

    NFCStatus ST25R3911B::GetFifoStatus(uint8_t& bytesInFifo, bool& fifoFull)
    {
        uint8_t status1, status2;
        NFCStatus result = ReadRegister(::ST25R3911B::REG_FIFO_RX_STATUS1, status1);
        if (result != NFCStatus::OK) {
            return result;
        }

        result = ReadRegister(::ST25R3911B::REG_FIFO_RX_STATUS2, status2);
        if (result != NFCStatus::OK) {
            return result;
        }

        bytesInFifo = (status2 & 0x80) ? ((status1 & 0x7F) | 0x80) : (status1 & 0x7F);
        fifoFull = (bytesInFifo >= ::ST25R3911B::FIFO_SIZE);

        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::ClearFifo(void)
    {
        return ExecuteCommand(::ST25R3911B::CMD_CLEAR_FIFO);
    }

    NFCStatus ST25R3911B::ReadFifo(std::vector<uint8_t>& data, uint8_t length)
    {
        if (!_initialized || length == 0) {
            return NFCStatus::INVALID_PARAM;
        }

        data.clear();
        data.reserve(length);

        for (uint8_t i = 0; i < length; ++i) {
            uint8_t value;
            NFCStatus status = ReadRegister(::ST25R3911B::REG_FIFO_DATA, value);
            if (status != NFCStatus::OK) {
                return status;
            }
            data.push_back(value);
        }

        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::WriteFifo(const std::vector<uint8_t>& data)
    {
        if (!_initialized || data.empty()) {
            return NFCStatus::INVALID_PARAM;
        }

        for (const uint8_t byte : data) {
            NFCStatus status = WriteRegister(::ST25R3911B::REG_FIFO_LOAD, byte);
            if (status != NFCStatus::OK) {
                return status;
            }
        }

        return NFCStatus::OK;
    }

    // ============================================================================
    // Interrupt Operations
    // ============================================================================

    NFCStatus ST25R3911B::GetInterruptStatus(uint8_t& mainIrq, uint8_t& timerNfcIrq, uint8_t& errorWupIrq)
    {
        NFCStatus status = ReadRegister(::ST25R3911B::REG_IRQ_MAIN, mainIrq);
        if (status != NFCStatus::OK) {
            return status;
        }

        status = ReadRegister(::ST25R3911B::REG_IRQ_TIMER_NFC, timerNfcIrq);
        if (status != NFCStatus::OK) {
            return status;
        }

        return ReadRegister(::ST25R3911B::REG_IRQ_ERROR_WUP, errorWupIrq);
    }

    NFCStatus ST25R3911B::ClearInterrupts(uint8_t mainIrq, uint8_t timerNfcIrq, uint8_t errorWupIrq)
    {
        NFCStatus status = WriteRegister(::ST25R3911B::REG_IRQ_MAIN, mainIrq);
        if (status != NFCStatus::OK) {
            return status;
        }

        status = WriteRegister(::ST25R3911B::REG_IRQ_TIMER_NFC, timerNfcIrq);
        if (status != NFCStatus::OK) {
            return status;
        }

        return WriteRegister(::ST25R3911B::REG_IRQ_ERROR_WUP, errorWupIrq);
    }

    NFCStatus ST25R3911B::SetInterruptMasks(uint8_t mainMask, uint8_t timerNfcMask, uint8_t errorWupMask)
    {
        NFCStatus status = WriteRegister(::ST25R3911B::REG_IRQ_MASK_MAIN, mainMask);
        if (status != NFCStatus::OK) {
            return status;
        }

        status = WriteRegister(::ST25R3911B::REG_IRQ_MASK_TIMER_NFC, timerNfcMask);
        if (status != NFCStatus::OK) {
            return status;
        }

        return WriteRegister(::ST25R3911B::REG_IRQ_MASK_ERROR_WUP, errorWupMask);
    }

    void ST25R3911B::HandleInterrupt(void)
    {
        _interruptPending = true;
        
        // Call user callback if provided
        if (_config.irqCallback) {
            _config.irqCallback();
        }
    }

    // ============================================================================
    // Communication Operations
    // ============================================================================

    NFCStatus ST25R3911B::Transmit(const std::vector<uint8_t>& data, bool crc)
    {
        if (!_initialized || data.empty()) {
            return NFCStatus::INVALID_PARAM;
        }

        // Clear FIFO
        NFCStatus status = ClearFifo();
        if (status != NFCStatus::OK) {
            return status;
        }

        // Write data to FIFO
        status = WriteFifo(data);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Execute transmit command
        uint8_t cmd = crc ? ::ST25R3911B::CMD_TRANSMIT_WITH_CRC : ::ST25R3911B::CMD_TRANSMIT_WITHOUT_CRC;
        return ExecuteCommand(cmd);
    }

    NFCStatus ST25R3911B::Receive(std::vector<uint8_t>& data, uint32_t timeoutMs)
    {
        if (!_initialized) {
            return NFCStatus::NOT_INITIALIZED;
        }

        if (timeoutMs == 0) {
            timeoutMs = _config.timeoutMs;
        }

        // Wait for receive complete interrupt
        NFCStatus status = waitForInterrupt(timeoutMs);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Check interrupt status
        uint8_t mainIrq, timerNfcIrq, errorWupIrq;
        status = GetInterruptStatus(mainIrq, timerNfcIrq, errorWupIrq);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Check for errors
        if (mainIrq & ::ST25R3911B::IRQ_MAIN_COL) {
            return NFCStatus::COLLISION_ERROR;
        }

        // Check for receive complete
        if (mainIrq & ::ST25R3911B::IRQ_MAIN_RXE) {
            // Get FIFO status
            uint8_t bytesInFifo;
            bool fifoFull;
            status = GetFifoStatus(bytesInFifo, fifoFull);
            if (status != NFCStatus::OK) {
                return status;
            }

            // Read data from FIFO
            if (bytesInFifo > 0) {
                status = ReadFifo(data, bytesInFifo);
            }

            // Clear interrupts
            ClearInterrupts(mainIrq, timerNfcIrq, errorWupIrq);
            return status;
        }

        return NFCStatus::TIMEOUT;
    }

    NFCStatus ST25R3911B::TransmitReceive(const std::vector<uint8_t>& txData, std::vector<uint8_t>& rxData, uint32_t timeoutMs)
    {
        NFCStatus status = Transmit(txData, true);
        if (status != NFCStatus::OK) {
            return status;
        }

        return Receive(rxData, timeoutMs);
    }

    // ============================================================================
    // Private Helper Functions
    // ============================================================================

    NFCStatus ST25R3911B::configureDefaults(void)
    {
        NFCStatus status;

        // Set default operation control
        status = WriteRegister(::ST25R3911B::REG_OP_CONTROL, 
                             ::ST25R3911B::OP_CONTROL_RX_EN | 
                             ::ST25R3911B::OP_CONTROL_RX_MAN | 
                             ::ST25R3911B::OP_CONTROL_TX_CRC);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Set default interrupt masks (enable main interrupts)
        status = SetInterruptMasks(
            ::ST25R3911B::IRQ_MAIN_RXS | ::ST25R3911B::IRQ_MAIN_RXE | 
            ::ST25R3911B::IRQ_MAIN_TXE | ::ST25R3911B::IRQ_MAIN_COL,
            0x00, 0x00);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Configure FIFO water level
        status = WriteRegister(::ST25R3911B::REG_IO_CONF1, ::ST25R3911B::FIFO_WATER_LEVEL);
        
        return status;
    }

    NFCStatus ST25R3911B::configureProtocol(NFCProtocol protocol)
    {
        NFCStatus status;
        uint8_t modeValue = 0;

        switch (protocol) {
            case NFCProtocol::NFC_A:
                modeValue = ::ST25R3911B::MODE_OM_ISO14443A;
                // Configure ISO14443A specific settings
                status = WriteRegister(::ST25R3911B::REG_ISO14443A_NFC, 0x88);
                break;

            case NFCProtocol::NFC_B:
                modeValue = ::ST25R3911B::MODE_OM_ISO14443B;
                // Configure ISO14443B specific settings
                status = WriteRegister(::ST25R3911B::REG_ISO14443B, 0x00);
                break;

            case NFCProtocol::NFC_F:
                modeValue = ::ST25R3911B::MODE_OM_FELICA;
                // Configure FeliCa specific settings
                status = WriteRegister(::ST25R3911B::REG_BIT_RATE, 0x00);
                break;

            case NFCProtocol::NFC_V:
                // ISO15693 uses subcarrier mode
                modeValue = ::ST25R3911B::MODE_OM_SUBCARRIER;
                status = WriteRegister(::ST25R3911B::REG_STREAM_MODE, 0x00);
                break;

            case NFCProtocol::NFC_P2P:
                modeValue = ::ST25R3911B::MODE_OM_NFC;
                status = WriteRegister(::ST25R3911B::REG_P2P_RX_CONF, 0x00);
                break;

            case NFCProtocol::MIFARE_CLASSIC:
                modeValue = ::ST25R3911B::MODE_OM_ISO14443A;
                // MIFARE Classic uses ISO14443A with specific settings
                status = WriteRegister(::ST25R3911B::REG_ISO14443A_NFC, 0x88);
                break;

            default:
                return NFCStatus::INVALID_PARAM;
        }

        if (status != NFCStatus::OK) {
            return status;
        }

        // Set mode register
        return ModifyRegister(::ST25R3911B::REG_MODE, ::ST25R3911B::MODE_OM_MASK, modeValue);
    }

    NFCStatus ST25R3911B::waitForInterrupt(uint32_t timeoutMs)
    {
        uint32_t startTime = xTaskGetTickCount();
        uint32_t timeoutTicks = pdMS_TO_TICKS(timeoutMs);

        while (!_interruptPending) {
            if ((xTaskGetTickCount() - startTime) >= timeoutTicks) {
                return NFCStatus::TIMEOUT;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        _interruptPending = false;
        return NFCStatus::OK;
    }

    NFCStatus ST25R3911B::convertSpiStatus(SPI::SPIStatus spiStatus)
    {
        switch (spiStatus) {
            case SPI::SPIStatus::OK:
                return NFCStatus::OK;
            case SPI::SPIStatus::TIMEOUT:
                return NFCStatus::TIMEOUT;
            case SPI::SPIStatus::INVALID_PARAM:
                return NFCStatus::INVALID_PARAM;
            default:
                return NFCStatus::COMMUNICATION_ERROR;
        }
    }

    bool ST25R3911B::isValidRegister(uint8_t reg)
    {
        // Check if register address is in valid range
        return (reg <= ::ST25R3911B::REG_FIFO_DATA);
    }

    bool ST25R3911B::isValidCommand(uint8_t cmd)
    {
        // Check if command is in direct command range
        return (cmd >= 0xC0);
    }

} // namespace NFC