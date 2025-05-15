#pragma once

#include "steganography/SteganographerManager.h"
#include "steganography/cli/CommandParser.h"

#include <string>

class Shell {
  public:
    Shell();

    auto run() -> void;

  private:
    auto printPrompt() const -> void;
    auto handleCommand(const Command& command) -> void;
    auto executeEncrypt(const std::vector<std::string>& tokens) -> void;
    auto executeDecrypt(const std::vector<std::string>& tokens) -> void;
    auto executeInfo(const std::vector<std::string>& tokens) -> void;
    auto executeCheck(const std::vector<std::string>& tokens) -> void;
    auto executeHelp() const -> void;

    CommandParser commandParser;
    SteganographerManager steganographerManager;
};

