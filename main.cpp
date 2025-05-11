#include "steganography/bmp/BmpSteganographer.h"
#include "steganography/ppm/PpmSteganographer.h"
#include "steganography/png/PngSteganographer.h" // <-- Add this line

#include <iostream>

int main() {
    std::string message = "Test message";
    std::string key = "secretKey";

    // Test BMP Steganography
    BmpSteganographer bmpStego;
    std::string bmpFilepath = "sample1.bmp";

    bool encodedBmp = bmpStego.encode(bmpFilepath, message, key);
    if (encodedBmp) {
        std::cout << "BMP Message encoded successfully!" << std::endl;
    } else {
        std::cout << "BMP Encoding failed!" << std::endl;
    }

    std::string decodedBmpMessage = bmpStego.decode(bmpFilepath, key);
    if (!decodedBmpMessage.empty()) {
        std::cout << "Decoded BMP Message: " << decodedBmpMessage << std::endl;
    } else {
        std::cout << "BMP Decoding failed!" << std::endl;
    }

    // Test PPM Steganography
    PpmSteganographer ppmStego;
    std::string ppmFilepath = "sample2.ppm";

    bool encodedPpm = ppmStego.encode(ppmFilepath, message, key);
    if (encodedPpm) {
        std::cout << "PPM Message encoded successfully!" << std::endl;
    } else {
        std::cout << "PPM Encoding failed!" << std::endl;
    }

    std::string decodedPpmMessage = ppmStego.decode(ppmFilepath, key);
    if (!decodedPpmMessage.empty()) {
        std::cout << "Decoded PPM Message: " << decodedPpmMessage << std::endl;
    } else {
        std::cout << "PPM Decoding failed!" << std::endl;
    }

    // Test PNG Steganography
    PngSteganographer pngStego; // <-- Add this object
    std::string pngFilepath = "sample3.png";

    bool encodedPng = pngStego.encode(pngFilepath, message, key);
    if (encodedPng) {
        std::cout << "PNG Message encoded successfully!" << std::endl;
    } else {
        std::cout << "PNG Encoding failed!" << std::endl;
    }

    std::string decodedPngMessage = pngStego.decode(pngFilepath, key);
    if (!decodedPngMessage.empty()) {
        std::cout << "Decoded PNG Message: " << decodedPngMessage << std::endl;
    } else {
        std::cout << "PNG Decoding failed!" << std::endl;
    }

    return 0;
}
