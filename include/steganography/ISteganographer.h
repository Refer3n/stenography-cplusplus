#pragma once
#include <string>
#include <vector>

class ISteganographer {
public:
  virtual ~ISteganographer() = default;

  virtual bool encode(const std::string& filepath, const std::string& message, const std::string& key) = 0;
  virtual std::string decode(const std::string& filepath, const std::string& key) = 0;
  virtual bool canEncode(const std::string& filepath, const std::string& message) = 0;
  virtual auto getImageDimensions(const std::string& filepath) -> std::pair<int, int> = 0;

protected:
  auto encodeLSB(std::vector<char>& buffer, size_t pixelDataOffset, const std::string& message, const std::string& key, const std::string& filepath) -> bool;
  auto decodeLSB(const std::vector<char>& buffer, size_t pixelDataOffset, const std::string& key) -> std::string;
  auto canEncodeWithDimensions(int width, int height, const std::string& message) -> bool;
};