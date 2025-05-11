#include "steganography/bmp/BmpSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
  bool getImageDimensions(const std::string& filepath, int& width, int& height) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.seekg(18);

    int32_t w = 0, h = 0;
    file.read(reinterpret_cast<char*>(&w), sizeof(w));
    file.read(reinterpret_cast<char*>(&h), sizeof(h));

    if (file.fail()) return false;

    width = w;
    height = std::abs(h);

    return true;
  }
}

bool BmpSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) return false;

  if (!canEncode(filepath, message)) return false;

  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) return false;
  file.close();

  std::string bitMessage = Utils::textToBitString(message);
  std::bitset<32> messageLength(bitMessage.length());

  bitMessage = Utils::xorString(bitMessage, key);
  bitMessage = messageLength.to_string() + bitMessage;

  uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&buffer[10]);

  for (auto i = 0; i < bitMessage.size(); i++) {
    buffer[pixelDataOffset + i] = Utils::setLSB(static_cast<uint8_t>(buffer[pixelDataOffset + i]), bitMessage[i]);
  }

  std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) return false;

  out.write(buffer.data(), buffer.size());
  out.close();

  return true;
}

std::string BmpSteganographer::decode(const std::string &filepath, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) return "";

  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) return "";
  file.close();

  uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&buffer[10]);

  std::string lengthBits;
  for (auto i = 0; i < 32; i++) {
    auto bit = (buffer[pixelDataOffset + i] & 1) ? '1' : '0';
    lengthBits += bit;
  }

  auto length = std::bitset<32>(lengthBits).to_ulong();
  if (length == 0) return "";

  std::string messageBits;

  for (auto i = 0; i < length; i++) {
    auto bit = (buffer[pixelDataOffset + 32 + i] & 1) ? '1' : '0';
    messageBits += bit;
  }

  messageBits = Utils::xorString(messageBits, key);

  return Utils::bitStringToText(messageBits);
}

bool BmpSteganographer::canEncode(const std::string &filepath, const std::string &message) {
  int width = 0;
  int height = 0;

  if (!getImageDimensions(filepath, width, height)) return false;

  auto availableBits = static_cast<long long>(width * height) * 3;
  auto messageBits = Utils::textToBitString(message).length();

  return availableBits >= messageBits + 32;
}
