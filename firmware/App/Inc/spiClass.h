/**
 * @file    App/Inc/spiClass.h
 * @brief   SPI header file.
 * @details This file contains the declarations for the SPI initialization and control functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_SPI_CLASS_H
#define INC_SPI_CLASS_H

/**
 * @include necessary headers
 */
#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"
#include <vector>
#include <cstdint>

/**
 * @namespace SPI
 * @brief Contains SPI related functions and definitions.
 */
namespace SPI
{
    /**
     * @enum SPIMode
     * @brief Defines the SPI communication mode.
     */
    enum class SPIMode
    {
        MODE_0 = 0, /**< CPOL=0, CPHA=0 */
        MODE_1 = 1, /**< CPOL=0, CPHA=1 */
        MODE_2 = 2, /**< CPOL=1, CPHA=0 */
        MODE_3 = 3  /**< CPOL=1, CPHA=1 */
    };

    /**
     * @enum SPIDataSize
     * @brief Defines the SPI data frame size.
     */
    enum class SPIDataSize
    {
        SIZE_8BIT = LL_SPI_DATAWIDTH_8BIT,
        SIZE_16BIT = LL_SPI_DATAWIDTH_16BIT
    };

    /**
     * @enum SPIBitOrder
     * @brief Defines the bit order for SPI transmission.
     */
    enum class SPIBitOrder
    {
        MSB_FIRST = LL_SPI_MSB_FIRST,
        LSB_FIRST = LL_SPI_LSB_FIRST
    };

    /**
     * @enum SPISpeed
     * @brief Defines the SPI clock speed prescaler.
     */
    enum class SPISpeed
    {
        PRESCALER_2 = LL_SPI_BAUDRATEPRESCALER_DIV2,
        PRESCALER_4 = LL_SPI_BAUDRATEPRESCALER_DIV4,
        PRESCALER_8 = LL_SPI_BAUDRATEPRESCALER_DIV8,
        PRESCALER_16 = LL_SPI_BAUDRATEPRESCALER_DIV16,
        PRESCALER_32 = LL_SPI_BAUDRATEPRESCALER_DIV32,
        PRESCALER_64 = LL_SPI_BAUDRATEPRESCALER_DIV64,
        PRESCALER_128 = LL_SPI_BAUDRATEPRESCALER_DIV128,
        PRESCALER_256 = LL_SPI_BAUDRATEPRESCALER_DIV256
    };

    /**
     * @enum SPIStatus
     * @brief Return status codes for SPI operations.
     */
    enum class SPIStatus
    {
        OK = 0,
        ERROR,
        BUSY,
        TIMEOUT,
        INVALID_PARAM
    };

    /**
     * @struct SPIConfig
     * @brief Configuration structure for SPI interface.
     */
    struct SPIConfig
    {
        SPI_TypeDef *instance;      /**< SPI instance (e.g., SPI1, SPI2) */
        SPIMode mode;               /**< SPI communication mode */
        SPIDataSize dataSize;       /**< Data frame size */
        SPIBitOrder bitOrder;       /**< Bit transmission order */
        SPISpeed speed;             /**< Clock speed prescaler */
        
        // GPIO Configuration for SPI pins
        GPIO_TypeDef *sckPort;      /**< SCK pin port */
        uint32_t sckPin;            /**< SCK pin number */
        uint32_t sckAlternate;      /**< SCK alternate function */
        
        GPIO_TypeDef *misoPort;     /**< MISO pin port */
        uint32_t misoPin;           /**< MISO pin number */
        uint32_t misoAlternate;     /**< MISO alternate function */
        
        GPIO_TypeDef *mosiPort;     /**< MOSI pin port */
        uint32_t mosiPin;           /**< MOSI pin number */
        uint32_t mosiAlternate;     /**< MOSI alternate function */
        
        GPIO_TypeDef *csPort;       /**< Chip Select pin port */
        uint32_t csPin;             /**< Chip Select pin number */
        
        uint32_t timeoutMs;         /**< Timeout for operations in milliseconds */
    };

    /**
     * @class SPIBase
     * @brief Base class for SPI operations.
     */
    class SPIBase
    {
        protected:
            SPIConfig _config;          /**< SPI configuration */
            bool _initialized;          /**< Initialization status */

        public:
            /**
             * @brief Constructor to initialize SPI with given configuration.
             * @param config Configuration structure for the SPI interface.
             */
            SPIBase(const SPIConfig &config) : _config(config), _initialized(false) {}

            /**
             * @brief Destructor.
             */
            virtual ~SPIBase() = default;

            /**
             * @brief Initialize SPI interface
             * @param config SPI configuration structure
             * @return SPIStatus indicating success or failure
             */
            SPIStatus Init(const SPIConfig &config);

            /**
             * @brief Deinitialize SPI interface
             * @return SPIStatus indicating success or failure
             */
            SPIStatus DeInit(void);

            /**
             * @brief Check if SPI is initialized
             * @return true if initialized, false otherwise
             */
            bool IsInitialized(void) const { return _initialized; }

            /**
             * @brief Get current SPI configuration
             * @return Reference to current configuration
             */
            const SPIConfig& GetConfig(void) const { return _config; }

        protected:
            /**
             * @brief Enable clock for SPI instance
             * @param instance SPI instance to enable clock for
             */
            void enableClock(SPI_TypeDef *instance);

            /**
             * @brief Configure GPIO pins for SPI
             * @param config SPI configuration containing GPIO settings
             */
            void configureGPIO(const SPIConfig &config);

            /**
             * @brief Configure SPI peripheral
             * @param config SPI configuration
             */
            void configureSPI(const SPIConfig &config);

            /**
             * @brief Enable GPIO port clock
             * @param port GPIO port to enable clock for
             */
            void enableGPIOClock(GPIO_TypeDef *port);
    };

    /**
     * @class SPIMaster
     * @brief Class for SPI Master operations.
     */
    class SPIMaster : public SPIBase
    {
        public:
            /**
             * @brief Constructor to initialize SPI Master.
             * @param config Configuration structure for the SPI interface.
             */
            SPIMaster(const SPIConfig &config) : SPIBase(config) 
            {
                Init(config);
            }

            /**
             * @brief Destructor.
             */
            ~SPIMaster() override
            {
                DeInit();
            }

            /**
             * @brief Select slave device (assert CS)
             */
            void SelectSlave(void);

            /**
             * @brief Deselect slave device (deassert CS)
             */
            void DeselectSlave(void);

            /**
             * @brief Transmit data over SPI
             * @param data Vector containing data to transmit
             * @return SPIStatus indicating success or failure
             */
            SPIStatus Transmit(const std::vector<uint8_t> &data);

            /**
             * @brief Receive data over SPI
             * @param data Vector to store received data
             * @param size Number of bytes to receive
             * @return SPIStatus indicating success or failure
             */
            SPIStatus Receive(std::vector<uint8_t> &data, size_t size);

            /**
             * @brief Transmit and receive data simultaneously
             * @param txData Vector containing data to transmit
             * @param rxData Vector to store received data
             * @return SPIStatus indicating success or failure
             */
            SPIStatus TransmitReceive(const std::vector<uint8_t> &txData, std::vector<uint8_t> &rxData);

            /**
             * @brief Transmit single byte
             * @param data Byte to transmit
             * @return SPIStatus indicating success or failure
             */
            SPIStatus TransmitByte(uint8_t data);

            /**
             * @brief Receive single byte
             * @param data Reference to store received byte
             * @return SPIStatus indicating success or failure
             */
            SPIStatus ReceiveByte(uint8_t &data);

            /**
             * @brief Transmit and receive single byte
             * @param txData Byte to transmit
             * @param rxData Reference to store received byte
             * @return SPIStatus indicating success or failure
             */
            SPIStatus TransmitReceiveByte(uint8_t txData, uint8_t &rxData);

        private:
            /**
             * @brief Wait for SPI operation to complete
             * @param timeoutMs Timeout in milliseconds
             * @return SPIStatus indicating success or timeout
             */
            SPIStatus waitForCompletion(uint32_t timeoutMs);

            /**
             * @brief Check if SPI is busy
             * @return true if busy, false if ready
             */
            bool isBusy(void);
    };

} // namespace SPI

#endif /* INC_SPI_CLASS_H */