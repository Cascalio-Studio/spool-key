/**
 * @file    App/Inc/App.h
 * @brief   Application header file.
 * @details This file contains the declarations for the application initialization and start functions.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_APP_H
#define INC_APP_H

/**
 * @ifdef __cplusplus
 * @brief Ensures the C++ compiler uses C linkage for the enclosed declarations.
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the application.
 * @details This function sets up the necessary components for the application to run.
 */
void App_init( void );

/**
 * @brief Starts the main application task.
 * @param data Pointer to data passed to the task (if any).
 * @details This function contains the main loop of the application, toggling an LED every 2 seconds.
 */
void App_start( void *data );

/**
 * @brief Checks if the NFC chip is responding.
 * @details Reads the chip ID register to verify communication.
 */
void checkNFCChip( void );

#ifdef __cplusplus
}
#endif

#endif /* INC_APP_H */
