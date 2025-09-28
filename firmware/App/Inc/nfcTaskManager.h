/**
 * @file    App/Inc/nfcTaskManager.h
 * @brief   NFC Task Manager Header
 * @details This file contains the NFC task manager for non-blocking operations with FreeRTOS.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_NFC_TASK_MANAGER_H
#define INC_NFC_TASK_MANAGER_H

/**
 * @include necessary headers
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "nfcClass.h"
#include "st25r3911b.h"
#include <vector>
#include <string>
#include <functional>

/**
 * @namespace NFCTask
 * @brief Contains NFC task management functions and definitions.
 */
namespace NFCTask
{
    /**
     * @enum NFCCommand
     * @brief NFC command types for task queue.
     */
    enum class NFCCommand
    {
        INITIALIZE = 0,         /**< Initialize NFC system */
        DEINITIALIZE,          /**< Deinitialize NFC system */
        START_DETECTION,       /**< Start tag detection */
        STOP_DETECTION,        /**< Stop tag detection */
        READ_TAG,              /**< Read tag data */
        WRITE_TAG,             /**< Write tag data */
        READ_UID,              /**< Read tag UID only */
        READ_TEXT,             /**< Read text from tag */
        READ_URI,              /**< Read URI from tag */
        READ_WIFI,             /**< Read WiFi credentials */
        WRITE_TEXT,            /**< Write text to tag */
        WRITE_URI,             /**< Write URI to tag */
        WRITE_WIFI,            /**< Write WiFi credentials */
        WRITE_URL,             /**< Write URL to tag */
        WRITE_EMAIL,           /**< Write email to tag */
        WRITE_PHONE,           /**< Write phone number to tag */
        FORMAT_TAG,            /**< Format tag for NDEF */
        SET_FIELD,             /**< Set RF field state */
        GET_STATUS             /**< Get NFC status */
    };

    /**
     * @enum NFCPriority
     * @brief Command priority levels.
     */
    enum class NFCPriority
    {
        LOW = 0,               /**< Low priority */
        NORMAL,                /**< Normal priority */
        HIGH,                  /**< High priority */
        URGENT                 /**< Urgent priority */
    };

    /**
     * @struct NFCCommandData
     * @brief Command data structure for different NFC operations.
     */
    struct NFCCommandData
    {
        NFCCommand command;                     /**< Command type */
        NFCPriority priority;                   /**< Command priority */
        uint32_t requestId;                     /**< Unique request ID */
        
        // Data fields for various commands
        std::string textData;                   /**< Text data */
        std::string uriData;                    /**< URI data */
        std::string languageCode;               /**< Language code */
        std::string wifiSSID;                   /**< WiFi SSID */
        std::string wifiPassword;               /**< WiFi password */
        std::string wifiSecurity;               /**< WiFi security type */
        std::string emailAddress;               /**< Email address */
        std::string emailSubject;               /**< Email subject */
        std::string emailBody;                  /**< Email body */
        std::string phoneNumber;                /**< Phone number */
        NFC::NFCField fieldState;               /**< Field state */
        uint32_t protocolMask;                  /**< Protocol mask for detection */
        uint16_t address;                       /**< Address for raw operations */
        std::vector<uint8_t> rawData;           /**< Raw data */
        
        // Callback function
        std::function<void(const NFC::OperationResult&)> callback; /**< Result callback */
        
        /**
         * @brief Default constructor
         */
        NFCCommandData() : command(NFCCommand::GET_STATUS), priority(NFCPriority::NORMAL), requestId(0) {}
    };

    /**
     * @struct NFCTaskConfig
     * @brief Configuration for NFC task.
     */
    struct NFCTaskConfig
    {
        uint16_t taskStackSize;                 /**< Task stack size */
        UBaseType_t taskPriority;               /**< Task priority */
        uint8_t commandQueueSize;               /**< Command queue size */
        uint8_t responseQueueSize;              /**< Response queue size */
        uint32_t taskTimeoutMs;                 /**< Task operation timeout */
        const char* taskName;                   /**< Task name */
    };

    /**
     * @class NFCTaskManager
     * @brief NFC task manager for non-blocking operations.
     */
    class NFCTaskManager
    {
        public:
            /**
             * @brief Constructor
             */
            NFCTaskManager();

            /**
             * @brief Destructor
             */
            ~NFCTaskManager();

            /**
             * @brief Initialize NFC task manager
             * @param config Task configuration
             * @param nfcManager NFC manager instance
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus Initialize(const NFCTaskConfig& config, NFC::NFCManager* nfcManager);

            /**
             * @brief Deinitialize NFC task manager
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus Deinitialize(void);

            /**
             * @brief Check if task manager is initialized
             * @return true if initialized, false otherwise
             */
            bool IsInitialized(void) const { return _initialized; }

            /**
             * @brief Send command to NFC task
             * @param command Command data
             * @param timeoutMs Timeout in milliseconds (0 = no wait)
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus SendCommand(const NFCCommandData& command, uint32_t timeoutMs = 0);

            /**
             * @brief Send high-level commands with simplified interface
             */

            /**
             * @brief Start tag detection
             * @param protocols Protocol mask
             * @param callback Detection callback
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus StartTagDetection(uint32_t protocols, std::function<void(const NFC::TagInfo&)> callback);

            /**
             * @brief Stop tag detection
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus StopTagDetection(void);

            /**
             * @brief Read text from tag
             * @param callback Result callback
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus ReadText(std::function<void(const NFC::OperationResult&)> callback);

            /**
             * @brief Write text to tag
             * @param text Text to write
             * @param language Language code
             * @param callback Result callback
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus WriteText(const std::string& text, const std::string& language, std::function<void(const NFC::OperationResult&)> callback);

            /**
             * @brief Write URL to tag
             * @param url URL to write
             * @param callback Result callback
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus WriteURL(const std::string& url, std::function<void(const NFC::OperationResult&)> callback);

            /**
             * @brief Write WiFi credentials to tag
             * @param ssid WiFi SSID
             * @param password WiFi password
             * @param security Security type
             * @param callback Result callback
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus WriteWiFi(const std::string& ssid, const std::string& password, const std::string& security, std::function<void(const NFC::OperationResult&)> callback);

            /**
             * @brief Set RF field state
             * @param field Field state
             * @return NFC::NFCStatus indicating success or failure
             */
            NFC::NFCStatus SetField(NFC::NFCField field);

            /**
             * @brief Get task statistics
             * @param commandsProcessed Reference to store commands processed
             * @param commandsQueued Reference to store commands in queue
             * @param taskHighWaterMark Reference to store task high water mark
             */
            void GetTaskStatistics(uint32_t& commandsProcessed, uint32_t& commandsQueued, uint32_t& taskHighWaterMark);

            /**
             * @brief Handle NFC interrupt (called from ISR)
             */
            void HandleNFCInterrupt(void);

        private:
            bool _initialized;                      /**< Initialization status */
            NFCTaskConfig _config;                  /**< Task configuration */
            NFC::NFCManager* _nfcManager;           /**< NFC manager instance */
            
            // FreeRTOS objects
            TaskHandle_t _taskHandle;               /**< Task handle */
            QueueHandle_t _commandQueue;            /**< Command queue */
            QueueHandle_t _responseQueue;           /**< Response queue */
            SemaphoreHandle_t _nfcMutex;            /**< NFC access mutex */
            
            // Statistics
            uint32_t _commandsProcessed;            /**< Commands processed counter */
            uint32_t _nextRequestId;                /**< Next request ID */
            
            // Detection callback
            std::function<void(const NFC::TagInfo&)> _detectionCallback;

            /**
             * @brief NFC task function (static)
             * @param pvParameters Task parameters
             */
            static void nfcTaskFunction(void* pvParameters);

            /**
             * @brief NFC task main loop
             */
            void taskMainLoop(void);

            /**
             * @brief Process NFC command
             * @param command Command to process
             * @return NFC::OperationResult with result
             */
            NFC::OperationResult processCommand(const NFCCommandData& command);

            /**
             * @brief Send result via callback or queue
             * @param result Operation result
             * @param callback Callback function (optional)
             */
            void sendResult(const NFC::OperationResult& result, std::function<void(const NFC::OperationResult&)> callback = nullptr);

            /**
             * @brief Generate unique request ID
             * @return Unique request ID
             */
            uint32_t generateRequestId(void);

            /**
             * @brief Handle tag detection event
             * @param tagInfo Detected tag information
             */
            void handleTagDetection(const NFC::TagInfo& tagInfo);
    };

    // ============================================================================
    // Global Functions
    // ============================================================================

    /**
     * @brief Get default NFC task configuration
     * @return Default task configuration
     */
    NFCTaskConfig GetDefaultConfig(void);

    /**
     * @brief Create operation result with error
     * @param operation Operation type
     * @param status Error status
     * @param message Error message
     * @return Operation result
     */
    NFC::OperationResult CreateErrorResult(NFC::TagOperation operation, NFC::NFCStatus status, const std::string& message);

} // namespace NFCTask

#endif /* INC_NFC_TASK_MANAGER_H */