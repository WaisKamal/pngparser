#ifndef PNG_PARSER
#define PNG_PARSER

#include <vector>
#include <fstream>
#include <string>

namespace PNGParser {
    // Chunk types (liitle-endian)
    constexpr uint32_t IHDR = 0x52444849;
    constexpr uint32_t PLTE = 0x45544C50;
    constexpr uint32_t IDAT = 0x54414449;
    constexpr uint32_t IEND = 0x444E4549;

    // PNG magic number (little-endian)
    constexpr uint64_t PNG_MAGIC_NUMBER = 0x0A1A0A0D474E5089;

    // Converts little-endian to big-endian
    uint32_t toBigEndian(uint32_t num);

    // Paeth predictor
    int paethPredictor(int a, int b, int c);

    // Reconstruction functions
    uint8_t reconA(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel);

    uint8_t reconB(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel);

    uint8_t reconC(std::vector<std::vector<uint8_t>>& data, uint64_t width, uint64_t height, uint64_t row, uint64_t col, uint8_t bytesPerPixel);

    void parseImage(std::string inputFileName, std::string outputFileName);
}

#endif