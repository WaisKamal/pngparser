#include <fstream>
#include "bitreader.hpp"

BitReader::BitReader(std::list<uint8_t>& byteList) {
    ls = &byteList;
}