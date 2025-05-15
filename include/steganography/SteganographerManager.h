#pragma once

#include "ISteganographer.h"
#include "Utils.h"

#include <map>

class SteganographerManager {
public:
  SteganographerManager();

  auto getSteganographer(Utils::ImageFormat format) -> ISteganographer*;

private:
  std::map<Utils::ImageFormat, ISteganographer*> steganographers;
};