/**
 * @file    App/Src/nfcTaskManager.cpp
 * @brief   NFC Task Manager Implementation
 * @details This file contains the implementation of the NFC task manager for non-blocking operations.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

/**
 * @include necessary headers
 */
#include "nfcTaskManager.h"
#include <cstring>

namespace NFCTask
{
    // ============================================================================
    // NFCTaskManager Implementation
    // ============================================================================

    NFCTaskManager::NFCTaskManager()
        : _initialized(false)
        , _nfcManager(nullptr)
        , _taskHandle(nullptr)
        , _commandQueue(nullptr)
        , _responseQueue(nullptr)
        , _nfcMutex(nullptr)
        , _commandsProcessed(0)
        , _nextRequestId(1)
    {
    }

    NFCTaskManager::~NFCTaskManager()
    {
        Deinitialize();
    }

    NFC::NFCStatus NFCTaskManager::Initialize(const NFCTaskConfig& config, NFC::NFCManager* nfcManager)
    {
        if (_initialized || !nfcManager) {
            return NFC::NFCStatus::ERROR;
        }

        _config = config;
        _nfcManager = nfcManager;

        // Create command queue
        _commandQueue = xQueueCreate(_config.commandQueueSize, sizeof(NFCCommandData));
        if (!_commandQueue) {
            return NFC::NFCStatus::ERROR;
        }

        // Create response queue
        _responseQueue = xQueueCreate(_config.responseQueueSize, sizeof(NFC::OperationResult));
        if (!_responseQueue) {
            vQueueDelete(_commandQueue);
            return NFC::NFCStatus::ERROR;
        }

        // Create mutex
        _nfcMutex = xSemaphoreCreateMutex();
        if (!_nfcMutex) {
            vQueueDelete(_commandQueue);
            vQueueDelete(_responseQueue);
            return NFC::NFCStatus::ERROR;
        }

        // Create NFC task
        BaseType_t result = xTaskCreate(
            nfcTaskFunction,
            _config.taskName,
            _config.taskStackSize,
            this,
            _config.taskPriority,
            &_taskHandle
        );

        if (result != pdPASS) {
            vQueueDelete(_commandQueue);
            vQueueDelete(_responseQueue);
            vSemaphoreDelete(_nfcMutex);
            return NFC::NFCStatus::ERROR;
        }

        _initialized = true;
        return NFC::NFCStatus::OK;
    }

    NFC::NFCStatus NFCTaskManager::Deinitialize(void)
    {
        if (!_initialized) {
            return NFC::NFCStatus::OK;
        }

        // Delete task
        if (_taskHandle) {
            vTaskDelete(_taskHandle);
            _taskHandle = nullptr;
        }

        // Delete queues and mutex
        if (_commandQueue) {
            vQueueDelete(_commandQueue);
            _commandQueue = nullptr;
        }

        if (_responseQueue) {
            vQueueDelete(_responseQueue);
            _responseQueue = nullptr;
        }

        if (_nfcMutex) {
            vSemaphoreDelete(_nfcMutex);
            _nfcMutex = nullptr;
        }

        _initialized = false;
        return NFC::NFCStatus::OK;
    }

    NFC::NFCStatus NFCTaskManager::SendCommand(const NFCCommandData& command, uint32_t timeoutMs)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        TickType_t timeout = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
        
        if (xQueueSend(_commandQueue, &command, timeout) != pdPASS) {
            return NFC::NFCStatus::TIMEOUT;
        }

        return NFC::NFCStatus::OK;
    }

    NFC::NFCStatus NFCTaskManager::StartTagDetection(uint32_t protocols, std::function<void(const NFC::TagInfo&)> callback)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        _detectionCallback = callback;

        NFCCommandData command;
        command.command = NFCCommand::START_DETECTION;
        command.priority = NFCPriority::HIGH;
        command.requestId = generateRequestId();
        command.protocolMask = protocols;

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::StopTagDetection(void)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        _detectionCallback = nullptr;

        NFCCommandData command;
        command.command = NFCCommand::STOP_DETECTION;
        command.priority = NFCPriority::HIGH;
        command.requestId = generateRequestId();

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::ReadText(std::function<void(const NFC::OperationResult&)> callback)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        NFCCommandData command;
        command.command = NFCCommand::READ_TEXT;
        command.priority = NFCPriority::NORMAL;
        command.requestId = generateRequestId();
        command.callback = callback;

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::WriteText(const std::string& text, const std::string& language, std::function<void(const NFC::OperationResult&)> callback)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        NFCCommandData command;
        command.command = NFCCommand::WRITE_TEXT;
        command.priority = NFCPriority::NORMAL;
        command.requestId = generateRequestId();
        command.textData = text;
        command.languageCode = language;
        command.callback = callback;

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::WriteURL(const std::string& url, std::function<void(const NFC::OperationResult&)> callback)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        NFCCommandData command;
        command.command = NFCCommand::WRITE_URL;
        command.priority = NFCPriority::NORMAL;
        command.requestId = generateRequestId();
        command.uriData = url;
        command.callback = callback;

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::WriteWiFi(const std::string& ssid, const std::string& password, const std::string& security, std::function<void(const NFC::OperationResult&)> callback)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        NFCCommandData command;
        command.command = NFCCommand::WRITE_WIFI;
        command.priority = NFCPriority::NORMAL;
        command.requestId = generateRequestId();
        command.wifiSSID = ssid;
        command.wifiPassword = password;
        command.wifiSecurity = security;
        command.callback = callback;

        return SendCommand(command);
    }

    NFC::NFCStatus NFCTaskManager::SetField(NFC::NFCField field)
    {
        if (!_initialized) {
            return NFC::NFCStatus::NOT_INITIALIZED;
        }

        NFCCommandData command;
        command.command = NFCCommand::SET_FIELD;
        command.priority = NFCPriority::HIGH;
        command.requestId = generateRequestId();
        command.fieldState = field;

        return SendCommand(command);
    }

    void NFCTaskManager::GetTaskStatistics(uint32_t& commandsProcessed, uint32_t& commandsQueued, uint32_t& taskHighWaterMark)
    {
        commandsProcessed = _commandsProcessed;
        commandsQueued = _commandQueue ? uxQueueMessagesWaiting(_commandQueue) : 0;
        taskHighWaterMark = _taskHandle ? uxTaskGetStackHighWaterMark(_taskHandle) : 0;
    }

    void NFCTaskManager::HandleNFCInterrupt(void)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Notify NFC task about interrupt
        if (_taskHandle) {
            vTaskNotifyGiveFromISR(_taskHandle, &xHigherPriorityTaskWoken);
        }
        
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // ============================================================================
    // Private Methods
    // ============================================================================

    void NFCTaskManager::nfcTaskFunction(void* pvParameters)
    {
        NFCTaskManager* manager = static_cast<NFCTaskManager*>(pvParameters);
        manager->taskMainLoop();
    }

    void NFCTaskManager::taskMainLoop(void)
    {
        NFCCommandData command;
        const TickType_t maxBlockTime = pdMS_TO_TICKS(100);

        while (true) {
            // Wait for command or notification
            if (xQueueReceive(_commandQueue, &command, maxBlockTime) == pdPASS) {
                // Process command
                NFC::OperationResult result = processCommand(command);
                
                // Send result via callback or store in response queue
                sendResult(result, command.callback);
                
                _commandsProcessed++;
            }

            // Handle task notifications (interrupts)
            if (ulTaskNotifyTake(pdFALSE, 0) > 0) {
                // NFC interrupt occurred - let the NFC manager handle it
                if (_nfcManager) {
                    // This would typically trigger tag detection callbacks
                }
            }

            // Yield to other tasks
            taskYIELD();
        }
    }

    NFC::OperationResult NFCTaskManager::processCommand(const NFCCommandData& command)
    {
        NFC::OperationResult result;
        result.status = NFC::NFCStatus::OK;

        // Take mutex for NFC operations
        if (xSemaphoreTake(_nfcMutex, pdMS_TO_TICKS(_config.taskTimeoutMs)) != pdPASS) {
            return CreateErrorResult(NFC::TagOperation::READ, NFC::NFCStatus::TIMEOUT, "Failed to acquire NFC mutex");
        }

        switch (command.command) {
            case NFCCommand::INITIALIZE:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->Initialize();
                break;

            case NFCCommand::DEINITIALIZE:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->Deinitialize();
                break;

            case NFCCommand::START_DETECTION:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->StartTagDetection(command.protocolMask, 
                    [this](const NFC::TagInfo& tagInfo) { 
                        this->handleTagDetection(tagInfo); 
                    });
                break;

            case NFCCommand::STOP_DETECTION:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->StopTagDetection();
                break;

            case NFCCommand::READ_TEXT:
                result.operation = NFC::TagOperation::READ;
                if (_nfcManager->GetTagReader()) {
                    std::string text, language;
                    // This would need the current tag info - simplified for example
                    NFC::TagInfo dummyTag; // In real implementation, store current tag
                    result.status = _nfcManager->GetTagReader()->ReadText(dummyTag, text, language);
                    if (result.status == NFC::NFCStatus::OK) {
                        NFC::NDEFRecord record;
                        record.type = NFC::NDEFRecordType::TEXT;
                        record.payload = text;
                        record.language = language;
                        result.ndefMessage.records.push_back(record);
                    }
                } else {
                    result.status = NFC::NFCStatus::NOT_INITIALIZED;
                }
                break;

            case NFCCommand::WRITE_TEXT:
                result.operation = NFC::TagOperation::WRITE;
                if (_nfcManager->GetTagWriter()) {
                    NFC::TagInfo dummyTag; // In real implementation, use current tag
                    result.status = _nfcManager->GetTagWriter()->WriteText(dummyTag, command.textData, command.languageCode);
                } else {
                    result.status = NFC::NFCStatus::NOT_INITIALIZED;
                }
                break;

            case NFCCommand::WRITE_URL:
                result.operation = NFC::TagOperation::WRITE;
                if (_nfcManager->GetTagWriter()) {
                    NFC::TagInfo dummyTag; // In real implementation, use current tag
                    result.status = _nfcManager->GetTagWriter()->WriteURL(dummyTag, command.uriData);
                } else {
                    result.status = NFC::NFCStatus::NOT_INITIALIZED;
                }
                break;

            case NFCCommand::WRITE_WIFI:
                result.operation = NFC::TagOperation::WRITE;
                if (_nfcManager->GetTagWriter()) {
                    NFC::TagInfo dummyTag; // In real implementation, use current tag
                    result.status = _nfcManager->GetTagWriter()->WriteWiFi(dummyTag, command.wifiSSID, command.wifiPassword, command.wifiSecurity);
                } else {
                    result.status = NFC::NFCStatus::NOT_INITIALIZED;
                }
                break;

            case NFCCommand::SET_FIELD:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->SetField(command.fieldState);
                break;

            case NFCCommand::FORMAT_TAG:
                result.operation = NFC::TagOperation::FORMAT;
                if (_nfcManager->GetTagWriter()) {
                    NFC::TagInfo dummyTag; // In real implementation, use current tag
                    result.status = _nfcManager->GetTagWriter()->FormatTag(dummyTag);
                } else {
                    result.status = NFC::NFCStatus::NOT_INITIALIZED;
                }
                break;

            case NFCCommand::GET_STATUS:
                result.operation = NFC::TagOperation::DETECT;
                result.status = _nfcManager->IsDetectionActive() ? NFC::NFCStatus::OK : NFC::NFCStatus::ERROR;
                break;

            default:
                result = CreateErrorResult(NFC::TagOperation::READ, NFC::NFCStatus::INVALID_PARAM, "Unknown command");
                break;
        }

        // Release mutex
        xSemaphoreGive(_nfcMutex);

        return result;
    }

    void NFCTaskManager::sendResult(const NFC::OperationResult& result, std::function<void(const NFC::OperationResult&)> callback)
    {
        if (callback) {
            // Call callback function
            callback(result);
        } else {
            // Store in response queue
            xQueueSend(_responseQueue, &result, 0);
        }
    }

    uint32_t NFCTaskManager::generateRequestId(void)
    {
        return _nextRequestId++;
    }

    void NFCTaskManager::handleTagDetection(const NFC::TagInfo& tagInfo)
    {
        if (_detectionCallback) {
            _detectionCallback(tagInfo);
        }
    }

    // ============================================================================
    // Global Functions
    // ============================================================================

    NFCTaskConfig GetDefaultConfig(void)
    {
        NFCTaskConfig config;
        config.taskStackSize = 2048;
        config.taskPriority = tskIDLE_PRIORITY + 2;
        config.commandQueueSize = 10;
        config.responseQueueSize = 10;
        config.taskTimeoutMs = 5000;
        config.taskName = "NFCTask";
        
        return config;
    }

    NFC::OperationResult CreateErrorResult(NFC::TagOperation operation, NFC::NFCStatus status, const std::string& message)
    {
        NFC::OperationResult result;
        result.operation = operation;
        result.status = status;
        result.errorMessage = message;
        
        return result;
    }

} // namespace NFCTask