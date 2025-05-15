#pragma once

#include "steganography/SteganographerManager.h"
#include "steganography/cli/CommandParser.h"

#include <string>

class Shell {
  public:
    Shell();

    void run();

  private:
    void printPrompt() const;
    void handleCommand(const Command& command);
    void executeEncrypt(const std::vector<std::string>& tokens);
    void executeDecrypt(const std::vector<std::string>& tokens);
    void executeInfo(const std::vector<std::string>& tokens);
    void executeCheck(const std::vector<std::string>& tokens);
    void executeHelp() const;

    CommandParser commandParser;
    SteganographerManager steganographerManager;
};

