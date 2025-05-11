#include "steganography/png/PngSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <cstdint>
#include <fstream>
#include <vector>

namespace {
  size_t getIHDRChunkEndOffset(const std::vector<char> & buffer) {
    if (buffer.size() < 33) return std::string::npos;

    uint32_t ihdrLength = (uint8_t(buffer[8]) << 24) |
                          (uint8_t(buffer[9]) << 16) |
                          (uint8_t(buffer[10]) << 8) |
                           uint8_t(buffer[11]);

    std::string type(buffer.begin() + 12, buffer.begin() + 16);
    if (type != "IHDR") return std::string::npos;

    return 8 + 4 + 4 + ihdrLength + 4;
  }

  void appendUint32(std::vector<char>& out, uint32_t val) {
    out.push_back((val >> 24) & 0xFF);
    out.push_back((val >> 16) & 0xFF);
    out.push_back((val >> 8) & 0xFF);
    out.push_back(val & 0xFF);
  }

  std::vector<char> createPngChunk(const std::string &bitMessage) {
    std::string chunkType = "stEg";
    std::vector<char> chunk, payload;

    uint32_t bitLen = bitMessage.size();
    appendUint32(payload, bitLen);

    for (size_t i = 0; i < bitMessage.size(); i += 8) {
      std::bitset<8> bits(bitMessage.substr(i, 8));
      payload.push_back(static_cast<char>(bits.to_ulong()));
    }

    appendUint32(chunk, payload.size());
    chunk.insert(chunk.end(), chunkType.begin(), chunkType.end());
    chunk.insert(chunk.end(), payload.begin(), payload.end());

    std::vector<uint8_t> crcInput(chunk.begin() + 4, chunk.end());
    uint32_t crc = Utils::crc32(crcInput.data(), crcInput.size());
    appendUint32(chunk, crc);

    return chunk;
  }

}

bool PngSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) return false;

  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) return false;
  file.close();

  size_t insertPos = getIHDRChunkEndOffset(buffer);
  if (insertPos == std::string::npos) return false;

  std::string bitMessage = Utils::textToBitString(message);
  bitMessage = Utils::xorString(bitMessage, key);

  std::vector<char> chunk = createPngChunk(bitMessage);

  buffer.insert(buffer.begin() + insertPos, chunk.begin(), chunk.end());

  std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) return false;

  out.write(buffer.data(), buffer.size());
  out.close();

  return true;
}

std::string PngSteganographer::decode(const std::string &filepath, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) return "";

  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) return "";
  file.close();

  // Skip the 8-byte PNG signature
  size_t pos = 8;

  while (pos + 8 < buffer.size()) {
    // Read chunk length
    uint32_t length =
        (uint8_t(buffer[pos]) << 24) |
        (uint8_t(buffer[pos + 1]) << 16) |
        (uint8_t(buffer[pos + 2]) << 8) |
        uint8_t(buffer[pos + 3]);

    std::string type(buffer.begin() + pos + 4, buffer.begin() + pos + 8);

    if (type == "stEg") {
      size_t dataStart = pos + 8;
      if (dataStart + length > buffer.size()) return "";

      const uint8_t* data = reinterpret_cast<const uint8_t*>(&buffer[dataStart]);

      uint32_t bitLen =
          (data[0] << 24) | (data[1] << 16) |
          (data[2] << 8)  | (data[3]);

      std::string bitMessage;
      for (size_t i = 4; i < length; ++i) {
        std::bitset<8> byteBits(data[i]);
        bitMessage += byteBits.to_string();
      }

      if (bitMessage.size() < bitLen) return "";
      bitMessage = bitMessage.substr(0, bitLen);

      // Decrypt and convert
      bitMessage = Utils::xorString(bitMessage, key);
      return Utils::bitStringToText(bitMessage);
    }

    pos += 8 + length + 4;
  }

  return ""; // Chunk not found
}



bool PngSteganographer::canEncode(const std::string &filepath, const std::string &message) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) return false;

  char firstByte;
  file.read(&firstByte, 1);
  if (firstByte != '\x89')
    return false;

  constexpr size_t maxMessageSize = 1024 * 1024;
  return message.size() <= maxMessageSize;
}
