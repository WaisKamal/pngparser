#include <iostream>
#include <vector>
#include <functional>
#include "bitreader.hpp"
#include "huffmantree.hpp"
#include "inflater.hpp"

Inflater::Inflater() {}

// Produces uncompressed data from a deflate stream
std::vector<std::vector<uint8_t>> Inflater::inflate(std::list<uint8_t>& ls, uint64_t width, uint64_t height) {
    BitReader in(ls);

    // Stores the result
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width * 4 + 1));

    // Last empty slot (row and column)
    uint64_t currentRow = 0, currentCol = 0;

    // Insertion function
    std::function<void(uint8_t)> insertByteIntoResult = [&result, &currentRow, &currentCol](uint8_t byte) {
        if (currentCol == result[0].size()) {
            currentCol = 0;
            currentRow++;
        }
        result[currentRow][currentCol++] = byte;
    };

    std::function<void(uint32_t length, uint32_t backDist)> insertByteSequenceIntoResult
        = [&result, &currentRow, &currentCol, &insertByteIntoResult](uint32_t length, uint32_t backDist) {
        uint32_t targetRow = (currentRow * result[0].size() + currentCol - backDist) / result[0].size();
        uint32_t targetCol = (currentRow * result[0].size() + currentCol - backDist) % result[0].size();
        while (length > 0) {
            if (targetCol == result[0].size()) {
                targetRow++;
                targetCol = 0;
            }
            insertByteIntoResult(result[targetRow][targetCol++]);
            length--;
        }
    };

    uint8_t cmf, flg;
    uint8_t cm, cinfo, fcheck, flevel;
    uint32_t adler32;
    bool fdict;

    in.readBytes(cmf, 1);
    cm = cmf & 15;
    if (cm != 8) {
        std::cerr << "Only compression method DEFLATE is supported";
        return {};
    }
    cinfo = cmf >> 4;
    if (cinfo > 7) {
        std::cerr << "Maximum sliding window size is 32768 bytes";
        return {};
    }

    in.readBytes(flg, 1);
    if ((cmf * 256ull + flg) % 31 > 0) {
        std::cerr << "Error in checksum computed from CMF and FLG";
        return {};
    }
    fcheck = flg & 31;
    fdict = flg & 32;
    if (fdict) {
        std::cerr << "PNG files should have no preset dictionary";
        return {};
    }
    flevel = flg >> 6;


    // Build the literal/length Huffman tree
    std::vector<int> literalLengthCodeLengths(288);
    for (int i = 0; i < 288; i++) {
        if (i < 144) {
            literalLengthCodeLengths[i] = 8;
        }
        else if (i < 256) {
            literalLengthCodeLengths[i] = 9;
        }
        else if (i < 280) {
            literalLengthCodeLengths[i] = 7;
        }
        else {
            literalLengthCodeLengths[i] = 8;
        }
    }
    HuffmanTree<uint16_t> literalLengthHft(0, 287, literalLengthCodeLengths);

    // Build the distance Huffman tree
    std::vector<int> distanceCodeLengths(30, 5);
    HuffmanTree<uint8_t> distanceHft(0, 29, distanceCodeLengths);

    // read the DEFLATE blocks
    size_t currentByte = result.size();
    bool bfinal = false;
    while (!bfinal) {
        uint8_t btype;
        in.readBits(bfinal, 1);
        in.readBits(btype, 2);
        switch (btype) {
        case 0: {
            uint16_t blen, bnlen;
            in.readBytes(blen, 2);
            in.readBytes(bnlen, 2);
            for (int i = 0; i < blen; i++) {
                uint8_t byte;
                in.readBytes(byte, 1);
                currentByte++;
            }
            break;
        }
        case 1: {
            while (true) {
                uint16_t ch = literalLengthHft.decode(in);
                if (ch < 256) {
                    insertByteIntoResult(ch);
                    currentByte++;
                }
                else if (ch > 256) {
                    ch -= 257;
                    int extraBits = literalLengthExtraBits[ch];
                    int length = literalLengthBase[ch];
                    if (extraBits > 0) {
                        int extraLength;
                        in.readBits(extraLength, extraBits);
                        length += extraLength;
                    }
                    ch = distanceHft.decode(in);
                    uint8_t backDist = distanceBase[ch];
                    extraBits = distanceExtraBits[ch];
                    if (extraBits > 0) {
                        int extraDist;
                        in.readBits(extraDist, extraBits);
                        backDist += extraDist;
                    }
                    for (int i = 0; i < length; i++) {
                        result.push_back(result[currentByte - backDist]);
                        currentByte++;
                    }
                }
                else if (ch == 256) {
                    break;
                }
            }
            break;
        }
        case 2: {
            uint16_t hlit;
            uint8_t hdist, hclen;
            in.readBits(hlit, 5);
            in.readBits(hdist, 5);
            in.readBits(hclen, 4);
            hlit += 257;
            hdist += 1;
            hclen += 4;
            // Build the code length alphabet Huffman tree
            std::vector<int> codeLengthCodeLengths(19, 0);
            for (int i = 0; i < hclen; i++) {
                uint8_t codeword;
                in.readBits(codeword, 3);
                codeLengthCodeLengths[codeLengthCodeLengthsOrder[i]] = codeword;
            }
            HuffmanTree<uint8_t> codeLengthHft(0, 18, codeLengthCodeLengths);

            // Build the literal/length alphabet Huffman tree
            std::vector<int> literalLengthCodeLengths(hlit, 0);
            for (uint16_t i = 0; i < hlit; i++) {
                uint8_t codeLength = codeLengthHft.decode(in);
                if (codeLength < 16) {
                    literalLengthCodeLengths[i] = codeLength;
                }
                else if (codeLength == 16) {
                    uint8_t copies;
                    in.readBits(copies, 2);
                    copies += 3;
                    for (int j = 0; j < copies; j++) {
                        literalLengthCodeLengths[i + j] = literalLengthCodeLengths[i - 1];
                    }
                    i += copies - 1;
                }
                else if (codeLength == 17) {
                    uint8_t zeros;
                    in.readBits(zeros, 3);
                    zeros += 3;
                    for (int j = 0; j < zeros; j++) {
                        literalLengthCodeLengths[i + j] = 0;
                    }
                    i += zeros - 1;
                }
                else {
                    uint8_t zeros;
                    in.readBits(zeros, 7);
                    zeros += 11;
                    for (int j = 0; j < zeros; j++) {
                        literalLengthCodeLengths[i + j] = 0;
                    }
                    i += zeros - 1;
                }
            }
            HuffmanTree<uint16_t> literalLengthHft(0, 285, literalLengthCodeLengths);

            // Build the distance alphabet Huffman tree
            std::vector<int> distanceCodeLengths(hdist, 0);
            for (uint16_t i = 0; i < hdist; i++) {
                uint8_t codeLength = codeLengthHft.decode(in);
                if (codeLength < 16) {
                    distanceCodeLengths[i] = codeLength;
                }
                else if (codeLength == 16) {
                    uint8_t copies;
                    in.readBits(copies, 2);
                    copies += 3;
                    for (int j = 0; j < copies; j++) {
                        distanceCodeLengths[i + j] = distanceCodeLengths[i - 1];
                    }
                    i += copies - 1;
                }
                else if (codeLength == 17) {
                    uint8_t zeros;
                    in.readBits(zeros, 3);
                    zeros += 3;
                    for (int j = 0; j < zeros; j++) {
                        distanceCodeLengths[i + j] = 0;
                    }
                    i += zeros - 1;
                }
                else {
                    uint8_t zeros;
                    in.readBits(zeros, 7);
                    zeros += 11;
                    for (int j = 0; j < zeros; j++) {
                        distanceCodeLengths[i + j] = 0;
                    }
                    i += zeros - 1;
                }
            }
            HuffmanTree<uint8_t> distanceHft(0, 29, distanceCodeLengths);

            // Now read the data
            while (true) {
                uint16_t ch = literalLengthHft.decode(in);
                if (ch < 256) {
                    insertByteIntoResult(ch);
                    currentByte++;
                }
                else if (ch > 256) {
                    ch -= 257;
                    int extraBits = literalLengthExtraBits[ch];
                    int length = literalLengthBase[ch];
                    if (extraBits > 0) {
                        int extraLength;
                        in.readBits(extraLength, extraBits);
                        length += extraLength;
                    }
                    ch = distanceHft.decode(in);
                    uint64_t backDist = distanceBase[ch];
                    extraBits = distanceExtraBits[ch];
                    if (extraBits > 0) {
                        int extraDist;
                        in.readBits(extraDist, extraBits);
                        backDist += extraDist;
                    }
                    insertByteSequenceIntoResult(length, backDist);
                }
                else if (ch == 256) {
                    break;
                }
            }
            break;
        }
        }
    }

    return result;
}