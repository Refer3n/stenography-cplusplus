#pragma once
#include "CommandType.h"
#include <string>
#include <vector>

struct Command {
  CommandType type = CommandType::Unknown;
  std::vector<std::string> args;
  std::string error;
};
