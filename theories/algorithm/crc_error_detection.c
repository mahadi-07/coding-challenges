#include <stdio.h>
#include <string.h>

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

static u_int32_t crc32(const void *data, size_t len)
{
    const u_int8_t *bytes = (const u_int8_t *)data;
    u_int32_t crc = 0xFFFFFFFF;

    for(size_t i = 0; i < len; i++) {
        u_int8_t index = (crc ^ bytes[i]) & 0xFF;
        crc = (crc >> 8) ^ crc_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}

#include <stdint.h>

static void pack_u32(uint8_t *buf, uint32_t value)
{
    buf[0] = (uint8_t)( value        & 0xFF);
    buf[1] = (uint8_t)((value >>  8) & 0xFF);
    buf[2] = (uint8_t)((value >> 16) & 0xFF);
    buf[3] = (uint8_t)((value >> 24) & 0xFF);
}

static void pack_u64(uint8_t *buf, uint64_t value)
{
    buf[0] = (uint8_t)( value        & 0xFF);
    buf[1] = (uint8_t)((value >>  8) & 0xFF);
    buf[2] = (uint8_t)((value >> 16) & 0xFF);
    buf[3] = (uint8_t)((value >> 24) & 0xFF);
    buf[4] = (uint8_t)((value >> 32) & 0xFF);
    buf[5] = (uint8_t)((value >> 40) & 0xFF);
    buf[6] = (uint8_t)((value >> 48) & 0xFF);
    buf[7] = (uint8_t)((value >> 56) & 0xFF);
}

int main()
{

    crc32_init();

    const char *text = "123456789";

    u_int32_t crc = crc32(text, strlen(text));

    printf("String : \"%s\"\n", text);
    printf("CRC32  : %08X\n", crc);

    uint8_t b32[4];
    uint8_t b64[8];

    pack_u32(b32, 0x12345678);
    pack_u64(b64, 0x1122334455667788ULL);

    printf("u32: ");
    for (int i = 0; i < 4; i++)
        printf("%02X ", b32[i]);
    printf("\n");

    printf("u64: ");
    for (int i = 0; i < 8; i++)
        printf("%02X ", b64[i]);
    printf("\n");
    
    return 0;
}