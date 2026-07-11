#include <stdio.h>

void crc32_init(void);
u_int32_t crc32_of(const void *data, size_t len);

void pack_u32(u_int8_t *buf, u_int32_t value);
void pack_u64(u_int8_t *buf, u_int64_t value);