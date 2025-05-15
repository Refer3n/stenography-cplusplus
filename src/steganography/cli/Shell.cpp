#include "steganography/cli/Shell.h"
#include "steganography/cli/CommandType.h"
#include <fmt/core.h>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <windows.h>

namespace {
    auto printError(const std::string& message)-> void {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD saved_attributes = consoleInfo.wAttributes;

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

        fmt::println("Error: {}", message);

        SetConsoleTextAttribute(hConsole, saved_attributes);
    }
}

Shell::Shell() {}

auto Shell::printPrompt() const -> void {
    auto currentPath = std::filesystem::current_path().string();
    fmt::print("{} $ ", currentPath);
}

auto Shell::handleCommand(const Command& command)-> void {
    if (!command.error.empty()) {
        printError(command.error);
        return;
    }

    switch (command.type) {
    case CommandType::Encrypt:
        executeEncrypt(command.args);
        break;
    case CommandType::Decrypt:
        executeDecrypt(command.args);
        break;
    case CommandType::Info:
        executeInfo(command.args);
        break;
    case CommandType::Check:
        executeCheck(command.args);
        break;
    case CommandType::Help:
        executeHelp();
        break;
    case CommandType::Unknown:
        printError("Unknown command.");
        break;
    }
}


auto Shell::executeEncrypt(const std::vector<std::string>& tokens) -> void {
    auto file = tokens[0];
    auto message = tokens[1];
    auto key = tokens.size() > 2 ? tokens[2] : "";

    if (!Utils::hasWritePermission(file)) {
        printError("No write permissions for file: " + file);
        return;
    }

    auto format = Utils::getImageFormat(file);
    if (format == Utils::ImageFormat::NOT_SUPPORTED) {
        printError("Unsupported image format. Supported formats: .ppm, .bmp");
        return;
    }

    try {
        auto steganographer = steganographerManager.getSteganographer(format);

        if (!steganographer->canEncode(file, message)) {
            printError("Cannot encode message in this image.");
            return;
        }

        if (!steganographer->encode(file, message, key)) {
            printError("Encoding failed unexpectedly.");
            return;
        }

        fmt::println("Message encoded and saved to {}", file);
    } catch (const std::exception& e) {
        printError(std::string("Exception: ") + e.what());
    }
}

auto Shell::executeDecrypt(const std::vector<std::string>& tokens) -> void {
    auto file = tokens[0];
    auto key = tokens.size() > 1 ? tokens[1] : "";

    auto format = Utils::getImageFormat(file);
    if (format == Utils::ImageFormat::NOT_SUPPORTED) {
        printError("Unsupported image format. Supported formats: .ppm, .bmp");
        return;
    }

    try {
        auto steganographer = steganographerManager.getSteganographer(format);
        auto message = steganographer->decode(file, key);

        if (message.empty()) {
            printError("No message found or decryption failed.");
            return;
        }

        fmt::println("Decoded message: {}", message);
    } catch (const std::exception& e) {
        printError(std::string("Exception: ") + e.what());
    }
}

auto Shell::executeInfo(const std::vector<std::string>& tokens) -> void {
    const auto& file = tokens[0];
    auto format = Utils::getImageFormat(file);

    if (format == Utils::ImageFormat::NOT_SUPPORTED) {
        printError("Unsupported image format. Supported formats: .ppm, .bmp");
        return;
    }

    try {
        auto steganographer = steganographerManager.getSteganographer(format);

        std::pair<int, int> dimensions = steganographer->getImageDimensions(file);

        fmt::println("{}", Utils::getImageInfo(file, dimensions));
    } catch (const std::exception& e) {
        printError(std::string("Exception: ") + e.what());
    }
}

auto Shell::executeCheck(const std::vector<std::string>& tokens)-> void {
    auto file = tokens[0];
    auto message = tokens[1];
    auto format = Utils::getImageFormat(file);

    if (format == Utils::ImageFormat::NOT_SUPPORTED) {
        printError("Unsupported image format. Supported formats: .ppm, .bmp");
        return;
    }

    try {
        auto steganographer = steganographerManager.getSteganographer(format);

        if (steganographer->canEncode(file, message)) {
            fmt::print("This image can be used for encoding.\n");
        } else {
            fmt::print("This image cannot be used for encoding.\n");
        }
    } catch (const std::exception& e) {
        printError(std::string("Exception: ") + e.what());
    }
}
auto Shell::executeHelp() const -> void {
    fmt::println("Usage:");
    fmt::println("-e, --encrypt <file> <message> [key]  Encrypt a message in an image.");
    fmt::println("-d, --decrypt <file> [key]  Decrypt a message from an image.");
    fmt::println("-i, --info <file>  Display information about the image format.");
    fmt::println("-c, --check <file> <message>  Check if an image can encode a message.");
    fmt::println("-h, --help  Display this help message.");

    fmt::println("\nSupported image formats: .bmp, .ppm");
}

auto Shell::run()-> void {
    std::string input;

    while (true) {
        printPrompt();
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        Command command = commandParser.parse(input);
        handleCommand(command);
    }
}