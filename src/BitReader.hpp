#ifndef BIT_READER
#define BIT_READER

#include <list>
#include <string>

class BitReader {
public:
    BitReader(std::list<uint8_t>& ls);

    template <typename T>
    void readBytes(T& into, int n) {
        // Ignore any remaining bits
        buffer = bufferLength = 0;
        into = 0;
        for (int i = 0; i < n; i++) {
            uint8_t byte = ls->front();
            ls->pop_front();
            into |= (byte << (i * 8));
        }
    }

    template <typename T>
    void readBits(T& into, int n) {
        into = 0;
        int pos = 0;
        while (pos < n) {
            if (bufferLength == 0) {
                buffer = ls->front();
                ls->pop_front();
                bufferLength = 8;
            }
            into |= (buffer & 1) << pos++;
            buffer >>= 1;
            bufferLength--;
        }
    }

    int getBufferLength() {
        return bufferLength;
    }

private:
    std::list<uint8_t>* ls;
    uint8_t buffer = 0;
    int bufferLength = 0;
};

#endif