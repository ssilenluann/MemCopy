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
    // Low-Level Req 1: size overflow 校验
    // ---------------------------------------------------------
    if (aSrcBufSizeInBytes > (aParamSizeInBits + 7) / 8) {
        return false;
    }

    // current_end_bit 记录当前目标写入范围的截止 Bit 位（开区间）
    uint32_t current_end_bit = aParamBitOffset + aParamSizeInBits;

    // ---------------------------------------------------------
    // Low-Level Req 2 & 3: 从 aSrcBuf[aSrcBufSizeInBytes-1] 倒序向前写入
    // ---------------------------------------------------------
    for (uint32_t i_plus_one = aSrcBufSizeInBytes; i_plus_one > 0; --i_plus_one) {
        uint32_t i = i_plus_one - 1;
        uint32_t bits_to_write = 8;

        // 处理 aSrcBuf[0] 截断逻辑
        if (i == 0) {
            uint32_t bits_left = current_end_bit - aParamBitOffset;
            if (bits_left < 8) {
                bits_to_write = bits_left; // 只取低 x 位
            }
        }

        uint8_t val = aSrcBuf[i];
        uint32_t start_bit = current_end_bit - bits_to_write;

        // 将提取的 bits 依次写入 msg buffer 
        // 遵循大端位序（Big-Endian / Motorola）的映射
        for (uint32_t k = 0; k < bits_to_write; ++k) {
            uint32_t target_bit_idx = start_bit + k;
            
            // 计算在内存中的 Byte 索引
            uint32_t byte_idx = target_bit_idx / 8;
            
            // 核心修正：大端位序中，Bit 0 是 MSB (1<<7)，Bit 7 是 LSB (1<<0)
            uint32_t bit_in_byte = 7 - (target_bit_idx % 8);

            // "前x个bit"：提取 val 的最低 bits_to_write 位中的对应位
            // 按从高位向低位的顺序取出，并放入 target 较低的 index 中
            uint8_t bit_val = (val >> (bits_to_write - 1 - k)) & 1;

            // 读-修改-写操作：绝对确保不会影响同个 Byte 里的其它参数 (满足 Req #3)
            if (bit_val) {
                aMsgBufAddr[byte_idx] |= (1 << bit_in_byte);
            } else {
                aMsgBufAddr[byte_idx] &= ~(1 << bit_in_byte);
            }
        }

        // 更新下一轮写入的结束位置
        current_end_bit = start_bit;
    }

    // ---------------------------------------------------------
    // Low-Level Req 4: 如果 aSrcBuf 写完后，高位仍有空余 bit，将其全部置 0
    // ---------------------------------------------------------
    while (current_end_bit > aParamBitOffset) {
        current_end_bit--;
        uint32_t byte_idx = current_end_bit / 8;
        uint32_t bit_in_byte = 7 - (current_end_bit % 8);
        
        // 同样安全的单 bit 置 0 操作
        aMsgBufAddr[byte_idx] &= ~(1 << bit_in_byte);
    }

    return true;
}
#endif // MEMCPY_H
