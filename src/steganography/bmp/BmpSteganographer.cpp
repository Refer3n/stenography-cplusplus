#include "steganography/bmp/BmpSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace {
    auto readDimensions(std::ifstream& file) -> std::pair<int, int> {
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

auto BmpSteganographer::getImageDimensions(const std::string& filepath) -> std::pair<int, int> {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    return readDimensions(file);
}

auto BmpSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) -> bool {
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

    uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&buffer[10]);
    
    return encodeLSB(buffer, pixelDataOffset, message, key, filepath);
}

auto BmpSteganographer::decode(const std::string &filepath, const std::string &key) -> std::string {
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
    
    return decodeLSB(buffer, pixelDataOffset, key);
}

auto BmpSteganographer::canEncode(const std::string &filepath, const std::string &message) -> bool {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open BMP file: " + filepath);
    }

    auto [width, height] = readDimensions(file);
    file.close();

    return canEncodeWithDimensions(width, height, message);
}