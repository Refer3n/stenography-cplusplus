#pragma once

#include "Command.h"

#include <string>

class CommandParser {
public:
  [[nodiscard]] auto parse(const std::string& input) const -> Command;
};
