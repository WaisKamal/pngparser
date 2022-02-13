#include <iostream>
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include "Inflater.hpp"
#include "PNGParser.hpp"

// Converts little-endian to big-endian
uint32_t PNGParser::toBigEndian(uint32_t num) {
    uint32_t result = 0;
    int numShifts = 4;
    while (numShifts-- > 0) {
        result <<= 8;
        result |= num & 255;
        num >>= 8;
    }
    return result;
}

// Paeth predictor
int PNGParser::paethPredictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = std::abs(p - a);
    int pb = std::abs(p - b);
    int pc = std::abs(p - c);
    int Pr;
    if (pa <= pb && pa <= pc) {
        Pr = a;
    }
    else if (pb <= pc) {
        Pr = b;
    }
    else {
        Pr = c;
    }
    return Pr;
}

// Reconstruction functions
uint8_t PNGParser::reconA(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel) {
    if (col <= bytesPerPixel) return 0;
    return data[row][col - bytesPerPixel];
}

uint8_t PNGParser::reconB(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel) {
    if (row == 0) return 0;
    return data[row - 1][col];
}

uint8_t PNGParser::reconC(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel) {
    if (row == 0 || col <= bytesPerPixel) return 0;
    return data[row - 1][col - bytesPerPixel];
}

void PNGParser::parseImage(std::string inputFileName, std::string outputFileName) {
    // Image metadata
    uint32_t width, height;
    uint8_t bitDepth, colorType, compressionMethod, filterMethod, interlaceMethod;

    // The DEFLATE stream
    std::list<uint8_t> deflateStream;

    // The image file
    std::ifstream file(inputFileName, std::ios::binary);

    uint64_t magicNumber = 0;
    file.read(reinterpret_cast<char*>(&magicNumber), 8);
    if (magicNumber != PNG_MAGIC_NUMBER) {
        std::cerr << "Invalid PNG file\n";
        return;
    }

    // Initialize inflater
    Inflater inflater;

    // Read the chunks
    while (file.peek() != EOF) {
        uint32_t chLength, chType;
        uint32_t chCRC;
        // Read chunk length
        file.read(reinterpret_cast<char*>(&chLength), 4);
        chLength = toBigEndian(chLength);
        file.read(reinterpret_cast<char*>(&chType), 4);

        switch (chType) {
            case IDAT: {
                for (uint32_t i = 0; i < chLength; i++) {
                    uint8_t byte;
                    file.read(reinterpret_cast<char*>(&byte), 1);
                    deflateStream.push_back(byte);
                }
                file.read(reinterpret_cast<char*>(&chCRC), 4);
                break;
            }
            case IHDR: {
                file.read(reinterpret_cast<char*>(&width), 4);
                width = toBigEndian(width);
                file.read(reinterpret_cast<char*>(&height), 4);
                height = toBigEndian(height);
                file.read(reinterpret_cast<char*>(&bitDepth), 1);
                file.read(reinterpret_cast<char*>(&colorType), 1);
                file.read(reinterpret_cast<char*>(&compressionMethod), 1);
                file.read(reinterpret_cast<char*>(&filterMethod), 1);
                file.read(reinterpret_cast<char*>(&interlaceMethod), 1);
                file.read(reinterpret_cast<char*>(&chCRC), 4);
                break;
            }
            case IEND: {
                // Stop reading
                file.read(reinterpret_cast<char*>(&chCRC), 4);
                break;
            }
            default: {
                // Skip the chunk
                file.ignore(chLength + 4);
            }
        }
    }

    // The number of bytes per pixel
    int bytesPerPixel;
    if (colorType == 2) {
        bytesPerPixel = 3;
    } else if (colorType == 6) {
        bytesPerPixel = 4;
    } else {
        std::cerr << "Unsupported color type\n";
        return;
    }

    // Inflate the image data
    std::vector<std::vector<uint8_t>> rawData = inflater.inflate(deflateStream, width, height, bytesPerPixel);

    // Apply reverse filtering to the data
    for (size_t row = 0; row < height; row++) {
        uint8_t filterMethod = rawData[row][0];
        switch (filterMethod) {
            case 0: {
                // Leave the data as it is
                break;
            }
            case 1: {
                for (size_t col = 1; col < width * bytesPerPixel + 1; col++) {
                    rawData[row][col] += reconA(rawData, width, height, row, col, bytesPerPixel);
                }
                break;
            }
            case 2: {
                for (size_t col = 1; col < width * bytesPerPixel + 1; col++) {
                    rawData[row][col] += reconB(rawData, width, height, row, col, bytesPerPixel);
                }
                break;
            }
            case 3: {
                for (size_t col = 1; col < width * bytesPerPixel + 1; col++) {
                    uint16_t result = static_cast<uint16_t>(rawData[row][col])
                                    + (static_cast<uint16_t>(reconA(rawData, width, height, row, col, bytesPerPixel))
                                    + static_cast<uint16_t>(reconB(rawData, width, height, row, col, bytesPerPixel))) / 2;
                    rawData[row][col] = result;
                }
                break;
            }
            case 4: {
                for (size_t col = 1; col < width * bytesPerPixel + 1; col++) {
                    rawData[row][col] += paethPredictor(reconA(rawData, width, height, row, col, bytesPerPixel),
                                                        reconB(rawData, width, height, row, col, bytesPerPixel),
                                                        reconC(rawData, width, height, row, col, bytesPerPixel));
                }
                break;
            }
        }
    }

    // Write the pixel data to the output file
    std::ofstream out(outputFileName);
    out << width << " " << height << " " << bytesPerPixel << "\n";
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 1; j < width * bytesPerPixel + 1; j++) {
            out << int(rawData[i][j]) << " ";
        }
        out << "\n";
    }
};