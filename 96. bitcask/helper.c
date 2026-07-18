#include <stdio.h>
#include <stdint.h>
#include <string.h>

/**
 * CRC
 */
#define POLYNOMIAL 0xEDB88320
static u_int32_t crc_table[256];

static void crc32_init(void)
{
    for(u_int32_t i = 0; i < 256; i++) {
        u_int32_t crc = i;
        for(int j = 0; j < 8; j++) {
            if(crc & 1)
                crc = (crc >> 1) ^ POLYNOMIAL;
            else
                crc = crc >> 1;
        }
        crc_table[i] = crc;
    }
}

u_int32_t crc32_of(const void *data, size_t len)
{
    crc32_init();
    
    const u_int8_t *bytes = (const u_int8_t *)data;
    u_int32_t crc = 0xFFFFFFFF;
    for(size_t i = 0; i < len; i++) {
        u_int8_t index = (crc ^ bytes[i]) & 0xFF;
        crc = (crc >> 8) ^ crc_table[index];
    }
    return crc ^ 0xFFFFFFFF;
}

/**
 * Litte endian
 */
void pack_u32(u_int8_t *buf, u_int32_t value)
{
    buf[0] = (value & 0xFF);
    buf[1] = ((value >> 8) & 0xFF);
    buf[2] = ((value >> 16) & 0xFF);
    buf[3] = ((value >> 24) & 0xFF);
}

void pack_u64(u_int8_t *buf, u_int64_t value)
{
    buf[0] = (value & 0xFF);
    buf[1] = ((value >> 8) & 0xFF);
    buf[2] = ((value >> 16) & 0xFF);
    buf[3] = ((value >> 24) & 0xFF);
    buf[4] = ((value >> 32) & 0xFF);
    buf[5] = ((value >> 40) & 0xFF);
    buf[6] = ((value >> 48) & 0xFF);
    buf[7] = ((value >> 56) & 0xFF);
}