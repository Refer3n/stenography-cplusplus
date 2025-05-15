#include "steganography/Utils.h"

#include <bitset>
#include <filesystem>
#include <fmt/ostream.h>

namespace Utils {
  ImageFormat getImageFormat(const std::string &filename) {
    auto ext = filename.substr(filename.find_last_of(".") + 1);

    if (ext == "bmp") return ImageFormat::BMP;
    if (ext == "ppm") return ImageFormat::PPM;
    return ImageFormat::NOT_SUPPORTED;
  }

  std::string getImageFormatName(const ImageFormat format) {
    switch (format) {
      case ImageFormat::BMP: return "BMP";
      case ImageFormat::PPM: return "PPM";
      default: return "NOT_SUPPORTED";
    }
  }

  auto getImageInfo(const std::string &filePath, const std::pair<int, int> &dimensions) -> std::string {
    try {
      if (!std::filesystem::exists(filePath)) {
        return "Error: File does not exist.";
      }

      auto fileSize = std::filesystem::file_size(filePath);
      auto lastWriteTime = std::filesystem::last_write_time(filePath);
      auto lastWriteTimeSystem = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(lastWriteTime));

      auto format = getImageFormat(filePath);
      auto formatStr = getImageFormatName(format);

      std::ostringstream infoStream;
      infoStream << "File Info for: " << filePath << "\n"
                 << "Size: " << fileSize / 1024 << " KB\n"
                 << "Last Modified: " << std::ctime(&lastWriteTimeSystem)
                 << "Format: " << formatStr << "\n"
                 << "Dimensions: " << dimensions.first << " x " << dimensions.second << "\n";

      return infoStream.str();
    } catch (const std::exception& e) {
      return std::string("Error: ") + e.what();
    }
  }

  bool hasWritePermission(const std::string &filepath) {
    std::ofstream file(filepath, std::ios::app);
    return file.is_open();
  }

  std::string textToBitString(const std::string &message) {
    std::string result;

    for (char c : message) {
      auto ascii_code = static_cast<unsigned char>(c);
      result += std::bitset<8>(ascii_code).to_string();
    }

    return result;
  }

  std::string bitStringToText(const std::string &bitString) {
    std::string result;

    size_t padding = bitString.length() % 8;

    if (padding != 0) {
      auto formatted = bitString + std::string(padding, '0');
      return textToBitString(formatted);
    }

    for (size_t i = 0; i < bitString.length(); i+=8) {
      auto byte = bitString.substr(i, 8);
      std::bitset<8> bits(byte);
      auto c = static_cast<char>(bits.to_ulong());
      result += c;
    }

    result.erase(std::ranges::find(result, '\0'), result.end());

    return result;
  }

  std::string xorString(const std::string& bitString, const std::string& key) {
    if (key.empty()) return bitString;

    std::string result = bitString;
    auto keyBits = textToBitString(key);

    if (keyBits.empty()) return bitString;
    size_t index = 0;

    for (auto i = 0; i < bitString.length(); i++) {
      result[i] = (bitString[i] == keyBits[index]) ? '0' : '1';
      index = (index + 1) % keyBits.length();
    }

    return result;
  }

  uint8_t setLSB(uint8_t byte, char bit) {
      byte &= 0b11111110;
      byte |= (bit == '1' ? 1 : 0);
      return byte;
  }

}