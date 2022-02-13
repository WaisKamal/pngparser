#include <string>
#include <iostream>
#include "PNGParser.hpp"

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Please specify input and output file names";
        return -1;
    }

    PNGParser::parseImage(argv[1], argv[2]);
    
    return 0;
}