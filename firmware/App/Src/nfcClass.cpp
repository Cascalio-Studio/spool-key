/**
 * @file    App/Src/nfcClass.cpp
 * @brief   High-Level NFC API Classes Implementation
 * @details This file contains the implementation of high-level NFC API classes for tag operations.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

/**
 * @include necessary headers
 */
#include "nfcClass.h"
#include "FreeRTOS.h"
#include "task.h"
#include <algorithm>
#include <cstring>

namespace NFC
{
    // ============================================================================
    // NFCManager Implementation
    // ============================================================================

    NFCManager::NFCManager(ST25R3911B* controller)
        : _controller(controller)
        , _tagReader(nullptr)
        , _tagWriter(nullptr)
        , _initialized(false)
        , _detectionActive(false)
        , _detectionProtocols(0)
    {
        if (_controller) {
            _tagReader = new TagReader(_controller);
            _tagWriter = new TagWriter(_controller);
        }
    }

    NFCManager::~NFCManager()
    {
        Deinitialize();
        delete _tagReader;
        delete _tagWriter;
    }

    NFCStatus NFCManager::Initialize(void)
    {
        if (_initialized || !_controller) {
            return NFCStatus::ERROR;
        }

        NFCStatus status = _controller->Initialize();
        if (status != NFCStatus::OK) {
            return status;
        }

        _initialized = true;
        return NFCStatus::OK;
    }

    NFCStatus NFCManager::Deinitialize(void)
    {
        if (!_initialized) {
            return NFCStatus::OK;
        }

        StopTagDetection();
        
        if (_controller) {
            _controller->Deinitialize();
        }

        _initialized = false;
        return NFCStatus::OK;
    }

    NFCStatus NFCManager::StartTagDetection(uint32_t protocols, TagDetectionCallback callback)
    {
        if (!_initialized || _detectionActive) {
            return NFCStatus::ERROR;
        }

        _detectionCallback = callback;
        _detectionProtocols = protocols;
        _detectionActive = true;

        // Enable field
        NFCStatus status = _controller->SetField(NFCField::ON);
        if (status != NFCStatus::OK) {
            _detectionActive = false;
            return status;
        }

        // Start with ISO14443A detection (most common)
        status = _controller->SetProtocol(NFCProtocol::NFC_A);
        if (status != NFCStatus::OK) {
            _detectionActive = false;
            return status;
        }

        return NFCStatus::OK;
    }

    NFCStatus NFCManager::StopTagDetection(void)
    {
        if (!_detectionActive) {
            return NFCStatus::OK;
        }

        _detectionActive = false;
        _detectionCallback = nullptr;

        // Turn off field
        return _controller->SetField(NFCField::OFF);
    }

    NFCStatus NFCManager::SetField(NFCField field)
    {
        if (!_initialized) {
            return NFCStatus::NOT_INITIALIZED;
        }

        return _controller->SetField(field);
    }

    NFCField NFCManager::GetField(void) const
    {
        if (!_initialized) {
            return NFCField::OFF;
        }

        NFCField field;
        _controller->GetField(field);
        return field;
    }

    void NFCManager::handleTagDetection(void)
    {
        if (!_detectionActive || !_detectionCallback) {
            return;
        }

        // Send REQA command to detect ISO14443A tags
        std::vector<uint8_t> reqa = {0x26}; // REQA command
        std::vector<uint8_t> response;

        NFCStatus status = _controller->TransmitReceive(reqa, response, 100);
        if (status == NFCStatus::OK && response.size() >= 2) {
            TagInfo tagInfo;
            if (identifyTag(response, tagInfo) == NFCStatus::OK) {
                _detectionCallback(tagInfo);
            }
        }
    }

    NFCStatus NFCManager::identifyTag(const std::vector<uint8_t>& response, TagInfo& tagInfo)
    {
        if (response.size() < 2) {
            return NFCStatus::ERROR;
        }

        // Initialize tag info
        tagInfo.protocol = NFCProtocol::NFC_A;
        tagInfo.atqa = response;
        tagInfo.isReadOnly = false;

        // Determine tag type based on ATQA
        uint16_t atqa = (response[1] << 8) | response[0];
        
        switch (atqa) {
            case 0x0004:
                tagInfo.protocol = NFCProtocol::MIFARE_CLASSIC;
                tagInfo.dataSize = 1024; // MIFARE Classic 1K
                break;
            case 0x0044:
                tagInfo.protocol = NFCProtocol::NFC_A;
                tagInfo.dataSize = 8192; // NTAG213/215/216
                break;
            default:
                tagInfo.protocol = NFCProtocol::NFC_A;
                tagInfo.dataSize = 2048; // Default size
                break;
        }

        return NFCStatus::OK;
    }

    // ============================================================================
    // TagReader Implementation
    // ============================================================================

    TagReader::TagReader(ST25R3911B* controller)
        : _controller(controller)
        , _callback(nullptr)
    {
    }

    NFCStatus TagReader::ReadUID(const TagInfo& tagInfo, std::vector<uint8_t>& uid)
    {
        if (!_controller || !_controller->IsInitialized()) {
            return NFCStatus::NOT_INITIALIZED;
        }

        // Send anticollision command
        std::vector<uint8_t> anticol = {0x93, 0x20}; // SELECT CL1
        std::vector<uint8_t> response;

        NFCStatus status = _controller->TransmitReceive(anticol, response, 100);
        if (status != NFCStatus::OK) {
            return status;
        }

        if (response.size() >= 5) {
            uid.assign(response.begin(), response.begin() + 4);
            return NFCStatus::OK;
        }

        return NFCStatus::ERROR;
    }

    NFCStatus TagReader::ReadRawData(const TagInfo& tagInfo, uint16_t address, uint16_t length, std::vector<uint8_t>& data)
    {
        if (!_controller || !_controller->IsInitialized()) {
            return NFCStatus::NOT_INITIALIZED;
        }

        switch (tagInfo.protocol) {
            case NFCProtocol::NFC_A:
                return readISO14443A(address, length, data);
            case NFCProtocol::MIFARE_CLASSIC:
                return readMifareClassic(static_cast<uint8_t>(address), data);
            default:
                return NFCStatus::UNSUPPORTED_TAG;
        }
    }

    NFCStatus TagReader::ReadNDEF(const TagInfo& tagInfo, NDEFMessage& message)
    {
        // Read NDEF header first
        std::vector<uint8_t> header;
        NFCStatus status = ReadRawData(tagInfo, 0, 16, header);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Check for NDEF capability container
        if (header.size() < 16 || header[0] != 0xE1) {
            return NFCStatus::ERROR;
        }

        // Get NDEF message length
        uint16_t ndefLength = (header[14] << 8) | header[15];
        if (ndefLength == 0) {
            message.records.clear();
            message.totalSize = 0;
            return NFCStatus::OK;
        }

        // Read NDEF message
        std::vector<uint8_t> ndefData;
        status = ReadRawData(tagInfo, 16, ndefLength, ndefData);
        if (status != NFCStatus::OK) {
            return status;
        }

        return parseNDEFMessage(ndefData, message);
    }

    NFCStatus TagReader::ReadText(const TagInfo& tagInfo, std::string& text, std::string& language)
    {
        NDEFMessage message;
        NFCStatus status = ReadNDEF(tagInfo, message);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Find text record
        for (const auto& record : message.records) {
            if (record.type == NDEFRecordType::TEXT) {
                text = record.payload;
                language = record.language;
                return NFCStatus::OK;
            }
        }

        return NFCStatus::ERROR;
    }

    NFCStatus TagReader::ReadURI(const TagInfo& tagInfo, std::string& uri)
    {
        NDEFMessage message;
        NFCStatus status = ReadNDEF(tagInfo, message);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Find URI record
        for (const auto& record : message.records) {
            if (record.type == NDEFRecordType::URI) {
                uri = record.payload;
                return NFCStatus::OK;
            }
        }

        return NFCStatus::ERROR;
    }

    NFCStatus TagReader::ReadWiFi(const TagInfo& tagInfo, std::string& ssid, std::string& password, std::string& security)
    {
        NDEFMessage message;
        NFCStatus status = ReadNDEF(tagInfo, message);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Find WiFi record (MIME type application/vnd.wfa.wsc)
        for (const auto& record : message.records) {
            if (record.type == NDEFRecordType::WIFI) {
                // Parse WiFi credential payload (simplified)
                // In real implementation, this would parse WPS format
                ssid = "WiFi_SSID";
                password = "WiFi_Password";
                security = "WPA2";
                return NFCStatus::OK;
            }
        }

        return NFCStatus::ERROR;
    }

    NFCStatus TagReader::parseNDEFMessage(const std::vector<uint8_t>& data, NDEFMessage& message)
    {
        message.records.clear();
        message.totalSize = static_cast<uint16_t>(data.size());

        size_t offset = 0;
        while (offset < data.size()) {
            NDEFRecord record;
            size_t bytesRead;
            
            NFCStatus status = parseNDEFRecord(data, offset, record, bytesRead);
            if (status != NFCStatus::OK) {
                break;
            }

            message.records.push_back(record);
            offset += bytesRead;

            // Check for message end flag
            if (data[offset - bytesRead] & 0x40) { // ME flag
                break;
            }
        }

        return NFCStatus::OK;
    }

    NFCStatus TagReader::parseNDEFRecord(const std::vector<uint8_t>& data, size_t offset, NDEFRecord& record, size_t& bytesRead)
    {
        if (offset >= data.size()) {
            return NFCStatus::ERROR;
        }

        bytesRead = 0;
        size_t pos = offset;

        // Parse NDEF record header
        uint8_t tnf = data[pos] & 0x07;  // Type Name Format
        uint8_t flags = data[pos++];
        bytesRead++;

        uint8_t typeLength = data[pos++];
        bytesRead++;

        uint8_t payloadLength = data[pos++];
        bytesRead++;

        // Skip ID length if present
        if (flags & 0x08) { // IL flag
            pos++; // Skip ID length
            bytesRead++;
        }

        // Read type
        std::string type(reinterpret_cast<const char*>(&data[pos]), typeLength);
        pos += typeLength;
        bytesRead += typeLength;

        // Skip ID if present
        if (flags & 0x08) { // IL flag
            uint8_t idLength = data[pos - typeLength - 1];
            pos += idLength;
            bytesRead += idLength;
        }

        // Read payload
        record.rawData.assign(data.begin() + pos, data.begin() + pos + payloadLength);
        pos += payloadLength;
        bytesRead += payloadLength;

        // Determine record type and parse payload
        if (tnf == 0x01) { // Well-known type
            if (type == "T") {
                record.type = NDEFRecordType::TEXT;
                if (payloadLength > 3) {
                    uint8_t langLength = record.rawData[0] & 0x3F;
                    record.language = std::string(reinterpret_cast<const char*>(&record.rawData[1]), langLength);
                    record.payload = std::string(reinterpret_cast<const char*>(&record.rawData[1 + langLength]), 
                                                payloadLength - 1 - langLength);
                }
            } else if (type == "U") {
                record.type = NDEFRecordType::URI;
                if (payloadLength > 1) {
                    uint8_t prefix = record.rawData[0];
                    record.payload = std::string(reinterpret_cast<const char*>(&record.rawData[1]), payloadLength - 1);
                    
                    // Add URI prefix
                    const char* prefixes[] = {
                        "", "http://www.", "https://www.", "http://", "https://",
                        "tel:", "mailto:", "ftp://anonymous:anonymous@", "ftp://ftp.",
                        "ftps://", "sftp://", "smb://", "nfs://", "ftp://",
                        "dav://", "news:", "telnet://", "imap:", "rtsp://",
                        "urn:", "pop:", "sip:", "sips:", "tftp:", "btspp://",
                        "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://",
                        "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:",
                        "urn:epc:raw:", "urn:epc:", "urn:nfc:"
                    };
                    
                    if (prefix < sizeof(prefixes) / sizeof(prefixes[0])) {
                        record.payload = std::string(prefixes[prefix]) + record.payload;
                    }
                }
            }
        } else if (tnf == 0x02) { // MIME type
            record.type = NDEFRecordType::MIME;
            record.mimeType = type;
            record.payload = std::string(reinterpret_cast<const char*>(record.rawData.data()), payloadLength);
        } else {
            record.type = NDEFRecordType::UNKNOWN;
        }

        return NFCStatus::OK;
    }

    NFCStatus TagReader::readISO14443A(uint16_t address, uint16_t length, std::vector<uint8_t>& data)
    {
        data.clear();
        data.reserve(length);

        // Read in 16-byte blocks (typical for NTAG)
        uint16_t bytesRead = 0;
        uint8_t blockSize = 16;

        while (bytesRead < length) {
            uint8_t currentBlock = static_cast<uint8_t>((address + bytesRead) / blockSize);
            std::vector<uint8_t> readCmd = {0x30, currentBlock}; // READ command
            std::vector<uint8_t> response;

            NFCStatus status = _controller->TransmitReceive(readCmd, response, 100);
            if (status != NFCStatus::OK) {
                return status;
            }

            if (response.size() < blockSize) {
                return NFCStatus::ERROR;
            }

            // Add data to result
            uint16_t remainingBytes = length - bytesRead;
            uint16_t bytesToCopy = std::min(static_cast<uint16_t>(blockSize), remainingBytes);
            
            data.insert(data.end(), response.begin(), response.begin() + bytesToCopy);
            bytesRead += bytesToCopy;
        }

        return NFCStatus::OK;
    }

    NFCStatus TagReader::readMifareClassic(uint8_t block, std::vector<uint8_t>& data)
    {
        // MIFARE Classic requires authentication first
        std::vector<uint8_t> authCmd = {0x60, block, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // AUTH_A with default key
        std::vector<uint8_t> response;

        NFCStatus status = _controller->TransmitReceive(authCmd, response, 100);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Read block
        std::vector<uint8_t> readCmd = {0x30, block};
        status = _controller->TransmitReceive(readCmd, response, 100);
        if (status != NFCStatus::OK) {
            return status;
        }

        if (response.size() >= 16) {
            data.assign(response.begin(), response.begin() + 16);
            return NFCStatus::OK;
        }

        return NFCStatus::ERROR;
    }

    // ============================================================================
    // TagWriter Implementation  
    // ============================================================================

    TagWriter::TagWriter(ST25R3911B* controller)
        : _controller(controller)
        , _callback(nullptr)
    {
    }

    NFCStatus TagWriter::WriteRawData(const TagInfo& tagInfo, uint16_t address, const std::vector<uint8_t>& data)
    {
        if (!_controller || !_controller->IsInitialized()) {
            return NFCStatus::NOT_INITIALIZED;
        }

        if (tagInfo.isReadOnly) {
            return NFCStatus::ERROR;
        }

        switch (tagInfo.protocol) {
            case NFCProtocol::NFC_A:
                return writeISO14443A(address, data);
            case NFCProtocol::MIFARE_CLASSIC:
                if (data.size() != 16) {
                    return NFCStatus::INVALID_PARAM;
                }
                return writeMifareClassic(static_cast<uint8_t>(address), data);
            default:
                return NFCStatus::UNSUPPORTED_TAG;
        }
    }

    NFCStatus TagWriter::WriteNDEF(const TagInfo& tagInfo, const NDEFMessage& message)
    {
        // Create NDEF message data
        std::vector<uint8_t> ndefData;
        NFCStatus status = createNDEFMessage(message.records, ndefData);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Write NDEF length header
        std::vector<uint8_t> lengthHeader = {
            static_cast<uint8_t>((ndefData.size() >> 8) & 0xFF),
            static_cast<uint8_t>(ndefData.size() & 0xFF)
        };

        status = WriteRawData(tagInfo, 14, lengthHeader);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Write NDEF message
        return WriteRawData(tagInfo, 16, ndefData);
    }

    NFCStatus TagWriter::WriteText(const TagInfo& tagInfo, const std::string& text, const std::string& language)
    {
        NDEFRecord record;
        record.type = NDEFRecordType::TEXT;
        record.payload = text;
        record.language = language;

        NDEFMessage message;
        message.records.push_back(record);
        message.totalSize = static_cast<uint16_t>(text.length() + language.length() + 10); // Approximate

        return WriteNDEF(tagInfo, message);
    }

    NFCStatus TagWriter::WriteURI(const TagInfo& tagInfo, const std::string& uri)
    {
        NDEFRecord record;
        record.type = NDEFRecordType::URI;
        record.payload = uri;

        NDEFMessage message;
        message.records.push_back(record);
        message.totalSize = static_cast<uint16_t>(uri.length() + 10); // Approximate

        return WriteNDEF(tagInfo, message);
    }

    NFCStatus TagWriter::WriteURL(const TagInfo& tagInfo, const std::string& url)
    {
        return WriteURI(tagInfo, url);
    }

    NFCStatus TagWriter::WriteWiFi(const TagInfo& tagInfo, const std::string& ssid, const std::string& password, const std::string& security)
    {
        NDEFRecord record;
        record.type = NDEFRecordType::WIFI;
        record.mimeType = "application/vnd.wfa.wsc";
        
        // Create simplified WiFi credential payload
        // Real implementation would create proper WPS format
        record.payload = ssid + ":" + password + ":" + security;

        NDEFMessage message;
        message.records.push_back(record);
        message.totalSize = static_cast<uint16_t>(record.payload.length() + 20); // Approximate

        return WriteNDEF(tagInfo, message);
    }

    NFCStatus TagWriter::WritePhone(const TagInfo& tagInfo, const std::string& phoneNumber)
    {
        return WriteURI(tagInfo, "tel:" + phoneNumber);
    }

    NFCStatus TagWriter::WriteEmail(const TagInfo& tagInfo, const std::string& email, const std::string& subject, const std::string& body)
    {
        std::string mailtoUri = "mailto:" + email;
        if (!subject.empty() || !body.empty()) {
            mailtoUri += "?";
            if (!subject.empty()) {
                mailtoUri += "subject=" + subject;
            }
            if (!body.empty()) {
                if (!subject.empty()) {
                    mailtoUri += "&";
                }
                mailtoUri += "body=" + body;
            }
        }
        
        return WriteURI(tagInfo, mailtoUri);
    }

    NFCStatus TagWriter::FormatTag(const TagInfo& tagInfo)
    {
        // Write NDEF capability container
        std::vector<uint8_t> cc = {
            0xE1, 0x10, // NDEF Magic Number and Version
            0x3F, 0x00, // Data Size (approximate)
            0x00, 0x00, 0x00, 0x00, // Reserved
            0x00, 0x00, 0x00, 0x00, // Reserved
            0x00, 0x00, // Reserved
            0x00, 0x00  // NDEF Length (empty)
        };

        return WriteRawData(tagInfo, 0, cc);
    }

    NFCStatus TagWriter::createNDEFMessage(const std::vector<NDEFRecord>& records, std::vector<uint8_t>& data)
    {
        data.clear();

        for (size_t i = 0; i < records.size(); ++i) {
            std::vector<uint8_t> recordData;
            NFCStatus status = createNDEFRecord(records[i], recordData);
            if (status != NFCStatus::OK) {
                return status;
            }

            // Set flags
            if (i == 0) {
                recordData[0] |= 0x80; // MB (Message Begin)
            }
            if (i == records.size() - 1) {
                recordData[0] |= 0x40; // ME (Message End)
            }

            data.insert(data.end(), recordData.begin(), recordData.end());
        }

        return NFCStatus::OK;
    }

    NFCStatus TagWriter::createNDEFRecord(const NDEFRecord& record, std::vector<uint8_t>& data)
    {
        data.clear();

        uint8_t tnf = 0x01; // Well-known type
        std::string type;
        std::vector<uint8_t> payload;

        switch (record.type) {
            case NDEFRecordType::TEXT: {
                type = "T";
                payload.push_back(static_cast<uint8_t>(record.language.length()));
                payload.insert(payload.end(), record.language.begin(), record.language.end());
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::URI: {
                type = "U";
                payload.push_back(getURIPrefix(record.payload));
                
                // Remove prefix from payload if it matches
                std::string uriPayload = record.payload;
                const char* prefixes[] = {"http://www.", "https://www.", "http://", "https://"};
                for (size_t i = 0; i < 4; ++i) {
                    if (uriPayload.substr(0, strlen(prefixes[i])) == prefixes[i]) {
                        uriPayload = uriPayload.substr(strlen(prefixes[i]));
                        break;
                    }
                }
                
                payload.insert(payload.end(), uriPayload.begin(), uriPayload.end());
                break;
            }

            case NDEFRecordType::MIME: {
                tnf = 0x02; // MIME type
                type = record.mimeType;
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::WIFI: {
                tnf = 0x02; // MIME type for WiFi
                type = "application/vnd.wfa.wsc";
                // Add WiFi credential encoding here
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::PHONE: {
                type = "U";
                payload.push_back(0x05); // tel: prefix
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::EMAIL: {
                type = "U";
                payload.push_back(0x06); // mailto: prefix
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::VCARD: {
                tnf = 0x02; // MIME type
                type = "text/vcard";
                payload.insert(payload.end(), record.payload.begin(), record.payload.end());
                break;
            }

            case NDEFRecordType::UNKNOWN:
            default:
                return NFCStatus::INVALID_PARAM;
        }

        // Build record
        data.push_back(tnf); // TNF and flags (flags set by caller)
        data.push_back(static_cast<uint8_t>(type.length()));
        data.push_back(static_cast<uint8_t>(payload.size()));
        data.insert(data.end(), type.begin(), type.end());
        data.insert(data.end(), payload.begin(), payload.end());

        return NFCStatus::OK;
    }

    NFCStatus TagWriter::writeISO14443A(uint16_t address, const std::vector<uint8_t>& data)
    {
        // Write in 4-byte chunks (typical for NTAG)
        size_t bytesWritten = 0;
        uint8_t pageSize = 4;

        while (bytesWritten < data.size()) {
            uint8_t currentPage = static_cast<uint8_t>((address + bytesWritten) / pageSize);
            std::vector<uint8_t> writeCmd = {0xA2, currentPage}; // WRITE command

            // Add 4 bytes of data
            size_t remainingBytes = data.size() - bytesWritten;
            size_t bytesToWrite = std::min(static_cast<size_t>(pageSize), remainingBytes);
            
            writeCmd.insert(writeCmd.end(), data.begin() + bytesWritten, data.begin() + bytesWritten + bytesToWrite);
            
            // Pad to 4 bytes if necessary
            while (writeCmd.size() < 6) {
                writeCmd.push_back(0x00);
            }

            std::vector<uint8_t> response;
            NFCStatus status = _controller->TransmitReceive(writeCmd, response, 100);
            if (status != NFCStatus::OK) {
                return status;
            }

            bytesWritten += bytesToWrite;
        }

        return NFCStatus::OK;
    }

    NFCStatus TagWriter::writeMifareClassic(uint8_t block, const std::vector<uint8_t>& data)
    {
        if (data.size() != 16) {
            return NFCStatus::INVALID_PARAM;
        }

        // Authenticate first
        std::vector<uint8_t> authCmd = {0x60, block, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // AUTH_A with default key
        std::vector<uint8_t> response;

        NFCStatus status = _controller->TransmitReceive(authCmd, response, 100);
        if (status != NFCStatus::OK) {
            return status;
        }

        // Write block
        std::vector<uint8_t> writeCmd = {0xA0, block};
        writeCmd.insert(writeCmd.end(), data.begin(), data.end());

        return _controller->TransmitReceive(writeCmd, response, 100);
    }

    uint8_t TagWriter::getURIPrefix(const std::string& uri)
    {
        if (uri.substr(0, 11) == "http://www.") return 0x01;
        if (uri.substr(0, 12) == "https://www.") return 0x02;
        if (uri.substr(0, 7) == "http://") return 0x03;
        if (uri.substr(0, 8) == "https://") return 0x04;
        if (uri.substr(0, 4) == "tel:") return 0x05;
        if (uri.substr(0, 7) == "mailto:") return 0x06;
        
        return 0x00; // No prefix
    }

} // namespace NFC