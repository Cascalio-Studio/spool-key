/**
 * Filename: App.cpp
 */

#include "App.h"

#include "FreeRTOS.h"
#include "task.h"


#include <cstdio>

#include "main.h"


void App_init( void )
{
    printf("App_init: Initializing application...\n");
}

void App_start( void *data )
{
    printf("App_start: Main application task started\n");
    
	while( true )
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		vTaskDelay( pdMS_TO_TICKS( 2000 ));
	}
}
