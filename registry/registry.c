/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2025
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
 * Last update:  29 Aug 2025
 *
 */

#define TRUE   1
#define FALSE  0

#include <ctype.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "registry.h"

#include "checksum.h"

uint32_t reg_spinlock = 0;

void *kernel_reg = NULL;

extern uint64_t timestamp(uint64_t now);
extern void spin_lock(uint32_t *lock);
extern void spin_unlock(uint32_t *lock);

// define this to allow the debug printing stuff
#define REGISTRY_DEBUG

int allocate_initialize_registry(size_t size) {

  // initialize the crc
  crc32_initialize();
  
  // allocate the memory to use
  kernel_reg = calloc(size, 1);
  if (kernel_reg == NULL)
    return -1;
  
  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  uint32_t *pos = (uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE));
  
  base->magic = REG_BASE_SIG_S;
  base->crc = 0;
  base->version = REG_BASE_VER;
  base->size = size;
  base->length = (sizeof(struct S_REGISTRY_BASE) + (12 * 4) + 4);
  base->last_modified = timestamp(0);
    // the required parent hive. All other hives and cells will 
    //  live within this hive. It must be called "System"
    pos[0] = REG_HIVE_SIG_S;
    memset(&pos[1], 0, REG_NAME_LEN_MAX);  // occupies 8 32-bit dwords (1 -> 8)
    strcpy((char *) &pos[1], "System");
    pos[9] = 0;  // depth (base hive)
    pos[10] = 0; // reserved
    pos[11] = REG_HIVE_SIG_E;
  pos[12] = REG_BASE_SIG_E;  // ending sig (nothing after this)
  base->crc = crc32(base, base->length);
  
  return 0;
}

// simply free's the memory used by the registry
void free_registry(void) {
  free(kernel_reg);
  kernel_reg = NULL;
}

int registry_read_boolean(const char *path, bool *value) {
  uint32_t bval;

  int ret = registry_read(path, &bval, sizeof(uint32_t), RegistryTypeBool);
  if ((ret > 0) && value)
    *value = (bval != 0);
  
  return ret;
}

int registry_read_int(const char *path, int *value) {
  int ival;

  int ret = registry_read(path, &ival, sizeof(int), RegistryTypeInt);
  if ((ret > 0) && value)
    *value = ival;
  
  return ret;
}

int registry_read_unsigned(const char *path, unsigned int *value) {
  int uval;

  int ret = registry_read(path, &uval, sizeof(unsigned int), RegistryTypeUnsigned);
  if ((ret > 0) && value)
    *value = uval;
  
  return ret;
}

int registry_read_int_long(const char *path, int64_t *value) {
  int64_t ilval;

  int ret = registry_read(path, &ilval, sizeof(int64_t), RegistryTypeIntLong);
  if ((ret > 0) && value)
    *value = ilval;
  
  return ret;
}

int registry_read_unsigned_long(const char *path, uint64_t *value) {
  uint64_t ulval;

  int ret = registry_read(path, &ulval, sizeof(uint64_t), RegistryTypeUnsignedLong);
  if ((ret > 0) && value)
    *value = ulval;
  
  return ret;
}

int registry_read_string(const char *path, char *str, const size_t max_len) {
  char *buf = (char *) malloc(max_len);

  int ret = registry_read(path, buf, max_len, RegistryTypeStr);
  if ((ret > 0) && str)
    memcpy(str, buf, (ret < max_len) ? ret : max_len);
  free(buf);
  
  return ret;
}

int registry_read_binary(const char *path, void *bin, const size_t max_len) {
  void *buf = malloc(max_len);

  int ret = registry_read(path, buf, max_len, RegistryTypeBin);
  if ((ret > 0) && bin)
    memcpy(bin, buf, (ret < max_len) ? ret : max_len);
  free(buf);
  
  return ret;
}

int registry_write_exist(const char *path) {
  return registry_write(path, NULL, 0, RegistryTypeExist);
}

int registry_write_boolean(const char *path, const bool value) {
  const uint32_t bval = (uint32_t) value & 0xFF;
  return registry_write(path, &bval, 4, RegistryTypeBool);
}

int registry_write_int(const char *path, const int value) {
  const int ival = value;
  return registry_write(path, &ival, sizeof(int), RegistryTypeInt);
}

int registry_write_unsigned(const char *path, const unsigned int value) {
  const unsigned int uval = value;
  return registry_write(path, &uval, sizeof(unsigned int), RegistryTypeUnsigned);
}

int registry_write_int_long(const char *path, const int64_t value) {
  const int64_t ilval = value;
  return registry_write(path, &ilval, sizeof(int64_t), RegistryTypeIntLong);
}

int registry_write_unsigned_long(const char *path, const uint64_t value) {
  const uint64_t ulval = value;
  return registry_write(path, &ulval, sizeof(uint64_t), RegistryTypeUnsignedLong);
}

int registry_write_string(const char *path, const char *str) {
  size_t len = strlen(str);
  if (len < (REG_MAX_SIZE - 1))
    return registry_write(path, str, len + 1, RegistryTypeStr);
  return 0;
}

int registry_write_binary(const char *path, const void *bin, const size_t len) {
  // we limit the length to REG_MAX_SIZE
  if (len < REG_MAX_SIZE)
    return registry_write(path, bin, len, RegistryTypeBin);
  else
    return 0;
}

// gets the item name from a path.
// the path could start with a '/' (just ignore)
// the name should not be longer than max_len on entry and on exit
//
// returns the null terminated name in 'name'
const char *registry_get_name(const char *path, char *name, size_t max_len) {
  char *n = name;
  
  // if the user started with a '/', ignore it
  if (*path == '/')
    path++;
  
  while (*path && (*path != '/') && max_len) {
    *n++ = *path++;
    max_len--;
  }
  *n = '\0';
  
  // if we found a name longer than we allow, return NULL
  if ((max_len == 0) && *path && (*path != '/'))
    return NULL;
  
  // if we didn't get a name, return an error
  if (name[0] == '\0')
    return NULL;
  
  return path;
}

// allocate a list of generation names and parse the path
// if successful (and valid path), returns a count and a pointer
//  to the list of names.
// if unsuccessful, return a count of zero and frees the list,
//  returning NULL
//
// we assume that the first HIVE is "/System".  So we remove it from
//  the path if we find it:
//      found:              actual:
//    "/first/second" == "/System/first/second"
//  however, 
//      found:                    actual:
//    "/System/first/second" == "/System/System/first/second"
void *registry_parse_path(const char *path, int *count) {
  const char *p = path;
  int i, cnt;
  bool first = FALSE;
  
  // allocate our array of names
  char **arr = (char **) calloc(REG_NAME_DEPTH_MAX * sizeof(char *), 1);
  for (i=0; i<REG_NAME_DEPTH_MAX; i++)
    arr[i] = (char *) malloc(REG_NAME_LEN_MAX);

  // get the generations/item name
  cnt = 0;
  do {
    p = registry_get_name(p, arr[cnt], REG_NAME_LEN_MAX - 1);
    // the /System first name is assumed.  If given on the path
    //  string, ignore it. (but only the first one.  i.e.: allow "/System/system/next..." = "/system/next..."
    if (!first && (cnt == 0) && (strcmp(arr[0], "System") == 0)) {
      first = TRUE;
      continue;
    }
    cnt++;
  } while (p && (*p == '/') && (cnt < REG_NAME_DEPTH_MAX));

  // p != NULL if we found a valid path
  if (p == NULL) {
    printf("Reg: Error in path... [%s]\n", path);  // todo:  remove this line??
    
    // free the memory used by the names
    for (i=0; i<REG_NAME_DEPTH_MAX; i++)
      free(arr[i]);
    free(arr);

    if (count) *count = 0;
    return NULL;
  }

  if (count) *count = cnt;
  return arr;
}

// search for an item with the 'name' in the current hive
uint32_t *registry_find_item(uint32_t *pos, const char *name, const uint32_t sig) {
  int depth = 0;

  while (1) {
    if (pos[0] == REG_HIVE_SIG_S) {
      if ((depth == 1) && (pos[0] == sig) && (strcmp((const char *) &pos[1], name) == 0))
        return pos;
      pos += 11;
      depth++;
    } else if (pos[0] == REG_CELL_SIG_S) {
      if ((depth == 1) && (pos[0] == sig) && (strcmp((const char *) &pos[1], name) == 0))
        return pos;
      pos += 11 + (size_t) pos[10] + 1;  // pos[10] can = zero
    } else if (pos[0] == REG_HIVE_SIG_E) {
      depth--;
      if (depth <= 0)
        break;
      else
        pos++;
    } else
      break;
  }
  
  return NULL;
}

// this find the ending hive tag for this generation.
//  we need to skip over the data portion of a cell so
//   incase the data itself has the ending HIVE sequence.
//   (it could happen)
// returns the dword of the found ending tag
uint32_t *registry_find_end_tag(uint32_t *pos) {
  int depth = 0;
  
  pos++;  // skip the opening tag
  while (1) {
    if (*pos == REG_HIVE_SIG_S)
      depth++;
    else if (*pos == REG_HIVE_SIG_E) {
      if (depth > 0)
        depth--;
      else
        break;
    } else if (*pos == REG_BASE_SIG_E) {
      break;
    } else if (*pos == REG_CELL_SIG_S) {
      pos += 11 + (size_t) pos[10];  // pos[10] can = zero
    }
    pos++;
  }
  
  return pos;
}

// move everything from 'pos' to the end, foward to make room for inserting item
void registry_move_forward(const uint32_t *pos, const size_t extra) {
  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;

  // is there room
  // (this shouldn't happen because we check for this after a write is done.
  if ((base->size - base->length) < ((12 + extra) * 4)) {
    printf("Error:  Ran out of room in the Registry...\n");
    abort();
  }

  uint32_t *s = (uint32_t *) ((uint8_t *) base + base->length - 4);
  uint32_t *t = s + 12 + extra;
  while (s >= pos)
    *t-- = *s--;
  base->length += ((12 + extra) * 4);
}

// add a hive to an existing hive
uint32_t *registry_add_hive(uint32_t *pos, const char *name, const uint32_t hive_depth) {
  
  // find the ending hive tag
  pos = registry_find_end_tag(pos);
  
  // found end of this hive.  Move everything after it a bit forward and insert new hive.
  registry_move_forward(pos, 0);

  pos[0] = REG_HIVE_SIG_S;
  memset(&pos[1], 0, REG_NAME_LEN_MAX);  // occupies 8 32-bit dwords (1 -> 8)
  strncpy((char *) &pos[1], name, REG_NAME_LEN_MAX);
  pos[9] = hive_depth;
  pos[10] = 0;
  pos[11] = REG_HIVE_SIG_E;

  // return position of this new hive
  return pos;
}

// add a cell to an existing hive
// len is in dwords
uint32_t *registry_add_cell(uint32_t *pos, const char *name, const size_t len) {

  // find the ending hive tag
  pos = registry_find_end_tag(pos);

  // found end of this hive.  Move everything after it a bit forward and insert new hive.
  registry_move_forward(pos, len);

  pos[0] = REG_CELL_SIG_S;
  memset(&pos[1], 0, REG_NAME_LEN_MAX);  // occupies 8 32-bit dwords (1 -> 8)
  strncpy((char *) &pos[1], name, REG_NAME_LEN_MAX);

  // return position of this new cell
  return pos;
}

// removes a Cell or a (number of) Hive(s) from the current position
uint32_t *registry_remove_item(uint32_t *pos) {
  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  uint32_t *s = pos, *end = NULL;
  size_t count, clear;

  // make sure we are at a Cell or a Hive
  if (*pos == REG_CELL_SIG_S)
    end = pos + 11 + pos[10] + 1;  // pos[10] can = zero
  else if (*pos == REG_HIVE_SIG_S) {
    // find the ending hive tag
    end = registry_find_end_tag(pos);
    if (end != NULL)
      end++;
  }

  // found the end, so remove the item, 
  //  clearing the now unused space at the end.
  if (end != NULL) {
    clear = end - pos;  // count of dwords to clear after the end
    count = (base->length - ((uint8_t *) end - (uint8_t *) base)) / 4;  // count of remaining dwords to move
    base->length -= (clear * 4);  // update the count
    while (count--)
      *s++ = *end++;
    while (clear--)
      *s++ = 0;
  }

  return pos;
}

// see if a registry item exists.
// Path can end in hive or cell. Will return existence of either.
// ('/System' is assumed.  we start at '/System'.  Everything is a child of '/System')
// returns 1 if exist
// returns 0 if item was not found
int registry_exist(const char *path) {

  spin_lock(&reg_spinlock);

  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  int i, count = 0, ret = 0;
  uint32_t *pos = (uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE));

  // allocate and parse the path
  char **arr = (char **) registry_parse_path(path, &count);
  
  // did we get a valid path?
  if ((arr != NULL) && count) {
    
    // go through the hives
    for (i=0; i<count-1; i++) {
      pos = registry_find_item(pos, arr[i], REG_HIVE_SIG_S);
      if (pos == NULL)
        break;
    }
    
    // now we are at the "item" to check
    if (pos != NULL) {
      // is the item an existing cell or hive?
      ret = ((registry_find_item(pos, arr[i], REG_CELL_SIG_S) != NULL) || 
             (registry_find_item(pos, arr[i], REG_HIVE_SIG_S) != NULL)) ? 1 : 0;
    }
    
    // free the memory used by the names
    for (i=0; i<REG_NAME_DEPTH_MAX; i++)
      free(arr[i]);
    free(arr);
  }

  spin_unlock(&reg_spinlock);

  return ret;
}

// read an exisiting registry item
// ('/System' is assumed.  we start at '/System'.  Everything is a child of '/System')
// returns length of data if valid item found
// returns 0 if nothing read or item was not found
int registry_read(const char *path, void *data, size_t max_len, const REGISTRY_TYPE type) {
  
  spin_lock(&reg_spinlock);

  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  int i, count = 0, ret = 0;
  uint32_t *pos = (uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE));

  // allocate and parse the path
  char **arr = (char **) registry_parse_path(path, &count);

  // did we get a valid path?
  if ((arr != NULL) && count) {
    
    // go through the hives
    for (i=0; i<count-1; i++) {
      pos = registry_find_item(pos, arr[i], REG_HIVE_SIG_S);
      if (pos == NULL)
        break;
    }
    
    // now we are at the "item" to add/update
    if (pos != NULL) {
      pos = registry_find_item(pos, arr[i], REG_CELL_SIG_S);
      if (pos != NULL) {
        // we are at the original cell.  Get the data
        if (pos[9] == type) { // does the item match what we are trying to read?
          switch (type) {
            case RegistryTypeBool:
            case RegistryTypeInt:
            case RegistryTypeUnsigned:
              if (max_len >= 4) {
                * (uint32_t *) data = pos[11];
                ret = 4;
              } else
                ret = 0;
              break;
            case RegistryTypeIntLong:
            case RegistryTypeUnsignedLong:
              if (max_len >= 8) {
                * (uint64_t *) data = * (uint64_t *) &pos[11];
                ret = 8;
              } else
                ret = 0;
              break;
            case RegistryTypeStr:
            case RegistryTypeBin:
              if (((size_t) pos[10] * 4) < max_len) {
                memcpy(data, &pos[11], (size_t) pos[10] * 4);
                ret = (int) ((size_t) pos[10] * 4);
              } else {
                memcpy(data, &pos[11], max_len);
                ret = (int) max_len;
              }
              break;
          }
        } // else simply return 0 since type != CELL:Type
      }
    }
    
    // free the memory used by the names
    for (i=0; i<REG_NAME_DEPTH_MAX; i++)
      free(arr[i]);
    free(arr);
  }

  spin_unlock(&reg_spinlock);

  return ret;
}

// write an exisiting or create a new registry item
// ('/System' is assumed.  we start at '/System'.  Everything is a child of '/System')
// returns length of data written successful
// returns 0 if unsuccessful
int registry_write(const char *path, const void *data, size_t len, const REGISTRY_TYPE type) {
  
  spin_lock(&reg_spinlock);

  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  int i, count = 0, ret = (int) len;
  uint32_t *p, *pos = (uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE));

  // make sure len is a multiple of 4
  len = (len + 3) & ~3ull;

  // allocate and parse the path
  char **arr = (char **) registry_parse_path(path, &count);

  // did we get a valid path?
  if ((arr != NULL) && count) {
    
    // go through the hives, adding if needed
    for (i=0; i<count-1; i++) {
      p = registry_find_item(pos, arr[i], REG_HIVE_SIG_S);
      if (p == NULL)
        p = registry_add_hive(pos, arr[i], i+1);
      pos = p;
    }
    
    // now we are at the "item" to add/update
    if (registry_find_item(pos, arr[i], REG_HIVE_SIG_S) == NULL) {
      p = registry_find_item(pos, arr[i], REG_CELL_SIG_S);
      // if we find an existing item, we need to delete it incase
      //  the new data is larger than the old data
      if (p != NULL)
        registry_remove_item(p);
      pos = registry_add_cell(pos, arr[i], len / 4);

      // we are at the (original or new) cell.  Update it
      pos[9] = type; 
      pos[10] = (uint32_t) len / 4;
      switch (type) {
        case RegistryTypeExist:
          // do nothing (the data[] member is non-existent. len = 0)
          break;
        case RegistryTypeBool:
          pos[11] = (* (uint32_t *) data) != 0;
          break;
        case RegistryTypeInt:
        case RegistryTypeUnsigned:
          pos[11] = * (uint32_t *) data;
          break;
        case RegistryTypeIntLong:
        case RegistryTypeUnsignedLong:
          * (uint64_t *) &pos[11] = * (uint64_t *) data;
          break;
        case RegistryTypeStr:
        case RegistryTypeBin:
          memcpy(&pos[11], data, len);
          break;
      }
      pos += (11 + (len / 4));
      pos[0] = REG_CELL_SIG_E;
    } else {
      //printf("Found HIVE with same name a Cell: %s\n", arr[i]);
      ret = 0;
    }
    
    // update the last_mod and crc for the registry
    base->last_modified = timestamp(0);
    base->crc = 0;
    base->crc = crc32(base, base->length);
    
    // free the memory used by the names
    for (i=0; i<REG_NAME_DEPTH_MAX; i++)
      free(arr[i]);
    free(arr);
    
    // now see if the room at the end of the registry is getting low.
    //  (we make sure there is always enough room for a single entry with 
    //   the maximum size of the data (string or binary))
    //  if it is, realloc() so we have more room for next time.
    if ((base->size - base->length) < ((11 * 4) + REG_MAX_SIZE + (1 * 4))) {
      // realloc the memory
      base = (struct S_REGISTRY_BASE *) realloc(base, base->size + (1024 * 1024));
      base->size += (1024 * 1024);
      kernel_reg = (void *) base;
    }
  }

  spin_unlock(&reg_spinlock);

  return ret;
}

// remove a Cell or Hive(s) from the registry
// returns 1 if successful
// returns 0 if unsuccessful
int registry_remove(const char *path) {
  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  int i, count = 0, ret = 0;
  uint32_t *p, *pos = (uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE));

  spin_lock(&reg_spinlock);

  // allocate and parse the path
  char **arr = (char **) registry_parse_path(path, &count);

  // did we get a valid path?
  if ((arr != NULL) && count) {
    
    // go through the hives, adding if needed
    for (i=0; i<count-1; i++) {
      pos = registry_find_item(pos, arr[i], REG_HIVE_SIG_S);
      if (pos == NULL)
        break;
    }
    
    // now we are at the "item" to remove
    if (pos != NULL) {
      // if we find it as a Cell, delete it
      p = registry_find_item(pos, arr[i], REG_CELL_SIG_S);
      if (p != NULL) {
        registry_remove_item(p);
        ret = 1;
      } else {
        p = registry_find_item(pos, arr[i], REG_HIVE_SIG_S);
        if (p != NULL) {
          registry_remove_item(p);
          ret = 1;
        }
      }
    }
    
    // update the last_mod and crc for the registry
    base->last_modified = timestamp(0);
    base->crc = 0;
    base->crc = crc32(base, base->length);
    
    // free the memory used by the names
    for (i=0; i<REG_NAME_DEPTH_MAX; i++)
      free(arr[i]);
    free(arr);
  }

  spin_unlock(&reg_spinlock);

  return ret;
}

#ifdef REGISTRY_DEBUG
void dump_reg_lines(int depth) {
  while (depth--)
    printf("| ");
}

uint32_t *dump_reg_rec(uint32_t *pos, int depth) {
  uint8_t *data;
  uint32_t i, l;

  while (1) {
    if (pos[0] == REG_HIVE_SIG_S) {
      dump_reg_lines(depth-1); printf("--> Hive: (%'p)", pos);
                               printf(" Name: '%s'", (char *) &pos[1]);
                               printf(" Depth: %i", pos[9] /*, (pos[9] == depth) ? "correct" : "incorrect"*/);
                               printf(" Reserved: %08X\n", pos[10]);
      if ((pos[11] == REG_HIVE_SIG_S) || (pos[11] == REG_CELL_SIG_S))
        pos = dump_reg_rec(&pos[11], depth + 1);
      else
        pos += 11;
      if (pos[0] != REG_HIVE_SIG_E)
        break;
      pos++;
    } else if (pos[0] == REG_CELL_SIG_S) {
      dump_reg_lines(depth-1); printf("--> Cell: (%'p)", pos);
                               printf(" Name: '%s',", (char *) &pos[1]);
                               printf(" Type: %i,", pos[9]);
                               printf(" Length: %5i", pos[10] * 4);
      if (pos[10] > 0) {
        data = (uint8_t *) &pos[11];
        printf(", Data: ");
        l = ((pos[10] * 4) < 32) ? (pos[10] * 4) : 32;
        for (i=0; i<l; i++)
          printf("%02X ", data[i]);
        if (l < (pos[10] * 4))
          printf("...");
      }
      puts("");

      pos += 11 + (size_t) pos[10];
      if (pos[0] != REG_CELL_SIG_E)
        break;
      pos++;
    } else
      break;
  }
  
  return pos;
}

void fdebug(const void *addr, size_t size) {
  uint32_t offset = 0;
  uint8_t *buf = (uint8_t *) addr;
  uint8_t *temp_buf;
  unsigned i;
  
  while (size) {
    printf(" %08X  ", offset);
    offset += 16;
    temp_buf = buf;
    for (i=0; (i<16) && (i<size); i++)
      printf("%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    for (; i<16; i++)
      printf("   ");
    printf("   ");
    for (i=0; (i<16) && (i<size); i++) {
      putchar(isprint(*buf) ? *buf : '.');
      buf++;
    }
    size -= i;
    puts("");
  }
}

void dump_registry(void) {
  struct S_REGISTRY_BASE *base = (struct S_REGISTRY_BASE *) kernel_reg;
  uint32_t crc, org_crc;
  
  // making sure to preserve original crc, then calculate so we can compare
  org_crc = base->crc;
  base->crc = 0;
  crc = crc32(base, base->length);
  base->crc = org_crc;
  
  // print the base structure data
  puts("\n");
  printf("Base:  %p\n", base);
  printf("    Magic: %08X (%s)\n", base->magic, (base->magic == REG_BASE_SIG_S) ? "correct" : "incorrect");
  printf("      CRC: %08X (%s)\n", base->crc, (base->crc == crc) ? "correct" : "incorrect");
  printf("  version: %i.%i\n", base->version >> 16, base->version & 0xFFFF);
  printf("  padding: %08X\n", base->padding);
  printf("     size: %I64i\n", base->size);
  printf("   length: %I64i\n", base->length);
  printf("  last_modified: %I64i\n", base->last_modified);
  
  // call the dump routine
  uint32_t *pos = dump_reg_rec((uint32_t *) ((uint8_t *) base + sizeof(struct S_REGISTRY_BASE)), 1);
  if (*pos != REG_BASE_SIG_E)
    printf("Ending tag was not found:  %08X\n", *pos);
  
  // dump the binary to the screen "DOS DEBUG" style.
  fdebug(base, base->length);
}
#endif
