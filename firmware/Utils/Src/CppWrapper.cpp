/**
 * Filename: CppWrapper.cpp
 *
 */

#include "CppWrapper.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "App.h"

#include <new>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include <map>
#include <cstdlib>
#include <cstdio>

// C++ operator new/delete overrides for ST Newlib integration
// These use standard malloc/free which are thread-safe via heap_useNewlib_ST.c

void* operator new(size_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
    {
        // For embedded systems: Go into error handler to stop system
    	Error_Handler( );
    }
    return ptr;
}

void* operator new[](size_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
    {
        // For embedded systems: Go into error handler to stop system
    	Error_Handler( );
    }
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (ptr) {
        free(ptr);
    }
}

void operator delete[](void* ptr) noexcept
{
    if (ptr) {
        free(ptr);
    }
}

// C++14 sized delete operators
void operator delete(void* ptr, size_t size) noexcept
{
    (void)size; // Suppress unused parameter warning
    if (ptr) {
        free(ptr);
    }
}

void operator delete[](void* ptr, size_t size) noexcept
{
    (void)size; // Suppress unused parameter warning
    if (ptr) {
        free(ptr);
    }
}


void EventLoopC( void )
{
	App_init();

	size_t usage = xPortGetFreeHeapSize();
	TaskHandle_t handle;
	BaseType_t xReturned;
	xReturned = xTaskCreate(App_start, "Main Thread", 256, 0,
							tskIDLE_PRIORITY + 2, &handle);
	if (pdPASS != xReturned)
	{
		while (1)
		{
		}
	}
	usage = xPortGetFreeHeapSize();
	printf("Free heap: %u bytes\n", usage);

	vTaskStartScheduler();
}

