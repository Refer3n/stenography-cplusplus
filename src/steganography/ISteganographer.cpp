#include "steganography/ISteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <fstream>
#include <stdexcept>

auto ISteganographer::encodeLSB(std::vector<char>& buffer, size_t pixelDataOffset, const std::string& message,
    const std::string& key, const std::string& filepath) -> bool {
    std::string bitMessage = Utils::textToBitString(message);
    std::bitset<32> messageLength(bitMessage.length());

    bitMessage = Utils::xorString(bitMessage, key);
    bitMessage = messageLength.to_string() + bitMessage;

    if (pixelDataOffset + bitMessage.size() > buffer.size()) {
        throw std::runtime_error("Message too long to encode in this image.");
    }

    for (size_t i = 0; i < bitMessage.size(); i++) {
        buffer[pixelDataOffset + i] = Utils::setLSB(static_cast<uint8_t>(buffer[pixelDataOffset + i]), bitMessage[i]);
    }

    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }

    out.write(buffer.data(), buffer.size());
    out.close();

    return true;
}

auto ISteganographer::decodeLSB(const std::vector<char>& buffer, size_t pixelDataOffset, const std::string& key) -> std::string {
    if (pixelDataOffset + 32 > buffer.size()) {
        throw std::runtime_error("File is corrupted or too small for a valid encoded message.");
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

auto ISteganographer::canEncodeWithDimensions(int width, int height, const std::string& message) -> bool {
    auto availableBits = static_cast<long long>(width * height) * 3;
    auto messageBits = Utils::textToBitString(message).length();

    return availableBits >= messageBits + 32;
}
