/**
 * @file    App/Src/App.cpp
 * @brief   Application source file.
 * @details This file contains the implementation of the application initialization and start functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

/**
 * @include necessary headers
 */
#include "App.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include <cstdio>

#include "gpio.h"


static GPIO::PinConfig ledConfig = {
	.port = LED2_GPIO_Port,
	.pin = LED2_Pin,
	.mode = GPIO::PinMode::OUTPUT,
	.pull = GPIO::PinPull::NOPULL,
	.speed = GPIO::PinSpeed::LOW,
	.outputType = GPIO::PinOutputType::PUSHPULL,
};

static GPIO::PinConfig ledextConfig = {
	.port = LED3_GPIO_Port,
	.pin = LED3_Pin,
	.mode = GPIO::PinMode::OUTPUT,
	.pull = GPIO::PinPull::NOPULL,
	.speed = GPIO::PinSpeed::LOW,
	.outputType = GPIO::PinOutputType::PUSHPULL,
};

static GPIO::PinConfig buttonConfig = {
	.port = KEY_UP_GPIO_Port,
	.pin = KEY_UP_Pin,
	.mode = GPIO::PinMode::INPUT,
	.pull = GPIO::PinPull::PULLUP,
	.speed = GPIO::PinSpeed::LOW,
	.outputType = GPIO::PinOutputType::PUSHPULL,
	.alternate = 0,
	.extiTrigger = GPIO::ExtiTrigger::FALLING,
	.extiLine = 0  // KEY_OK is on PC3 -> EXTI Line 3
};

static GPIO::PinConfig buttonExtiConfig = {
	.port = KEY_OK_GPIO_Port,
	.pin = KEY_OK_Pin,
	.mode = GPIO::PinMode::INPUT,
	.pull = GPIO::PinPull::PULLUP,
	.speed = GPIO::PinSpeed::LOW,
	.outputType = GPIO::PinOutputType::PUSHPULL,
	.alternate = 0,
	.extiTrigger = GPIO::ExtiTrigger::FALLING,
	.extiLine = LL_EXTI_LINE_3  // KEY_OK is on PC3 -> EXTI Line 3
}; 

// Global GPIO objects
static GPIO::GPIOOutput* ledOutput = nullptr;
static GPIO::GPIOOutput* ledextOutput = nullptr;
static GPIO::GPIOInput* buttonInput = nullptr;
static GPIO::GPIOInterrupt* buttonExtiInterrupt = nullptr;


void buttonCallback(void)
{
	// Handle button press event
	ledextOutput->Toggle();
	printf("Button callback: LED toggled!\n");
}

// C-linkage function called from ISR
extern "C" void handleButtonEXTI3(void)
{
 	if (buttonExtiInterrupt != nullptr) {
		buttonExtiInterrupt->handleInterrupt();
	}
}

/**
 * @brief Initializes the application.
 */
void App_init( void )
{
    printf("App_init: Initializing application...\n");
    
    // Initialize LED and Button
    ledOutput = new GPIO::GPIOOutput(ledConfig);
	ledextOutput = new GPIO::GPIOOutput(ledextConfig);
    buttonInput = new GPIO::GPIOInput(buttonConfig);
    buttonExtiInterrupt = new GPIO::GPIOInterrupt(buttonExtiConfig, buttonCallback);
    
    // Turn off LED initially
    ledOutput->Write(true);
    ledextOutput->Write(true);
    
    printf("App_init: LED and Button initialized\n");
}

/**
 * @brief Starts the main application task.
 */
void App_start( void *data )
{
    printf("App_start: Main application task started\n");
    printf("Press KEY_OK button to toggle LED2\n");
    
    bool lastButtonState = true;  // Button is active low (pullup)
    bool currentButtonState = true;
    
    while( true )
    {
        // Read current button state
        currentButtonState = buttonInput->Read();
        
        // Check for button press (transition from high to low)
        if( lastButtonState == true && currentButtonState == false )
        {
            // Button pressed - toggle LED
            ledOutput->Toggle();
            printf("Button pressed - LED toggled\n");
            
            // Simple debounce delay
            vTaskDelay( pdMS_TO_TICKS( 50 ));
        }
        
        // Update last state
        lastButtonState = currentButtonState;
        
        // Small delay for polling
        vTaskDelay( pdMS_TO_TICKS( 10 ));
    }
}
