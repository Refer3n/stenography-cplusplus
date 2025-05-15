#include "steganography/ppm/PpmSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <fstream>
#include <vector>
#include <stdexcept>

namespace {
    auto calculateOffset(const std::vector<char>& buffer) -> int {
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

    auto readDimensions(std::ifstream& file) -> std::pair<int, int> {
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


auto PpmSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) -> bool {
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

    return encodeLSB(buffer, pixelDataOffset, message, key, filepath);
}

auto PpmSteganographer::decode(const std::string &filepath, const std::string &key) -> std::string {
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

    return decodeLSB(buffer, pixelDataOffset, key);
}

auto PpmSteganographer::canEncode(const std::string &filepath, const std::string &message) -> bool {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for capability check: " + filepath);
    }

    auto [width, height] = readDimensions(file);
    
    return canEncodeWithDimensions(width, height, message);
}