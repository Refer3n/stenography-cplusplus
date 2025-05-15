#pragma once
#include <cstdint>
#include <string>

namespace Utils {
  enum class ImageFormat { BMP, PPM, NOT_SUPPORTED };

  ImageFormat getImageFormat(const std::string& filename);

  std::string getImageFormatName(ImageFormat format);

  auto getImageInfo(const std::string& filePath, const std::pair<int, int>& dimensions) -> std::string;

  bool hasWritePermission(const std::string& filepath);

  std::string textToBitString(const std::string& message);
  std::string bitStringToText(const std::string& bitString);

  std::string xorString(const std::string& bitString, const std::string& key);

  uint8_t setLSB(uint8_t byte, char bit);

  uint32_t crc32(const uint8_t* data, size_t length);
}