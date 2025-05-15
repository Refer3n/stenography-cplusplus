#pragma once
#include "../ISteganographer.h"

class BmpSteganographer : public ISteganographer {
public:
  bool encode(const std::string &filepath, const std::string &message, const std::string &key) override;
  std::string decode(const std::string &filepath, const std::string &key) override;
  bool canEncode(const std::string &filepath, const std::string &message) override;
  auto getImageDimensions(const std::string& filepath) -> std::pair<int, int> override;
};