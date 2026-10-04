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
    // Low-Level Req #1: Size overflow check
    uint32_t maxSrcSizeInBytes = (aParamSizeInBits + 7) / 8;
    if (aSrcBufSizeInBytes > maxSrcSizeInBytes) {
        return false;
    }

    // Low-Level Req #4: Zero-fill the parameter bits first
    uint32_t startBit = aParamBitOffset;
    uint32_t endBit = aParamBitOffset + aParamSizeInBits;

    for (uint32_t bitIdx = startBit; bitIdx < endBit; bitIdx++) {
        uint32_t byteIdx = bitIdx / 8;
        uint32_t bitPos = bitIdx % 8;
        aMsgBufAddr[byteIdx] &= ~(1 << bitPos);
    }

    // Low-Level Req #2: Write source bytes in reverse order
    // Start from the end of the parameter space
    uint32_t currentBitPos = aParamBitOffset + aParamSizeInBits - 1;

    // Process from aSrcBuf[aSrcBufSizeInBytes-1] down to aSrcBuf[0]
    for (int srcIdx = aSrcBufSizeInBytes - 1; srcIdx >= 0; srcIdx--) {
        uint8_t srcByte = aSrcBuf[srcIdx];

        // Determine how many bits to write from this byte
        uint32_t bitsToWrite = 8;

        // Low-Level Req #3: For aSrcBuf[0], only write remaining bits if < 8
        if (srcIdx == 0) {
            uint32_t bitsWrittenSoFar = (aSrcBufSizeInBytes - 1) * 8;
            uint32_t bitsRemaining = aParamSizeInBits - bitsWrittenSoFar;
            if (bitsRemaining < 8) {
                bitsToWrite = bitsRemaining;
                // Only keep the high bits of srcByte
                srcByte &= (0xFF << (8 - bitsToWrite));
            }
        }

        // Write bits from this byte (LSB to MSB of srcByte writes to current position backwards)
        for (uint32_t bitInByte = 0; bitInByte < bitsToWrite; bitInByte++) {
            uint32_t targetByteIdx = currentBitPos / 8;
            uint32_t targetBitPos = currentBitPos % 8;

            // Extract bit from srcByte (from LSB side)
            uint8_t bitValue = (srcByte >> bitInByte) & 1;

            // Write bit to target
            if (bitValue) {
                aMsgBufAddr[targetByteIdx] |= (1 << targetBitPos);
            }

            if (currentBitPos == aParamBitOffset) break;
            currentBitPos--;
        }
    }

    return true;
}

#endif // MEMCPY_H
