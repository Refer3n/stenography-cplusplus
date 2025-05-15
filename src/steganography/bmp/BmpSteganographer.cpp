#include "steganography/bmp/BmpSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace {
    std::pair<int, int> readDimensions(std::ifstream& file) {
        file.seekg(18);
        if (file.fail()) {
            throw std::runtime_error("Failed to seek to BMP width/height fields.");
        }

        int32_t w = 0, h = 0;
        file.read(reinterpret_cast<char*>(&w), sizeof(w));
        file.read(reinterpret_cast<char*>(&h), sizeof(h));

        if (file.fail()) {
            throw std::runtime_error("Failed to read BMP dimensions.");
        }

        return {w, std::abs(h)};
    }
}

std::pair<int, int> BmpSteganographer::getImageDimensions(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    return readDimensions(file);
}

bool BmpSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open BMP file for encoding: " + filepath);
    }

    if (!canEncode(filepath, message)) {
        throw std::runtime_error("BMP image is too small to encode the message.");
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read BMP file contents.");
    }
    file.close();

    std::string bitMessage = Utils::textToBitString(message);
    std::bitset<32> messageLength(bitMessage.length());

    bitMessage = Utils::xorString(bitMessage, key);
    bitMessage = messageLength.to_string() + bitMessage;

    uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&buffer[10]);

    if (pixelDataOffset + bitMessage.size() > buffer.size()) {
        throw std::runtime_error("Encoded message exceeds BMP file size.");
    }

    for (size_t i = 0; i < bitMessage.size(); i++) {
        buffer[pixelDataOffset + i] = Utils::setLSB(static_cast<uint8_t>(buffer[pixelDataOffset + i]), bitMessage[i]);
    }

    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open BMP file for writing: " + filepath);
    }

    out.write(buffer.data(), buffer.size());
    out.close();

    return true;
}

std::string BmpSteganographer::decode(const std::string &filepath, const std::string &key) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open BMP file for decoding: " + filepath);
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read BMP file for decoding.");
    }
    file.close();

    uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&buffer[10]);

    if (pixelDataOffset + 32 > buffer.size()) {
        throw std::runtime_error("BMP file is corrupted or too small for a valid encoded message.");
    }

    std::string lengthBits;
    for (int i = 0; i < 32; i++) {
        auto bit = (buffer[pixelDataOffset + i] & 1) ? '1' : '0';
        lengthBits += bit;
    }

    auto length = std::bitset<32>(lengthBits).to_ulong();
    if (length == 0 || pixelDataOffset + 32 + length > buffer.size()) {
        throw std::runtime_error("Invalid or corrupted encoded message length.");
    }

    std::string messageBits;
    for (size_t i = 0; i < length; i++) {
        auto bit = (buffer[pixelDataOffset + 32 + i] & 1) ? '1' : '0';
        messageBits += bit;
    }

    messageBits = Utils::xorString(messageBits, key);
    return Utils::bitStringToText(messageBits);
}

bool BmpSteganographer::canEncode(const std::string &filepath, const std::string &message) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open BMP file: " + filepath);
    }

    auto [width, height] = readDimensions(file);
    file.close();

    auto availableBits = static_cast<long long>(width * height) * 3;
    auto messageBits = Utils::textToBitString(message).length();

    return availableBits >= messageBits + 32;
}