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

#include "gpioClass.h"
#include "spiClass.h"
#include "nfcTaskManager.h"
#include "st25r3911b.h"

// ST25R3911B NFC Controller SPI Configuration
static SPI::SPIConfig nfcSpiConfig = {
	.instance = SPI1,
	.mode = SPI::SPIMode::MODE_0,           // ST25R3911B uses SPI Mode 0
	.dataSize = SPI::SPIDataSize::SIZE_8BIT,
	.bitOrder = SPI::SPIBitOrder::MSB_FIRST,
	.speed = SPI::SPISpeed::PRESCALER_8,    // ~10MHz @ 80MHz sysclk
	
	// SPI1 GPIO Configuration for ST25R3911B
	.sckPort = GPIOA,                       // PA5 - SPI1_SCK
	.sckPin = LL_GPIO_PIN_5,
	.sckAlternate = LL_GPIO_AF_5,           // AF5 for SPI1
	
	.misoPort = GPIOA,                      // PA6 - SPI1_MISO  
	.misoPin = LL_GPIO_PIN_6,
	.misoAlternate = LL_GPIO_AF_5,          // AF5 for SPI1
	
	.mosiPort = GPIOA,                      // PA7 - SPI1_MOSI
	.mosiPin = LL_GPIO_PIN_7,
	.mosiAlternate = LL_GPIO_AF_5,          // AF5 for SPI1
	
	.csPort = GPIOA,                        // PA4 - Chip Select (manual)
	.csPin = LL_GPIO_PIN_4,
	
	.timeoutMs = 1000                       // 1 second timeout
};

// ST25R3911B Interrupt Pin Configuration  
static GPIO::PinConfig nfcIrqConfig = {
	.port = GPIOA,                          // PA0 - IRQ from ST25R3911B
	.pin = LL_GPIO_PIN_0,
	.mode = GPIO::PinMode::INPUT,
	.pull = GPIO::PinPull::PULLUP,
	.speed = GPIO::PinSpeed::HIGH,
	.outputType = GPIO::PinOutputType::PUSHPULL,
	.alternate = 0,
	.extiTrigger = GPIO::ExtiTrigger::FALLING,
	.extiLine = LL_EXTI_LINE_0              // PA0 -> EXTI Line 0
};

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

// Global SPI objects
static SPI::SPIMaster* nfcSpiMaster = nullptr;
static GPIO::GPIOInterrupt* nfcIrqInterrupt = nullptr;

// Global NFC objects
static NFC::ST25R3911B* nfcController = nullptr;
static NFC::NFCManager* nfcManager = nullptr;
static NFCTask::NFCTaskManager* nfcTaskManager = nullptr;


void buttonCallback(void)
{
	// Handle button press event
	ledextOutput->Toggle();
}

/**
 * @brief NFC interrupt callback function (called from interrupt)
 */
void nfcIrqCallback(void)
{
    if (nfcTaskManager) {
        nfcTaskManager->HandleNFCInterrupt();
    }
}

/**
 * @brief NFC tag detection callback
 */
void nfcTagDetectedCallback(const NFC::TagInfo& tagInfo)
{
    // Toggle LED to indicate tag detection
    static bool nfcLedState = false;
    nfcLedState = !nfcLedState;
    ledextOutput->Write(!nfcLedState);
    
    // Example: Read text from detected tag
    if (nfcTaskManager) {
        nfcTaskManager->ReadText([](const NFC::OperationResult& result) {
            if (result.status == NFC::NFCStatus::OK) {
                // Successfully read text - could store or process it
                printf("NFC: Successfully read text from tag\n");
            }
        });
    }
}

// C-linkage function called from ISR
extern "C" void handleButtonEXTI3(void)
{
 	if (buttonExtiInterrupt != nullptr) 
	{
		buttonExtiInterrupt->handleInterrupt();
	}
}

// C-linkage function for NFC interrupt (PA0 -> EXTI Line 0)
extern "C" void handleNFCEXTI0(void)
{
	if (nfcIrqInterrupt != nullptr) 
	{
		nfcIrqInterrupt->handleInterrupt();
	}
}

/**
 * @brief Initializes the application.
 */
void App_init( void )
{
    // Initialize LED and Button
    ledOutput = new GPIO::GPIOOutput(ledConfig);
	ledextOutput = new GPIO::GPIOOutput(ledextConfig);
    buttonInput = new GPIO::GPIOInput(buttonConfig);
    buttonExtiInterrupt = new GPIO::GPIOInterrupt(buttonExtiConfig, buttonCallback);
    
    // Initialize NFC SPI Interface
    nfcSpiMaster = new SPI::SPIMaster(nfcSpiConfig);
    nfcIrqInterrupt = new GPIO::GPIOInterrupt(nfcIrqConfig, nfcIrqCallback);
    
    // Initialize NFC Controller and Manager
    NFC::NFCConfig nfcConfig;
    nfcConfig.spiMaster = nfcSpiMaster;
    nfcConfig.irqPin = nfcIrqInterrupt;
    nfcConfig.defaultProtocol = NFC::NFCProtocol::NFC_A;
    nfcConfig.timeoutMs = 1000;
    nfcConfig.irqCallback = nfcIrqCallback;
    
    nfcController = new NFC::ST25R3911B(nfcConfig);
    nfcManager = new NFC::NFCManager(nfcController);
    
    // Initialize NFC Task Manager
    nfcTaskManager = new NFCTask::NFCTaskManager();
    NFCTask::NFCTaskConfig taskConfig = NFCTask::GetDefaultConfig();
    nfcTaskManager->Initialize(taskConfig, nfcManager);
    
    // Turn off LED initially
    ledOutput->Write(true);
    ledextOutput->Write(true);
}

/**
 * @brief Starts the main application task.
 */
void App_start( void *data )
{
    printf("App_start: Starting application task with NFC support\n");
    
    // Wait for system to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Start NFC tag detection
    if (nfcTaskManager) {
        // Start detection for ISO14443A and MIFARE tags
        uint32_t protocols = (1 << static_cast<uint32_t>(NFC::NFCProtocol::NFC_A)) | 
                            (1 << static_cast<uint32_t>(NFC::NFCProtocol::MIFARE_CLASSIC));
        
        NFC::NFCStatus status = nfcTaskManager->StartTagDetection(protocols, nfcTagDetectedCallback);
        if (status == NFC::NFCStatus::OK) {
            printf("NFC: Tag detection started\n");
        } else {
            printf("NFC: Failed to start tag detection\n");
        }
    }
    
    bool lastButtonState = true;  // Button is active low (pullup)
    bool currentButtonState = true;
    uint32_t loopCounter = 0;
    
    while( true )
    {
        // Read current button state
        currentButtonState = buttonInput->Read();
        
        // Check for button press (transition from high to low)
        if( lastButtonState == true && currentButtonState == false )
        {
            // Button pressed - demo NFC write operation
            ledOutput->Toggle();
            
            if (nfcTaskManager) {
                // Cycle through different write operations
                switch (loopCounter % 3) {
                    case 0:
                        nfcTaskManager->WriteURL("https://www.example.com", [](const NFC::OperationResult& result) {
                            if (result.status == NFC::NFCStatus::OK) {
                                printf("NFC: Successfully wrote URL to tag\n");
                            } else {
                                printf("NFC: Failed to write URL to tag\n");
                            }
                        });
                        break;
                    case 1:
                        nfcTaskManager->WriteText("Hello NFC World!", "en", [](const NFC::OperationResult& result) {
                            if (result.status == NFC::NFCStatus::OK) {
                                printf("NFC: Successfully wrote text to tag\n");
                            } else {
                                printf("NFC: Failed to write text to tag\n");
                            }
                        });
                        break;
                    case 2:
                        nfcTaskManager->WriteWiFi("MyWiFi", "Password123", "WPA2", [](const NFC::OperationResult& result) {
                            if (result.status == NFC::NFCStatus::OK) {
                                printf("NFC: Successfully wrote WiFi credentials to tag\n");
                            } else {
                                printf("NFC: Failed to write WiFi credentials to tag\n");
                            }
                        });
                        break;
                }
            }
            
            // Simple debounce delay
            vTaskDelay( pdMS_TO_TICKS( 50 ));
        }
        
        // Update last state
        lastButtonState = currentButtonState;
        
        // Every 10 seconds, print NFC statistics
        if ((loopCounter % 1000) == 0 && nfcTaskManager) {
            uint32_t processed, queued, highWater;
            nfcTaskManager->GetTaskStatistics(processed, queued, highWater);
            printf("NFC Stats: Processed=%lu, Queued=%lu, HighWater=%lu\n", 
                   processed, queued, highWater);
        }
        
        loopCounter++;
        
        // Small delay for polling
        vTaskDelay( pdMS_TO_TICKS( 10 ));
    }
}
