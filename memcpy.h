#ifndef MEMCPY_H
#define MEMCPY_H

#include <stdint.h>
#include <string.h>

bool SetParamValue(
    uint8_t* aSrcBuf,
    uint32_t aSrcBufSizeInBytes,
    uint8_t* aMsgBufAddr,
    uint32_t aParamBitOffset,
    uint32_t aParamSizeInBits)
{
    // ---------------------------------------------------------
    // Low-Level Req 1: size overflow
    // ---------------------------------------------------------
    if (aSrcBufSizeInBytes > (aParamSizeInBits + 7) / 8) {
        return false;
    }

    uint32_t current_end_bit = aParamBitOffset + aParamSizeInBits;

    // ---------------------------------------------------------
    // Low-Level Req 2 & 3
    // ---------------------------------------------------------
    for (uint32_t i_plus_one = aSrcBufSizeInBytes; i_plus_one > 0; --i_plus_one) {
        uint32_t i = i_plus_one - 1;
        uint32_t bits_to_write = 8;

        if (i == 0) {
            uint32_t bits_left = current_end_bit - aParamBitOffset;
            if (bits_left < 8) {
                bits_to_write = bits_left;
            }
        }

        uint8_t val = aSrcBuf[i];
        uint32_t start_bit = current_end_bit - bits_to_write;

        for (uint32_t b = 0; b < bits_to_write; ++b) {
            uint32_t target_bit_idx = start_bit + b;
            uint32_t byte_idx = target_bit_idx / 8;
            uint32_t bit_in_byte = target_bit_idx % 8;

            uint8_t bit_val = (val >> b) & 1;

            if (bit_val) {
                aMsgBufAddr[byte_idx] |= (1 << bit_in_byte); 
            } else {
                aMsgBufAddr[byte_idx] &= ~(1 << bit_in_byte);
            }
        }

        current_end_bit = start_bit;
    }

    // ---------------------------------------------------------
    // Low-Level Req 4: 
    // ---------------------------------------------------------
    while (current_end_bit > aParamBitOffset) {
        current_end_bit--;
        uint32_t byte_idx = current_end_bit / 8;
        uint32_t bit_in_byte = current_end_bit % 8;
        
        aMsgBufAddr[byte_idx] &= ~(1 << bit_in_byte);
    }

    return true;
}

#endif // MEMCPY_H
