/**
 * @file    App/Inc/st25r3911b.h
 * @brief   ST25R3911B NFC Controller Class Header
 * @details This file contains the ST25R3911B NFC controller class declarations for low-level register operations.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_ST25R3911B_H
#define INC_ST25R3911B_H

/**
 * @include necessary headers
 */
#include "spiClass.h"
#include "gpioClass.h"
#include "st25r3911b_registers.h"
#include <vector>
#include <cstdint>
#include <functional>

/**
 * @namespace NFC
 * @brief Contains NFC related functions and definitions.
 */
namespace NFC
{
    /**
     * @enum NFCStatus
     * @brief Return status codes for NFC operations.
     */
    enum class NFCStatus
    {
        OK = 0,                 /**< Operation successful */
        ERROR,                  /**< General error */
        TIMEOUT,                /**< Operation timed out */
        INVALID_PARAM,          /**< Invalid parameter */
        NOT_INITIALIZED,        /**< Controller not initialized */
        FIFO_OVERFLOW,          /**< FIFO overflow error */
        FIFO_UNDERFLOW,         /**< FIFO underflow error */
        CRC_ERROR,              /**< CRC error */
        COLLISION_ERROR,        /**< Collision detected */
        NO_TAG_FOUND,           /**< No NFC tag found */
        UNSUPPORTED_TAG,        /**< Unsupported tag type */
        COMMUNICATION_ERROR     /**< Communication error */
    };

    /**
     * @enum NFCProtocol
     * @brief Supported NFC protocols.
     */
    enum class NFCProtocol
    {
        NFC_A = 0,             /**< ISO14443 Type A */
        NFC_B,                 /**< ISO14443 Type B */
        NFC_F,                 /**< FeliCa */
        NFC_V,                 /**< ISO15693 */
        NFC_P2P,               /**< NFC Peer-to-Peer (ISO18092) */
        MIFARE_CLASSIC         /**< MIFARE Classic */
    };

    /**
     * @enum NFCField
     * @brief NFC field state.
     */
    enum class NFCField
    {
        OFF = 0,               /**< RF field off */
        ON                     /**< RF field on */
    };

    /**
     * @struct NFCConfig
     * @brief Configuration structure for NFC controller.
     */
    struct NFCConfig
    {
        SPI::SPIMaster* spiMaster;          /**< SPI master interface */
        GPIO::GPIOInterrupt* irqPin;        /**< Interrupt pin interface */
        NFCProtocol defaultProtocol;        /**< Default protocol to use */
        uint32_t timeoutMs;                 /**< Default timeout in milliseconds */
        std::function<void(void)> irqCallback; /**< Interrupt callback function */
    };

    /**
     * @struct TagInfo
     * @brief Information about detected NFC tag.
     */
    struct TagInfo
    {
        NFCProtocol protocol;               /**< Detected protocol */
        std::vector<uint8_t> uid;           /**< Tag UID */
        uint8_t sak;                        /**< SAK byte (for Type A tags) */
        std::vector<uint8_t> atqa;          /**< ATQA bytes (for Type A tags) */
        std::vector<uint8_t> pupi;          /**< PUPI (for Type B tags) */
        std::vector<uint8_t> appData;       /**< Application data */
        uint16_t dataSize;                  /**< Available data size */
        bool isReadOnly;                    /**< Read-only flag */
    };

    /**
     * @class ST25R3911B
     * @brief ST25R3911B NFC Controller Class
     */
    class ST25R3911B
    {
        public:
            /**
             * @brief Constructor
             * @param config NFC controller configuration
             */
            ST25R3911B(const NFCConfig& config);

            /**
             * @brief Destructor
             */
            ~ST25R3911B();

            /**
             * @brief Initialize the NFC controller
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Initialize(void);

            /**
             * @brief Deinitialize the NFC controller
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Deinitialize(void);

            /**
             * @brief Check if controller is initialized
             * @return true if initialized, false otherwise
             */
            bool IsInitialized(void) const { return _initialized; }

            /**
             * @brief Reset the NFC controller
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Reset(void);

            /**
             * @brief Get IC identity
             * @param identity Reference to store identity value
             * @return NFCStatus indicating success or failure
             */
            NFCStatus GetIdentity(uint8_t& identity);

            /**
             * @brief Set RF field state
             * @param field Field state (ON/OFF)
             * @return NFCStatus indicating success or failure
             */
            NFCStatus SetField(NFCField field);

            /**
             * @brief Get RF field state
             * @param field Reference to store field state
             * @return NFCStatus indicating success or failure
             */
            NFCStatus GetField(NFCField& field);

            /**
             * @brief Set NFC protocol mode
             * @param protocol Protocol to set
             * @return NFCStatus indicating success or failure
             */
            NFCStatus SetProtocol(NFCProtocol protocol);

            /**
             * @brief Get current protocol
             * @return Current NFC protocol
             */
            NFCProtocol GetProtocol(void) const { return _currentProtocol; }

            // ============================================================================
            // Low-Level Register Operations
            // ============================================================================

            /**
             * @brief Read single register
             * @param reg Register address
             * @param value Reference to store read value
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadRegister(uint8_t reg, uint8_t& value);

            /**
             * @brief Write single register
             * @param reg Register address
             * @param value Value to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteRegister(uint8_t reg, uint8_t value);

            /**
             * @brief Read multiple registers
             * @param startReg Starting register address
             * @param data Vector to store read data
             * @param length Number of registers to read
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadRegisters(uint8_t startReg, std::vector<uint8_t>& data, uint8_t length);

            /**
             * @brief Write multiple registers
             * @param startReg Starting register address
             * @param data Data to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteRegisters(uint8_t startReg, const std::vector<uint8_t>& data);

            /**
             * @brief Execute direct command
             * @param cmd Command to execute
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ExecuteCommand(uint8_t cmd);

            /**
             * @brief Modify register bits
             * @param reg Register address
             * @param mask Bit mask
             * @param value New bit values
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ModifyRegister(uint8_t reg, uint8_t mask, uint8_t value);

            // ============================================================================
            // FIFO Operations
            // ============================================================================

            /**
             * @brief Get FIFO status
             * @param bytesInFifo Reference to store number of bytes in FIFO
             * @param fifoFull Reference to store FIFO full flag
             * @return NFCStatus indicating success or failure
             */
            NFCStatus GetFifoStatus(uint8_t& bytesInFifo, bool& fifoFull);

            /**
             * @brief Clear FIFO
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ClearFifo(void);

            /**
             * @brief Read data from FIFO
             * @param data Vector to store read data
             * @param length Number of bytes to read
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadFifo(std::vector<uint8_t>& data, uint8_t length);

            /**
             * @brief Write data to FIFO
             * @param data Data to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteFifo(const std::vector<uint8_t>& data);

            // ============================================================================
            // Interrupt Operations
            // ============================================================================

            /**
             * @brief Get interrupt status
             * @param mainIrq Reference to store main interrupt flags
             * @param timerNfcIrq Reference to store timer/NFC interrupt flags
             * @param errorWupIrq Reference to store error/wakeup interrupt flags
             * @return NFCStatus indicating success or failure
             */
            NFCStatus GetInterruptStatus(uint8_t& mainIrq, uint8_t& timerNfcIrq, uint8_t& errorWupIrq);

            /**
             * @brief Clear interrupt flags
             * @param mainIrq Main interrupt flags to clear
             * @param timerNfcIrq Timer/NFC interrupt flags to clear
             * @param errorWupIrq Error/wakeup interrupt flags to clear
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ClearInterrupts(uint8_t mainIrq, uint8_t timerNfcIrq, uint8_t errorWupIrq);

            /**
             * @brief Set interrupt masks
             * @param mainMask Main interrupt mask
             * @param timerNfcMask Timer/NFC interrupt mask
             * @param errorWupMask Error/wakeup interrupt mask
             * @return NFCStatus indicating success or failure
             */
            NFCStatus SetInterruptMasks(uint8_t mainMask, uint8_t timerNfcMask, uint8_t errorWupMask);

            /**
             * @brief Handle interrupt (called from ISR)
             */
            void HandleInterrupt(void);

            // ============================================================================
            // Communication Operations
            // ============================================================================

            /**
             * @brief Transmit data
             * @param data Data to transmit
             * @param crc Enable CRC calculation
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Transmit(const std::vector<uint8_t>& data, bool crc = true);

            /**
             * @brief Receive data
             * @param data Vector to store received data
             * @param timeoutMs Timeout in milliseconds
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Receive(std::vector<uint8_t>& data, uint32_t timeoutMs = 0);

            /**
             * @brief Transmit and receive data
             * @param txData Data to transmit
             * @param rxData Vector to store received data
             * @param timeoutMs Timeout in milliseconds
             * @return NFCStatus indicating success or failure
             */
            NFCStatus TransmitReceive(const std::vector<uint8_t>& txData, std::vector<uint8_t>& rxData, uint32_t timeoutMs = 0);

        private:
            NFCConfig _config;                  /**< Controller configuration */
            bool _initialized;                  /**< Initialization status */
            NFCProtocol _currentProtocol;       /**< Current protocol */
            NFCField _fieldState;               /**< Current field state */
            bool _interruptPending;             /**< Interrupt pending flag */

            /**
             * @brief Configure default registers
             * @return NFCStatus indicating success or failure
             */
            NFCStatus configureDefaults(void);

            /**
             * @brief Configure protocol-specific settings
             * @param protocol Protocol to configure
             * @return NFCStatus indicating success or failure
             */
            NFCStatus configureProtocol(NFCProtocol protocol);

            /**
             * @brief Wait for interrupt or timeout
             * @param timeoutMs Timeout in milliseconds
             * @return NFCStatus indicating success or timeout
             */
            NFCStatus waitForInterrupt(uint32_t timeoutMs);

            /**
             * @brief Convert SPI status to NFC status
             * @param spiStatus SPI status to convert
             * @return Corresponding NFC status
             */
            NFCStatus convertSpiStatus(SPI::SPIStatus spiStatus);

            /**
             * @brief Check if register address is valid
             * @param reg Register address
             * @return true if valid, false otherwise
             */
            bool isValidRegister(uint8_t reg);

            /**
             * @brief Check if direct command is valid
             * @param cmd Command to check
             * @return true if valid, false otherwise
             */
            bool isValidCommand(uint8_t cmd);
    };

} // namespace NFC

#endif /* INC_ST25R3911B_H */