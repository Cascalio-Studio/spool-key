/**
 * @file    App/Src/gpio.cpp
 * @brief   GPIO source file.
 * @details This file contains the implementation of the GPIO initialization and control functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

 /**
  * @include necessary headers
  */
#include "gpio.h"
#include "main.h"
#include <cstdio>


/**
 * @namespace GPIO
 */
namespace GPIO
{
    void GPIOBase::Init( const PinConfig &config )
    {
        enableClock( config.port );
        configurePin( config );
    }

    void GPIOBase::DeInit( void )
    {
        LL_GPIO_DeInit( _config.port );
    }

    void GPIOBase::Write( bool state )
    {
        if ( _config.mode == PinMode::OUTPUT )
        {
            LL_GPIO_SetOutputPin( _config.port, _config.pin );
            if ( !state )
            {
                LL_GPIO_ResetOutputPin( _config.port, _config.pin );
            }
        }
    }

    bool GPIOBase::Read( void )
    {
        if ( _config.mode == PinMode::INPUT )
        {
            return LL_GPIO_IsInputPinSet( _config.port, _config.pin );
        }
        return false;
    }

    void GPIOBase::Toggle( void )
    {
        if ( _config.mode == PinMode::OUTPUT )
        {
            LL_GPIO_TogglePin( _config.port, _config.pin );
        }
    }

    void GPIOBase::enableClock( GPIO_TypeDef *port )
    {
        if ( port == GPIOA ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOA );
        else if ( port == GPIOB ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOB );
        else if ( port == GPIOC ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOC );
        else if ( port == GPIOD ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOD );
        else if ( port == GPIOE ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOE );
        else if ( port == GPIOH ) LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOH );
    }

    void GPIOBase::configurePin( const PinConfig &config )
    {
        LL_GPIO_InitTypeDef gpioInit = {0};

        gpioInit.Pin = config.pin;
        gpioInit.Mode = static_cast<uint32_t>( config.mode );
        gpioInit.Speed = static_cast<uint32_t>( config.speed );
        gpioInit.OutputType = static_cast<uint32_t>( config.outputType );
        gpioInit.Pull = static_cast<uint32_t>( config.pull );

        if ( config.mode == PinMode::ALTERNATE )
        {
            gpioInit.Alternate = config.alternate;
        }

        LL_GPIO_Init( config.port, &gpioInit );

        // Configure EXTI if needed
        if ( config.mode == PinMode::INPUT && config.extiLine != 0 )
        {
            printf("Configuring EXTI for port %p, pin %lu, line %lu\n", 
                   (void*)config.port, config.pin, config.extiLine);
            
            // Enable SYSCFG clock for EXTI configuration
            LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_SYSCFG );
            
            // Configure SYSCFG EXTI line to connect to the correct GPIO port
            uint32_t extiPort;
            if ( config.port == GPIOA ) extiPort = LL_SYSCFG_EXTI_PORTA;
            else if ( config.port == GPIOB ) extiPort = LL_SYSCFG_EXTI_PORTB;
            else if ( config.port == GPIOC ) extiPort = LL_SYSCFG_EXTI_PORTC;
            else if ( config.port == GPIOD ) extiPort = LL_SYSCFG_EXTI_PORTD;
            else if ( config.port == GPIOE ) extiPort = LL_SYSCFG_EXTI_PORTE;
            else if ( config.port == GPIOH ) extiPort = LL_SYSCFG_EXTI_PORTH;
            else return; // Invalid port
            
            // Get EXTI line number (0-15) from LL_EXTI_LINE_x
            uint32_t extiLineNumber = 0;
            uint32_t tempLine = config.extiLine;
            while ( tempLine > 1 ) { tempLine >>= 1; extiLineNumber++; }
            
            LL_SYSCFG_SetEXTISource( extiPort, extiLineNumber );

            LL_EXTI_InitTypeDef extiInit = {0};
            extiInit.Line_0_31 = config.extiLine;
            extiInit.LineCommand = ENABLE;
            extiInit.Mode = LL_EXTI_MODE_IT;
            extiInit.Trigger = static_cast<uint32_t>( config.extiTrigger );
            LL_EXTI_Init( &extiInit );

            // Enable and set EXTI Interrupt to the lowest priority
            uint32_t irqNumber;
            switch ( config.extiLine )
            {
                case LL_EXTI_LINE_0: irqNumber = EXTI0_IRQn; break;
                case LL_EXTI_LINE_1: irqNumber = EXTI1_IRQn; break;
                case LL_EXTI_LINE_2: irqNumber = EXTI2_IRQn; break;
                case LL_EXTI_LINE_3: irqNumber = EXTI3_IRQn; break;
                case LL_EXTI_LINE_4: irqNumber = EXTI4_IRQn; break;
                case LL_EXTI_LINE_9: irqNumber = EXTI9_5_IRQn; break;
                case LL_EXTI_LINE_15: irqNumber = EXTI15_10_IRQn; break;
                default: return; // Invalid line
            }
            NVIC_SetPriority( static_cast<IRQn_Type>( irqNumber ), NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 10, 0 ) );
            NVIC_EnableIRQ( static_cast<IRQn_Type>( irqNumber ) );
            
            printf("EXTI configured: Line %lu, Port %lu, IRQ %lu enabled\n", 
                   extiLineNumber, extiPort, irqNumber);
        }
    }



    void GPIOInterrupt::setCallback( void (*callback)(void) )
    {
        _callback = callback;
    }



    bool GPIOInterrupt::isInterruptPending( void )
    {
        return LL_EXTI_IsActiveFlag_0_31( _config.extiLine );
    }

    void GPIOInterrupt::clearInterrupt( void )
    {
        LL_EXTI_ClearFlag_0_31( _config.extiLine );
    }

    void GPIOInterrupt::handleInterrupt( void )
    {
        if ( isInterruptPending() )
        {
            clearInterrupt();
            if ( _callback != nullptr )
            {
                _callback();
            }
        }
    }
} // namespace GPIO