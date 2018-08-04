#ifndef CRC32_H
#define CRC32_H

// CRC32
#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
bit32u crc32(void *, bit32u);
void crc32_partial(bit32u *, void *, bit32u);
bit32u crc32_reflect(bit32u, char);

#endif  // CRC32_H
