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


/**
 * @brief Initializes the application.
 */
void App_init( void )
{
    printf("App_init: Initializing application...\n");
}

/**
 * @brief Starts the main application task.
 */
void App_start( void *data )
{
    printf("App_start: Main application task started\n");
    
	while( true )
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		vTaskDelay( pdMS_TO_TICKS( 2000 ));
	}
}
