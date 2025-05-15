#include "steganography/ppm/PpmSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <fstream>
#include <vector>
#include <stdexcept>

namespace {
    int calculateOffset(const std::vector<char>& buffer) {
        int lineCount = 0;

        for (int i = 0; i < buffer.size(); i++) {
            if (buffer[i] == '\n') {
                lineCount++;
            }
            if (lineCount == 4) {
                return i + 1;
            }
        }

        return -1;
    }

    std::pair<int, int> readDimensions(std::ifstream& file) {
        std::string token;
        std::vector<std::string> header;

        while (header.size() < 3 && file >> token) {
            if (token.starts_with('#')) {
                std::getline(file, token);
                continue;
            }

            header.push_back(token);
        }

        if (header.size() < 3 || header[0] != "P6") {
            throw std::runtime_error("Invalid or unsupported PPM format.");
        }

        int width = std::stoi(header[1]);
        int height = std::stoi(header[2]);

        return {width, height};
    }
}

auto PpmSteganographer::getImageDimensions(const std::string& filepath) -> std::pair<int, int> {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    return readDimensions(file);
}


bool PpmSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for encoding: " + filepath);
    }

    if (!canEncode(filepath, message)) {
        throw std::runtime_error("Image is not suitable for encoding. Not enough space or unsupported format.");
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();

    int pixelDataOffset = calculateOffset(buffer);
    if (pixelDataOffset < 0) {
        throw std::runtime_error("Failed to calculate pixel data offset.");
    }

    std::string bitMessage = Utils::textToBitString(message);
    std::bitset<32> messageLength(bitMessage.length());

    bitMessage = Utils::xorString(bitMessage, key);
    bitMessage = messageLength.to_string() + bitMessage;

    if ((pixelDataOffset + bitMessage.size()) > buffer.size()) {
        throw std::runtime_error("Message too long to encode in this image.");
    }

    for (int i = 0; i < bitMessage.size(); i++) {
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

std::string PpmSteganographer::decode(const std::string &filepath, const std::string &key) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for decoding: " + filepath);
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();

    int pixelDataOffset = calculateOffset(buffer);
    if (pixelDataOffset == -1) {
        throw std::runtime_error("Could not find start of pixel data.");
    }

    std::string lengthBits;
    for (int i = 0; i < 32; i++) {
        auto bit = (buffer[pixelDataOffset + i] & 1) ? '1' : '0';
        lengthBits += bit;
    }

    auto length = std::bitset<32>(lengthBits).to_ulong();

    std::string messageBits;
    for (int i = 0; i < length; i++) {
        auto bit = (buffer[pixelDataOffset + 32 + i] & 1) ? '1' : '0';
        messageBits += bit;
    }

    messageBits = Utils::xorString(messageBits, key);

    return Utils::bitStringToText(messageBits);
}

bool PpmSteganographer::canEncode(const std::string &filepath, const std::string &message) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for capability check: " + filepath);
    }

    auto [width, height] = readDimensions(file);
    auto availableBits = width * height * 3;
    auto messageBits = Utils::textToBitString(message).length();

    return availableBits >= (messageBits + 32);
}