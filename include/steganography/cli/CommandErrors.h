#pragma once
#include "CommandType.h"
#include <string>

namespace CommandErrors {
  inline auto NoCommand() -> std::string {
    return "No command entered.";
  }

  inline auto UnknownFlag(const std::string& flag) -> std::string {
    return "Unknown flag or command: " + flag;
  }

  inline auto ArgError(const CommandType cmd, size_t givenCount) -> std::string {
    switch (cmd) {
    case CommandType::Encrypt:
      return "Encrypt expects 2 or 3 arguments: <image> <message> <secret key>. Got " + std::to_string(givenCount) + ".";
    case CommandType::Decrypt:
      return "Decrypt expects 1 or 2 arguments: <image> <secret key>. Got " + std::to_string(givenCount) + ".";
    case CommandType::Info:
      return "Info expects exactly 1 argument: <image>. Got " + std::to_string(givenCount) + ".";
    case CommandType::Check:
      return "Check expects exactly 2 argument: <image> <message>. Got " + std::to_string(givenCount) + ".";
    case CommandType::Help:
      return "Help takes no arguments.";
    default:
      return "Invalid command.";
    }
  }
}
