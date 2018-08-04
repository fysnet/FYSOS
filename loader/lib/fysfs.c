
#include "ctype.h"
#include "loader.h"

#ifdef FS_FYSFS

#include "crc32.h"
#include "disks.h"
#include "malloc.h"
#include "paraport.h"
#include "string.h"
#include "windows.h"

#include "fysfs.h"

bool fysfs_data_valid = FALSE;
struct S_FYSFS_DATA fysfs_data;

bit32u fs_fysfs(const char *filename, void *target) {
  
  // have we loaded the root and fat yet
  if (!fysfs_data_valid)
    if (!fysfs_load_data(&fysfs_data))
      return 0;
  
  struct S_FYSFS_ROOT *root = (struct S_FYSFS_ROOT *) fysfs_data.root_dir;
  struct S_FYSFS_SUPER *super = (struct S_FYSFS_SUPER *) fysfs_data.super;
  
  // make sure the crc stuff is initialized and ready to use
  crc32_initialize();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // allocate a temp buffer.
  bit8u *buffer = (bit8u *) malloc(512);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // find the entry in the root
  for (int slot=0; slot < super->root_entries; slot++) {
    fysfs_get_name(slot, buffer, root);
    if (!stricmp(buffer, filename)) {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      //  Walk the (FYS)FAT chain reading all 'clusters' of the file.
      bit8u *p = (bit8u *) target;
      bit32u lsn, *fat_entries = (bit32u *) malloc(128 * SIZEOFQUAD);
      int t = 0,
          cnt = fysfs_get_fat_entries(slot, &fat_entries, 128, root);
      
      // initialize the progress bar
      win_init_progress(cnt);
      
      // This assumes that the buffer allocated for us is large enough that we
      //  can read in a whole cluster even when this would read past the end
      //  of the file.  Since we allocate 16-meg, this should be fine.
      for (int j=0; j<cnt; j++, t+=2) {
        lsn = super->data[0] + (fat_entries[t] * super->sect_clust);
        if (read_sectors(lsn, super->sect_clust, p) != super->sect_clust) {
          win_printf(main_win, "Error reading from file...");
          mfree(buffer);
          mfree(fat_entries);
          return 0;
        }
        p += (512 * super->sect_clust);
        win_put_progress(j, 0);
      }
      
      win_put_progress(cnt, 0);
      mfree(buffer);
      mfree(fat_entries);
      return root[slot].fsize[0];
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  win_printf(main_win, "Did not find file...");
  
  mfree(buffer);
  return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to retrieve the super and the root
bool fysfs_load_data(struct S_FYSFS_DATA *fysfs_data) {
  
  // read in the Super
  if (spc_key_F2)
    para_printf("FYSFS: Reading Super Block\n");
  struct S_FYSFS_SUPER *super = (struct S_FYSFS_SUPER *) malloc(512);
  if (read_sectors(16, 1, super) == 1) {
    // check the super
    bit32u org_crc = super->crc;
    super->crc = 0;
    if ((crc32(super, sizeof(struct S_FYSFS_SUPER)) == org_crc) && 
       ((super->sig[0] == 0x46595346) && (super->sig[1] == 0x53555052))) {
      super->crc = org_crc;
      int sz = ((super->root_entries * sizeof(struct S_FYSFS_ROOT)) + 511) & ~511;
      struct S_FYSFS_ROOT *root = (struct S_FYSFS_ROOT *) malloc(sz);
      sz /= 512;  // convert to sectors
      if (read_sectors(super->root[0], sz, root) == sz) {
        if (fysfs_good_crc(root) && (root->sig == S_FYSFS_ROOT_NEW)) {
          fysfs_data->super = super;
          fysfs_data->root_dir = root;
          return fysfs_data_valid = TRUE;
        }
      }
      mfree(root);
    }
  }
  
  mfree(super);
  return FALSE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// return the name of the current chain
//  on entry:
//   current slot to start in
//   512 byte buffer to place asciiz name
//   pointer to root
//  on return
//   if this slot was a SLOT, and no errors
//     buffer filled with asciiz name.
//     return TRUE
//   else
//     return FALSE
bool fysfs_get_name(int slot, char *buffer, const struct S_FYSFS_ROOT *root) {
  unsigned len, buffer_len = 511;
  
  if ((root[slot].sig == S_FYSFS_ROOT_NEW) && fysfs_good_crc(&root[slot])) {
    len = (root[slot].namelen < buffer_len) ? root[slot].namelen : buffer_len;
	  memcpy(buffer, root[slot].name_fat, len);
    buffer += len;
    buffer_len -= len;
	  slot = root[slot].name_continue;
    while (slot && (buffer_len > 0)) {
      struct S_FYSFS_CONT *cont = (struct S_FYSFS_CONT *) &root[slot];
      if (fysfs_good_crc(cont) && (cont->sig == S_FYSFS_CONT_NAME)) {
        len = (cont->count < buffer_len) ? cont->count : buffer_len;
	      memcpy(buffer, cont->name_fat, len);
	      buffer += len;
        buffer_len -= len;
    	  slot = cont->next;
      } else
        break;
    }
  }
  
	*buffer = '\0';
	
	return TRUE;
}

bool fysfs_good_crc(void *slot) {
  struct S_FYSFS_ROOT *root = (struct S_FYSFS_ROOT *) slot;
  bit8u org_crc = root->crc;
  root->crc = 0;
  bool ret = (org_crc == (bit8u) crc32(root, sizeof(struct S_FYSFS_ROOT)));
  root->crc = org_crc;
	
  return ret;
}

// retrieves the FAT entries of the file
// will span continuation slots
// on entry:
//     slot = starting slot number
//  buffer -> buffer to hold entries
//  buf_cnt = count of entries current buffer will hold
//    root -> root buffer
// returns a list of 64-bit cluster numbers in buffer.
// please note that buffer can and possibly will be changed.
int fysfs_get_fat_entries(int slot, bit32u **buffer, int buf_cnt, const struct S_FYSFS_ROOT *root) {
  int i, t = 0, next, cnt = 0;
  
  // get the ones in the SLOT entry first
  for (i=0; i<root[slot].fat_entries; i++) {
    bit32u *p = (bit32u *) (root[slot].name_fat + ((root[slot].namelen + 3) & ~0x03));
    // The square brackets operator takes precedence over the pointer dereference operator,
    //  therefore we must use parentheses to get the address first.
    (*buffer)[t++] = p[i];
    (*buffer)[t++] = 0;  // can't have large cluster numbers in the 1st SLOT
    if (++cnt == buf_cnt) {
      buf_cnt += 128;
      *buffer = (bit32u *) mrealloc(*buffer, buf_cnt * SIZEOFQUAD);
    }
  }
  
  // now the chain of CONT entries
  next = root[slot].fat_continue;
  while (next) {
    struct S_FYSFS_CONT *cont = (struct S_FYSFS_CONT *) &root[next];
    
    // check a few things
    if ((cont->sig == S_FYSFS_CONT_FAT) && fysfs_good_crc(cont) && cont->count) {
      bit32u *p = (bit32u *) cont->name_fat;
      for (i=0; i<cont->count; i++) {
        // The square brackets operator takes precedence over the pointer dereference operator,
        //  therefore we must use parentheses to get the address first.
        (*buffer)[t++] = *p++;
        (*buffer)[t++] = (cont->flags & CONT_FLAGS_LARGE)? *p++ : 0;
        if (++cnt == buf_cnt) {
          buf_cnt += 128;
          *buffer = (bit32u *) mrealloc(*buffer, buf_cnt * SIZEOFQUAD);
        }
      }
    } else
      break;
    
    // get the next CONT slot
    next = cont->next;
  }
  
  return cnt;
}

#endif  // FS_FYSFS
