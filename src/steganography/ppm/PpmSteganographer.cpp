#include "steganography/ppm/PpmSteganographer.h"
#include "steganography/Utils.h"

#include <bitset>
#include <fstream>
#include <vector>

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
}

bool PpmSteganographer::encode(const std::string &filepath, const std::string &message, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) return false;

  if (!canEncode(filepath, message)) return false;

  std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
  file.close();

  int pixelDataOffset = calculateOffset(buffer);

  std::string bitMessage = Utils::textToBitString(message);
  std::bitset<32> messageLength(bitMessage.length());

  bitMessage = Utils::xorString(bitMessage, key);
  bitMessage = messageLength.to_string() + bitMessage;

  if (pixelDataOffset < 0 || (pixelDataOffset + bitMessage.size()) > buffer.size()) {
    return false;
  }

  for ( int i = 0; i < bitMessage.size(); i++) {
    buffer[pixelDataOffset + i] = Utils::setLSB(static_cast<uint8_t>(buffer[pixelDataOffset + i]), bitMessage[i]);
  }

  std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) return false;

  out.write(buffer.data(), buffer.size());
  out.close();

  return true;
}

std::string PpmSteganographer::decode(const std::string &filepath, const std::string &key) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) return "";

  std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
  file.close();

  int pixelDataOffset = calculateOffset(buffer);
  if (pixelDataOffset == -1) return "";

  std::string lengthBits;
  for (int i = 0; i < 32; i++ ) {
    auto bit = (buffer[pixelDataOffset + i] & 1) ? '1' : '0';
    lengthBits += bit;
  }

  auto length = std::bitset<32>(lengthBits).to_ulong();

  std::string messageBits;
  for ( int i = 0; i < length; i++ ) {
    auto bit = (buffer[pixelDataOffset + 32 + i] & 1) ? '1' : '0';
    messageBits += bit;
  }

  messageBits = Utils::xorString(messageBits, key);

  return Utils::bitStringToText(messageBits);
}

bool PpmSteganographer::canEncode(const std::string &filepath, const std::string &message) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) return false;

  std::string line;
  std::vector<std::string> header;

  while ( header.size() < 3 && file >> line ) {
    if (line.starts_with('#')) {
      std::getline(file, line);
      continue;
    }

    header.push_back(line);
  }

  if (header[0] != "P6" || header.size() < 3) return false;

  auto width = std::stoi(header[1]);
  auto height = std::stoi(header[2]);


  auto availableBits = width * height * 3;
  auto messageBits = Utils::textToBitString(message).length();

  return availableBits >= messageBits + 32;
}

