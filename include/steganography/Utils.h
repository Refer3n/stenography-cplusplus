#pragma once
#include <cstdint>
#include <string>

namespace Utils {
  enum class ImageFormat { BMP, PPM, NOT_SUPPORTED };

  auto getImageFormat(const std::string& filename) -> ImageFormat;

  auto getImageFormatName(ImageFormat format) -> std::string;

  auto getImageInfo(const std::string& filePath, const std::pair<int, int>& dimensions) -> std::string;

  auto hasWritePermission(const std::string& filepath) -> bool;

  auto textToBitString(const std::string& message) -> std::string;
  auto bitStringToText(const std::string& bitString) -> std::string;

  auto xorString(const std::string& bitString, const std::string& key) -> std::string;

  auto setLSB(uint8_t byte, char bit) -> uint8_t;
}