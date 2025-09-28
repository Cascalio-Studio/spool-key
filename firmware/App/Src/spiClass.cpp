/**
 * @file    App/Src/spiClass.cpp
 * @brief   SPI implementation file.
 * @details This file contains the implementation of the SPI initialization and control functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

/**
 * @include necessary headers
 */
#include "spiClass.h"
#include "stm32l4xx_ll_utils.h"

namespace SPI
{
    // ============================================================================
    // SPIBase Implementation
    // ============================================================================

    SPIStatus SPIBase::Init(const SPIConfig &config)
    {
        if (_initialized) {
            return SPIStatus::ERROR;
        }

        _config = config;

        // Enable clocks
        enableClock(_config.instance);
        
        // Configure GPIO pins
        configureGPIO(_config);
        
        // Configure SPI peripheral
        configureSPI(_config);
        
        // Enable SPI
        LL_SPI_Enable(_config.instance);
        
        _initialized = true;
        return SPIStatus::OK;
    }

    SPIStatus SPIBase::DeInit(void)
    {
        if (!_initialized) {
            return SPIStatus::ERROR;
        }

        // Disable SPI
        LL_SPI_Disable(_config.instance);
        
        // Reset SPI peripheral
        if (_config.instance == SPI1) {
            LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
            LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
        } else if (_config.instance == SPI2) {
            LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
            LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI2);
        } else if (_config.instance == SPI3) {
            LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI3);
            LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI3);
        }

        _initialized = false;
        return SPIStatus::OK;
    }

    void SPIBase::enableClock(SPI_TypeDef *instance)
    {
        if (instance == SPI1) {
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
        } else if (instance == SPI2) {
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
        } else if (instance == SPI3) {
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
        }
    }

    void SPIBase::enableGPIOClock(GPIO_TypeDef *port)
    {
        if (port == GPIOA) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
        } else if (port == GPIOB) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
        } else if (port == GPIOC) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
        } else if (port == GPIOD) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
        } else if (port == GPIOE) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
        } else if (port == GPIOH) {
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
        }
    }

    void SPIBase::configureGPIO(const SPIConfig &config)
    {
        // Enable GPIO clocks
        enableGPIOClock(config.sckPort);
        enableGPIOClock(config.misoPort);
        enableGPIOClock(config.mosiPort);
        enableGPIOClock(config.csPort);

        // Configure SCK pin
        LL_GPIO_SetPinMode(config.sckPort, config.sckPin, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetPinSpeed(config.sckPort, config.sckPin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(config.sckPort, config.sckPin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(config.sckPort, config.sckPin, LL_GPIO_PULL_NO);
        
        if (config.sckPin <= LL_GPIO_PIN_7) {
            LL_GPIO_SetAFPin_0_7(config.sckPort, config.sckPin, config.sckAlternate);
        } else {
            LL_GPIO_SetAFPin_8_15(config.sckPort, config.sckPin, config.sckAlternate);
        }

        // Configure MISO pin
        LL_GPIO_SetPinMode(config.misoPort, config.misoPin, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetPinSpeed(config.misoPort, config.misoPin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(config.misoPort, config.misoPin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(config.misoPort, config.misoPin, LL_GPIO_PULL_NO);
        
        if (config.misoPin <= LL_GPIO_PIN_7) {
            LL_GPIO_SetAFPin_0_7(config.misoPort, config.misoPin, config.misoAlternate);
        } else {
            LL_GPIO_SetAFPin_8_15(config.misoPort, config.misoPin, config.misoAlternate);
        }

        // Configure MOSI pin
        LL_GPIO_SetPinMode(config.mosiPort, config.mosiPin, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetPinSpeed(config.mosiPort, config.mosiPin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(config.mosiPort, config.mosiPin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(config.mosiPort, config.mosiPin, LL_GPIO_PULL_NO);
        
        if (config.mosiPin <= LL_GPIO_PIN_7) {
            LL_GPIO_SetAFPin_0_7(config.mosiPort, config.mosiPin, config.mosiAlternate);
        } else {
            LL_GPIO_SetAFPin_8_15(config.mosiPort, config.mosiPin, config.mosiAlternate);
        }

        // Configure CS pin as GPIO output (manual control)
        LL_GPIO_SetPinMode(config.csPort, config.csPin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinSpeed(config.csPort, config.csPin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(config.csPort, config.csPin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(config.csPort, config.csPin, LL_GPIO_PULL_NO);
        
        // Set CS high initially (deselected)
        LL_GPIO_SetOutputPin(config.csPort, config.csPin);
    }

    void SPIBase::configureSPI(const SPIConfig &config)
    {
        // Set master mode
        LL_SPI_SetMode(config.instance, LL_SPI_MODE_MASTER);
        
        // Set data width
        LL_SPI_SetDataWidth(config.instance, static_cast<uint32_t>(config.dataSize));
        
        // Set bit order
        LL_SPI_SetTransferBitOrder(config.instance, static_cast<uint32_t>(config.bitOrder));
        
        // Set baud rate
        LL_SPI_SetBaudRatePrescaler(config.instance, static_cast<uint32_t>(config.speed));
        
        // Set SPI mode (CPOL and CPHA)
        switch (config.mode) {
            case SPIMode::MODE_0:
                LL_SPI_SetClockPolarity(config.instance, LL_SPI_POLARITY_LOW);
                LL_SPI_SetClockPhase(config.instance, LL_SPI_PHASE_1EDGE);
                break;
            case SPIMode::MODE_1:
                LL_SPI_SetClockPolarity(config.instance, LL_SPI_POLARITY_LOW);
                LL_SPI_SetClockPhase(config.instance, LL_SPI_PHASE_2EDGE);
                break;
            case SPIMode::MODE_2:
                LL_SPI_SetClockPolarity(config.instance, LL_SPI_POLARITY_HIGH);
                LL_SPI_SetClockPhase(config.instance, LL_SPI_PHASE_1EDGE);
                break;
            case SPIMode::MODE_3:
                LL_SPI_SetClockPolarity(config.instance, LL_SPI_POLARITY_HIGH);
                LL_SPI_SetClockPhase(config.instance, LL_SPI_PHASE_2EDGE);
                break;
        }
        
        // Set NSS management
        LL_SPI_SetNSSMode(config.instance, LL_SPI_NSS_SOFT);
        
        // Set transfer direction
        LL_SPI_SetTransferDirection(config.instance, LL_SPI_FULL_DUPLEX);
    }

    // ============================================================================
    // SPIMaster Implementation
    // ============================================================================

    void SPIMaster::SelectSlave(void)
    {
        if (_initialized) {
            LL_GPIO_ResetOutputPin(_config.csPort, _config.csPin);
        }
    }

    void SPIMaster::DeselectSlave(void)
    {
        if (_initialized) {
            LL_GPIO_SetOutputPin(_config.csPort, _config.csPin);
        }
    }

    SPIStatus SPIMaster::Transmit(const std::vector<uint8_t> &data)
    {
        if (!_initialized || data.empty()) {
            return SPIStatus::INVALID_PARAM;
        }

        for (const uint8_t byte : data) {
            SPIStatus status = TransmitByte(byte);
            if (status != SPIStatus::OK) {
                return status;
            }
        }
        
        return SPIStatus::OK;
    }

    SPIStatus SPIMaster::Receive(std::vector<uint8_t> &data, size_t size)
    {
        if (!_initialized || size == 0) {
            return SPIStatus::INVALID_PARAM;
        }

        data.clear();
        data.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            uint8_t receivedByte;
            SPIStatus status = ReceiveByte(receivedByte);
            if (status != SPIStatus::OK) {
                return status;
            }
            data.push_back(receivedByte);
        }
        
        return SPIStatus::OK;
    }

    SPIStatus SPIMaster::TransmitReceive(const std::vector<uint8_t> &txData, std::vector<uint8_t> &rxData)
    {
        if (!_initialized || txData.empty()) {
            return SPIStatus::INVALID_PARAM;
        }

        rxData.clear();
        rxData.reserve(txData.size());

        for (const uint8_t txByte : txData) {
            uint8_t rxByte;
            SPIStatus status = TransmitReceiveByte(txByte, rxByte);
            if (status != SPIStatus::OK) {
                return status;
            }
            rxData.push_back(rxByte);
        }
        
        return SPIStatus::OK;
    }

    SPIStatus SPIMaster::TransmitByte(uint8_t data)
    {
        if (!_initialized) {
            return SPIStatus::ERROR;
        }

        // Wait for TXE (Transmit buffer empty)
        uint32_t timeout = _config.timeoutMs;
        while (!LL_SPI_IsActiveFlag_TXE(_config.instance)) {
            if (timeout-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }

        // Send data
        LL_SPI_TransmitData8(_config.instance, data);

        // Wait for transmission complete
        timeout = _config.timeoutMs;
        while (LL_SPI_IsActiveFlag_BSY(_config.instance)) {
            if (timeout-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }

        // Clear RXNE flag by reading DR
        if (LL_SPI_IsActiveFlag_RXNE(_config.instance)) {
            LL_SPI_ReceiveData8(_config.instance);
        }

        return SPIStatus::OK;
    }

    SPIStatus SPIMaster::ReceiveByte(uint8_t &data)
    {
        if (!_initialized) {
            return SPIStatus::ERROR;
        }

        // Send dummy byte to generate clock
        SPIStatus status = TransmitReceiveByte(0xFF, data);
        return status;
    }

    SPIStatus SPIMaster::TransmitReceiveByte(uint8_t txData, uint8_t &rxData)
    {
        if (!_initialized) {
            return SPIStatus::ERROR;
        }

        // Wait for TXE (Transmit buffer empty)
        uint32_t timeout = _config.timeoutMs;
        while (!LL_SPI_IsActiveFlag_TXE(_config.instance)) {
            if (timeout-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }

        // Send data
        LL_SPI_TransmitData8(_config.instance, txData);

        // Wait for RXNE (Receive buffer not empty)
        timeout = _config.timeoutMs;
        while (!LL_SPI_IsActiveFlag_RXNE(_config.instance)) {
            if (timeout-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }

        // Read received data
        rxData = LL_SPI_ReceiveData8(_config.instance);

        // Wait for transmission complete
        timeout = _config.timeoutMs;
        while (LL_SPI_IsActiveFlag_BSY(_config.instance)) {
            if (timeout-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }

        return SPIStatus::OK;
    }

    SPIStatus SPIMaster::waitForCompletion(uint32_t timeoutMs)
    {
        while (isBusy()) {
            if (timeoutMs-- == 0) {
                return SPIStatus::TIMEOUT;
            }
            LL_mDelay(1);
        }
        return SPIStatus::OK;
    }

    bool SPIMaster::isBusy(void)
    {
        return LL_SPI_IsActiveFlag_BSY(_config.instance);
    }

} // namespace SPI
