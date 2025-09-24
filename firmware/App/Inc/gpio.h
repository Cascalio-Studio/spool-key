/**
 * @file    App/Inc/gpio.h
 * @brief   GPIO header file.
 * @details This file contains the declarations for the GPIO initialization and control functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_GPIO_H
#define INC_GPIO_H

/**
 * @include necessary headers
 */
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_system.h"

/**
 * @namespace GPIO
 * @brief Contains GPIO related functions and definitions.
 */
namespace GPIO
{
    /**
     * @enum PinMode
     * @brief Defines the mode of a GPIO pin.
     */
    enum class PinMode
    {
        INPUT = LL_GPIO_MODE_INPUT,
        OUTPUT = LL_GPIO_MODE_OUTPUT,
        ALTERNATE = LL_GPIO_MODE_ALTERNATE,
        ANALOG = LL_GPIO_MODE_ANALOG
    };

    /**
     * @enum PinPull
     * @brief Defines the pull-up/pull-down configuration of a GPIO pin.
     */
    enum class PinPull
    {
        NOPULL = LL_GPIO_PULL_NO,
        PULLUP = LL_GPIO_PULL_UP,
        PULLDOWN = LL_GPIO_PULL_DOWN
    };

    /**
     * @enum PinSpeed
     * @brief Defines the speed of a GPIO pin.
     */
    enum class PinSpeed
    {
        LOW = LL_GPIO_SPEED_FREQ_LOW,
        MEDIUM = LL_GPIO_SPEED_FREQ_MEDIUM,
        HIGH = LL_GPIO_SPEED_FREQ_HIGH,
        VERY_HIGH = LL_GPIO_SPEED_FREQ_VERY_HIGH
    };

    /**
     * @enum PinOutputType
     * @brief Defines the output type of a GPIO pin.
     */
    enum class PinOutputType
    {
        PUSHPULL = LL_GPIO_OUTPUT_PUSHPULL,
        OPENDRAIN = LL_GPIO_OUTPUT_OPENDRAIN
    };

    /**
     * @enum ExtiTrigger
     * @brief Defines the trigger type for external interrupts.
     */
    enum class ExtiTrigger
    {
        RISING = LL_EXTI_TRIGGER_RISING,
        FALLING = LL_EXTI_TRIGGER_FALLING,
        RISING_FALLING = LL_EXTI_TRIGGER_RISING_FALLING
    };

    /**
     * @struct PinConfig
     * @brief Configuration structure for a GPIO pin.
     */
    struct PinConfig
    {
        GPIO_TypeDef *port;       /**< GPIO port (e.g., GPIOA, GPIOB) */
        uint32_t pin;             /**< GPIO pin number (e.g., LL_GPIO_PIN_0) */
        PinMode mode;             /**< Pin mode (input, output, etc.) */
        PinPull pull;             /**< Pull-up/pull-down configuration */
        PinSpeed speed;           /**< Pin speed */
        PinOutputType outputType; /**< Output type (push-pull, open-drain) */
        uint32_t alternate;        /**< Alternate function (if mode is ALTERNATE) */
        ExtiTrigger extiTrigger; /**< External interrupt trigger type (if applicable) */
        uint32_t extiLine;       /**< EXTI line number (if applicable) */
    }; 

    /**
     * @class GPIOBase
     * @brief Base class for GPIO operations.
     */
    class GPIOBase
    {
        protected: 
            PinConfig _config;  /**< Pin configuration */

        public:
            /**
             * @brief Constructor to initialize GPIO with given configuration.
             * @param config Configuration structure for the GPIO pin.
             */
            GPIOBase(const PinConfig &config) : _config(config) {}

            /**
             * @brief Destructor.
             */
            virtual ~GPIOBase() = default;

            void Init( const PinConfig &config );
            void DeInit( void );
            void Write( bool state );
            bool Read( void );
            void Toggle( void );


        private:
            void enableClock( GPIO_TypeDef *port );
            void configurePin( const PinConfig &config );
    }; // class GPIOBase

    class GPIOOutput : public GPIOBase
    {
        public:
            GPIOOutput( const PinConfig &config ) : GPIOBase( config ) 
            {
                Init( config );
            }

            ~GPIOOutput() override
            {
                DeInit();
            }

            using GPIOBase::Write;
            using GPIOBase::Toggle;
    }; // class GPIOOutput

    class GPIOInput : public GPIOBase
    {
        public:
            GPIOInput( const PinConfig &config ) : GPIOBase( config ) 
            {
                Init( config );
            }

            ~GPIOInput() override
            {
                DeInit();
            }

            using GPIOBase::Read;
    }; // class GPIOInput

    class GPIOInterrupt : public GPIOBase
    {
        public:
            /**
             * @brief Constructor for GPIO interrupt pin
             * @param config Pin configuration with EXTI settings
             * @param callback Function to call when interrupt occurs
             */
            GPIOInterrupt( const PinConfig &config, void (*callback)(void) = nullptr ) : GPIOBase( config ), _callback(callback)
            {
                Init( config );
            }

            ~GPIOInterrupt() override
            {
                DeInit();
            }

            using GPIOBase::Read;

            /**
             * @brief Set the interrupt callback function
             * @param callback Function pointer to call on interrupt
             */
            void setCallback( void (*callback)(void) );



            /**
             * @brief Check if interrupt is pending
             * @return true if interrupt is pending
             */
            bool isInterruptPending( void );

            /**
             * @brief Clear pending interrupt flag
             */
            void clearInterrupt( void );

            /**
             * @brief Handle the interrupt (called from ISR)
             */
            void handleInterrupt( void );

        private:
            void (*_callback)(void);  /**< Interrupt callback function */

    }; // class GPIOInterrupt
} // namespace GPIO

#endif /* INC_GPIO_H */