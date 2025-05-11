#pragma once
#include "../ISteganographer.h"

class PngSteganographer : public ISteganographer {
public:
  bool encode(const std::string &filepath, const std::string &message, const std::string &key) override;
  std::string decode(const std::string &message, const std::string &key) override;
  bool canEncode(const std::string &filepath, const std::string &message) override;
};