/**
 * @file    App/Inc/nfcClass.h
 * @brief   High-Level NFC API Classes Header
 * @details This file contains the high-level NFC API classes for tag reading and writing operations.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_NFC_CLASS_H
#define INC_NFC_CLASS_H

/**
 * @include necessary headers
 */
#include "st25r3911b.h"
#include <vector>
#include <string>
#include <functional>

/**
 * @namespace NFC
 * @brief Contains NFC related functions and definitions.
 */
namespace NFC
{
    /**
     * @enum NDEFRecordType
     * @brief NDEF record types.
     */
    enum class NDEFRecordType
    {
        TEXT = 0,               /**< Text record */
        URI,                    /**< URI record */
        MIME,                   /**< MIME type record */
        WIFI,                   /**< WiFi configuration record */
        PHONE,                  /**< Phone number record */
        EMAIL,                  /**< Email record */
        VCARD,                  /**< vCard contact record */
        UNKNOWN                 /**< Unknown record type */
    };

    /**
     * @struct NDEFRecord
     * @brief NDEF record structure.
     */
    struct NDEFRecord
    {
        NDEFRecordType type;        /**< Record type */
        std::string payload;        /**< Record payload as string */
        std::vector<uint8_t> rawData; /**< Raw record data */
        std::string language;       /**< Language code (for text records) */
        std::string mimeType;       /**< MIME type (for MIME records) */
    };

    /**
     * @struct NDEFMessage
     * @brief NDEF message containing multiple records.
     */
    struct NDEFMessage
    {
        std::vector<NDEFRecord> records; /**< NDEF records */
        uint16_t totalSize;             /**< Total message size in bytes */
    };

    /**
     * @enum TagOperation
     * @brief Tag operation types for callbacks.
     */
    enum class TagOperation
    {
        DETECT = 0,             /**< Tag detection */
        READ,                   /**< Tag read operation */
        WRITE,                  /**< Tag write operation */
        FORMAT,                 /**< Tag format operation */
        AUTHENTICATE            /**< Tag authentication */
    };

    /**
     * @struct OperationResult
     * @brief Result of tag operation.
     */
    struct OperationResult
    {
        TagOperation operation;     /**< Operation type */
        NFCStatus status;          /**< Operation status */
        TagInfo tagInfo;           /**< Tag information */
        NDEFMessage ndefMessage;   /**< NDEF message (for read operations) */
        std::string errorMessage;  /**< Error description */
    };

    // Forward declarations
    class TagReader;
    class TagWriter;

    /**
     * @brief Callback function type for tag operations
     */
    using TagOperationCallback = std::function<void(const OperationResult&)>;

    /**
     * @brief Callback function type for tag detection
     */
    using TagDetectionCallback = std::function<void(const TagInfo&)>;

    /**
     * @class NFCManager
     * @brief High-level NFC manager class for coordinating operations.
     */
    class NFCManager
    {
        public:
            /**
             * @brief Constructor
             * @param controller ST25R3911B controller instance
             */
            NFCManager(ST25R3911B* controller);

            /**
             * @brief Destructor
             */
            ~NFCManager();

            /**
             * @brief Initialize NFC manager
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Initialize(void);

            /**
             * @brief Deinitialize NFC manager
             * @return NFCStatus indicating success or failure
             */
            NFCStatus Deinitialize(void);

            /**
             * @brief Start tag detection
             * @param protocols Protocols to detect (bitmask)
             * @param callback Callback for tag detection
             * @return NFCStatus indicating success or failure
             */
            NFCStatus StartTagDetection(uint32_t protocols, TagDetectionCallback callback);

            /**
             * @brief Stop tag detection
             * @return NFCStatus indicating success or failure
             */
            NFCStatus StopTagDetection(void);

            /**
             * @brief Check if tag detection is active
             * @return true if active, false otherwise
             */
            bool IsDetectionActive(void) const { return _detectionActive; }

            /**
             * @brief Get tag reader instance
             * @return Pointer to tag reader
             */
            TagReader* GetTagReader(void) { return _tagReader; }

            /**
             * @brief Get tag writer instance
             * @return Pointer to tag writer
             */
            TagWriter* GetTagWriter(void) { return _tagWriter; }

            /**
             * @brief Set field state
             * @param field Field state
             * @return NFCStatus indicating success or failure
             */
            NFCStatus SetField(NFCField field);

            /**
             * @brief Get current field state
             * @return Current field state
             */
            NFCField GetField(void) const;

        private:
            ST25R3911B* _controller;        /**< NFC controller */
            TagReader* _tagReader;          /**< Tag reader instance */
            TagWriter* _tagWriter;          /**< Tag writer instance */
            bool _initialized;              /**< Initialization status */
            bool _detectionActive;          /**< Detection active flag */
            TagDetectionCallback _detectionCallback; /**< Detection callback */
            uint32_t _detectionProtocols;   /**< Protocols to detect */

            /**
             * @brief Handle tag detection interrupt
             */
            void handleTagDetection(void);

            /**
             * @brief Identify tag type from response
             * @param response Tag response data
             * @param tagInfo Reference to store tag information
             * @return NFCStatus indicating success or failure
             */
            NFCStatus identifyTag(const std::vector<uint8_t>& response, TagInfo& tagInfo);
    };

    /**
     * @class TagReader
     * @brief High-level tag reader class.
     */
    class TagReader
    {
        public:
            /**
             * @brief Constructor
             * @param controller ST25R3911B controller instance
             */
            TagReader(ST25R3911B* controller);

            /**
             * @brief Destructor
             */
            ~TagReader() = default;

            /**
             * @brief Read tag UID
             * @param tagInfo Tag information
             * @param uid Vector to store UID
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadUID(const TagInfo& tagInfo, std::vector<uint8_t>& uid);

            /**
             * @brief Read raw data from tag
             * @param tagInfo Tag information
             * @param address Start address
             * @param length Number of bytes to read
             * @param data Vector to store read data
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadRawData(const TagInfo& tagInfo, uint16_t address, uint16_t length, std::vector<uint8_t>& data);

            /**
             * @brief Read NDEF message from tag
             * @param tagInfo Tag information
             * @param message Reference to store NDEF message
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadNDEF(const TagInfo& tagInfo, NDEFMessage& message);

            /**
             * @brief Read text record from tag
             * @param tagInfo Tag information
             * @param text Reference to store text
             * @param language Reference to store language code
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadText(const TagInfo& tagInfo, std::string& text, std::string& language);

            /**
             * @brief Read URI from tag
             * @param tagInfo Tag information
             * @param uri Reference to store URI
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadURI(const TagInfo& tagInfo, std::string& uri);

            /**
             * @brief Read WiFi configuration from tag
             * @param tagInfo Tag information
             * @param ssid Reference to store SSID
             * @param password Reference to store password
             * @param security Reference to store security type
             * @return NFCStatus indicating success or failure
             */
            NFCStatus ReadWiFi(const TagInfo& tagInfo, std::string& ssid, std::string& password, std::string& security);

            /**
             * @brief Set operation callback
             * @param callback Callback function
             */
            void SetCallback(TagOperationCallback callback) { _callback = callback; }

        private:
            ST25R3911B* _controller;        /**< NFC controller */
            TagOperationCallback _callback; /**< Operation callback */

            /**
             * @brief Parse NDEF message from raw data
             * @param data Raw NDEF data
             * @param message Reference to store parsed message
             * @return NFCStatus indicating success or failure
             */
            NFCStatus parseNDEFMessage(const std::vector<uint8_t>& data, NDEFMessage& message);

            /**
             * @brief Parse NDEF record
             * @param data Raw record data
             * @param offset Starting offset
             * @param record Reference to store parsed record
             * @param bytesRead Reference to store number of bytes read
             * @return NFCStatus indicating success or failure
             */
            NFCStatus parseNDEFRecord(const std::vector<uint8_t>& data, size_t offset, NDEFRecord& record, size_t& bytesRead);

            /**
             * @brief Read from ISO14443A tag
             * @param address Address to read from
             * @param length Number of bytes to read
             * @param data Vector to store read data
             * @return NFCStatus indicating success or failure
             */
            NFCStatus readISO14443A(uint16_t address, uint16_t length, std::vector<uint8_t>& data);

            /**
             * @brief Read from MIFARE Classic tag
             * @param block Block number to read
             * @param data Vector to store read data
             * @return NFCStatus indicating success or failure
             */
            NFCStatus readMifareClassic(uint8_t block, std::vector<uint8_t>& data);
    };

    /**
     * @class TagWriter
     * @brief High-level tag writer class.
     */
    class TagWriter
    {
        public:
            /**
             * @brief Constructor
             * @param controller ST25R3911B controller instance
             */
            TagWriter(ST25R3911B* controller);

            /**
             * @brief Destructor
             */
            ~TagWriter() = default;

            /**
             * @brief Write raw data to tag
             * @param tagInfo Tag information
             * @param address Start address
             * @param data Data to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteRawData(const TagInfo& tagInfo, uint16_t address, const std::vector<uint8_t>& data);

            /**
             * @brief Write NDEF message to tag
             * @param tagInfo Tag information
             * @param message NDEF message to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteNDEF(const TagInfo& tagInfo, const NDEFMessage& message);

            /**
             * @brief Write text record to tag
             * @param tagInfo Tag information
             * @param text Text to write
             * @param language Language code
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteText(const TagInfo& tagInfo, const std::string& text, const std::string& language = "en");

            /**
             * @brief Write URI to tag
             * @param tagInfo Tag information
             * @param uri URI to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteURI(const TagInfo& tagInfo, const std::string& uri);

            /**
             * @brief Write URL to tag
             * @param tagInfo Tag information
             * @param url URL to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteURL(const TagInfo& tagInfo, const std::string& url);

            /**
             * @brief Write WiFi configuration to tag
             * @param tagInfo Tag information
             * @param ssid WiFi SSID
             * @param password WiFi password
             * @param security Security type
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteWiFi(const TagInfo& tagInfo, const std::string& ssid, const std::string& password, const std::string& security = "WPA2");

            /**
             * @brief Write phone number to tag
             * @param tagInfo Tag information
             * @param phoneNumber Phone number to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WritePhone(const TagInfo& tagInfo, const std::string& phoneNumber);

            /**
             * @brief Write email address to tag
             * @param tagInfo Tag information
             * @param email Email address to write
             * @param subject Email subject (optional)
             * @param body Email body (optional)
             * @return NFCStatus indicating success or failure
             */
            NFCStatus WriteEmail(const TagInfo& tagInfo, const std::string& email, const std::string& subject = "", const std::string& body = "");

            /**
             * @brief Format tag for NDEF
             * @param tagInfo Tag information
             * @return NFCStatus indicating success or failure
             */
            NFCStatus FormatTag(const TagInfo& tagInfo);

            /**
             * @brief Set operation callback
             * @param callback Callback function
             */
            void SetCallback(TagOperationCallback callback) { _callback = callback; }

        private:
            ST25R3911B* _controller;        /**< NFC controller */
            TagOperationCallback _callback; /**< Operation callback */

            /**
             * @brief Create NDEF message from records
             * @param records NDEF records
             * @param data Vector to store encoded message
             * @return NFCStatus indicating success or failure
             */
            NFCStatus createNDEFMessage(const std::vector<NDEFRecord>& records, std::vector<uint8_t>& data);

            /**
             * @brief Create NDEF record
             * @param record NDEF record
             * @param data Vector to store encoded record
             * @return NFCStatus indicating success or failure
             */
            NFCStatus createNDEFRecord(const NDEFRecord& record, std::vector<uint8_t>& data);

            /**
             * @brief Write to ISO14443A tag
             * @param address Address to write to
             * @param data Data to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus writeISO14443A(uint16_t address, const std::vector<uint8_t>& data);

            /**
             * @brief Write to MIFARE Classic tag
             * @param block Block number to write
             * @param data Data to write
             * @return NFCStatus indicating success or failure
             */
            NFCStatus writeMifareClassic(uint8_t block, const std::vector<uint8_t>& data);

            /**
             * @brief Get URI prefix code
             * @param uri URI string
             * @return Prefix code (0 if no standard prefix)
             */
            uint8_t getURIPrefix(const std::string& uri);
    };

} // namespace NFC

#endif /* INC_NFC_CLASS_H */