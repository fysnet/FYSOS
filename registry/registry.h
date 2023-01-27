/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2023
 *  
 *  This code is donated to the Freeware community.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and if distributed, have the same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  27 Jan 2023
 *
 */


#ifndef FYSOS_REGISTRY
#define FYSOS_REGISTRY

// this is the maximum size of the data of an entry (string or data)
#define REG_MAX_SIZE      65536

#define REG_NAME_DEPTH_MAX  256  // can't go more than 255 generations deep
#define REG_NAME_LEN_MAX     32  // includes the null terminator

#define REG_BASE_SIG_S  'BASE'
#define REG_BASE_SIG_E  'ESAB'
#define REG_BASE_VER  0x00010000   // 1.0

#if !defined(__cplusplus)
  typedef unsigned  char      bool;
#endif

#pragma pack(push, 1)

// must remain evenly divisable by 4
struct S_REGISTRY_BASE {
  uint32_t  magic;
  uint32_t  crc;
  uint32_t  version;
  uint32_t  padding;
  uint64_t  size;       // size in bytes of allocated memory
  uint64_t  length;     // count of dwords used
  uint64_t  last_modified;
  uint64_t  reserved;   // reserved for future use
};

#pragma pack(pop)

#define REG_HIVE_SIG_S  'HIVE'   // Hive start token
#define REG_HIVE_SIG_E  'EVIH'   // Hive end token
/*  Hive consists of:
 *   HIVE
 *   NAME[REG_NAME_LEN_MAX]      // occupies 8 32-bit dwords (1 -> 8)
 *   DEPTH                       // low 8 bits is the depth, remaining 24 bits are reserved
 *   RESERVED
 *     children
 *   EVIH
 */


#define REG_CELL_SIG_S  'CELL'   // Cell start token
#define REG_CELL_SIG_E  'LLEC'   // Cell end token
/*  Cell consists of:
 *   CELL
 *   NAME[REG_NAME_LEN_MAX]   // occupies 8 32-bit dwords (1 -> 8)
 *   32-bit type/flags
 *   32-bit length  (in dwords. i.e.: length of 4 = 1 here)
 *   data[length]
 *   LLEC
 */

typedef enum {
  RegistryTypeExist = 0,     // no data, empty cell
  RegistryTypeBool,          // 32-bit value of 0 or non-zero
  RegistryTypeInt,           // 32-bit signed integer value 
  RegistryTypeUnsigned,      // 32-bit unsigned integer value 
  RegistryTypeIntLong,       // 64-bit signed long integer value 
  RegistryTypeUnsignedLong,  // 64-bit unsigned long integer value 
  RegistryTypeStr,           // UTF-8 string (must be null-terminated)
  RegistryTypeBin            // Up to 65536 bytes of data
} REGISTRY_TYPE;


int allocate_initialize_registry(size_t size);
void free_registry(void);

int registry_exist(const char *path);

int registry_read_boolean(const char *path, bool *value);
int registry_read_int(const char *path, int *value);
int registry_read_unsigned(const char *path, unsigned int *value);
int registry_read_int_long(const char *path, int64_t *value);
int registry_read_unsigned_long(const char *path, uint64_t *value);
int registry_read_string(const char *path, char *str, const size_t max_len);
int registry_read_binary(const char *path, void *bin, const size_t max_len);
int registry_read(const char *path, void *data, size_t len, const REGISTRY_TYPE type);

int registry_write_exist(const char *path);
int registry_write_boolean(const char *path, const bool value);
int registry_write_int(const char *path, const int value);
int registry_write_unsigned(const char *path, const unsigned int value);
int registry_write_int_long(const char *path, const int64_t value);
int registry_write_unsigned_long(const char *path, const uint64_t value);
int registry_write_string(const char *path, const char *str);
int registry_write_binary(const char *path, const void *bin, const size_t len);
int registry_write(const char *path, const void *data, size_t len, const REGISTRY_TYPE type);

int registry_remove(const char *path);

void dump_registry(void);

#endif // FYSOS_REGISTRY
