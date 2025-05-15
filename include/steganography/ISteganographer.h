#pragma once
#include <string>

class ISteganographer {
public:
  virtual ~ISteganographer() = default;

  virtual bool encode(const std::string& filepath, const std::string& message, const std::string& key) = 0;
  virtual std::string decode(const std::string& filepath, const std::string& key) = 0;
  virtual bool canEncode(const std::string& filepath, const std::string& message) = 0;
  virtual auto getImageDimensions(const std::string& filepath) -> std::pair<int, int> = 0;
};