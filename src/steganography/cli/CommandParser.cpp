#include "steganography/cli/CommandParser.h"
#include "steganography/cli/CommandErrors.h"
#include "steganography/cli/CommandType.h"

#include <iomanip>
#include <map>
#include <string>

static const std::map<std::string, CommandType> commandMap = {
  {"-e", CommandType::Encrypt},
  {"--encrypt", CommandType::Encrypt},
  {"-d", CommandType::Decrypt},
  {"--decrypt", CommandType::Decrypt},
  {"-i", CommandType::Info},
  {"--info", CommandType::Info},
  {"-c", CommandType::Check},
  {"--check", CommandType::Check},
  {"-h", CommandType::Help},
  {"--help", CommandType::Help}
};

auto CommandParser::parse(const std::string &input) const -> Command {
  std::stringstream ss(input);

  std::vector<std::string> tokens;

  std::string token;

  while (ss >> std::quoted(token) || ss >> token) {
    tokens.push_back(token);
  }

  if (tokens.empty()) {
    return {CommandType::Help, {}, ""};
  }

  auto it = commandMap.find(tokens[0]);
  if (it == commandMap.end()) {
    return {CommandType::Unknown, {}, CommandErrors::UnknownFlag(tokens[0])};
  }

  const auto& type = it->second;
  tokens.erase(tokens.begin());

  bool err = false;
  switch (type) {
  case CommandType::Encrypt:
    err = (tokens.size() < 2 || tokens.size() > 3);
    break;
  case CommandType::Decrypt:
    err = (tokens.size() < 1 || tokens.size() > 2);
    break;
  case CommandType::Info:
    err = (tokens.size() != 1);
    break;
  case CommandType::Check:
    err = (tokens.size() != 2);
    break;
  case CommandType::Help:
    err = (!tokens.empty());
    break;
  default:
    err = true;
    break;
  }

  if (err) {
    return {CommandType::Unknown, tokens, CommandErrors::ArgError(type, tokens.size())};
  }

  return {type, tokens, ""};
}