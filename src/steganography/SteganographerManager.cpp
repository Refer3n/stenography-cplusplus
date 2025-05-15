#include "steganography/SteganographerManager.h"

#include "steganography/bmp/BmpSteganographer.h"
#include "steganography/ppm/PpmSteganographer.h"

SteganographerManager::SteganographerManager() {}

auto SteganographerManager::getSteganographer(const Utils::ImageFormat format) -> ISteganographer* {
  const auto& it = steganographers.find(format);
  if (it != steganographers.end()) {
    return it->second;
  }

  ISteganographer* steganographer = nullptr;
  switch (format) {
  case Utils::ImageFormat::BMP:
    steganographer = new BmpSteganographer();
    break;
  case Utils::ImageFormat::PPM:
    steganographer = new PpmSteganographer();
    break;
  default:
    return nullptr;
  }

  steganographers[format] = steganographer;
  return steganographer;
}
