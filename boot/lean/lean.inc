comment |*******************************************************************
*  Copyright (c) 1984-2021    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: lean.inc                                                           *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*   EQUates for lean.asm                                                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.80                                         *
*          Command line: nbasm lean<enter>                                 *
*                                                                          *
* Last Updated: 9 July 2021                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

; all of these are placed so that we don't have to check for
;  64k boundary read checks from the BIOS
;  (LEAN_ROOTSEG should be set on page boundary)
;  (LOADSEG should be set on 64k boundary)
;
; rootseg  ~128k = ~256 sectors
LEAN_ROOTSEG    equ  1000h          ; segment to load root dir to (0x10000)

SECT_BUFFER     equ 0BE00h
INDIRECT_BUFF   equ 0C000h


S_GUID struct
  data1  dword
  data2  word
  data3  word
  data4  dup 8
S_GUID ends


S_LEAN_SUPER struct
  checksum              dword    ; CRC of all fields in this structure
  magic                 dword    ; 0x4E41454C ('LEAN')
  fs_version            word     ; 0x0007 = 0.7
  pre_alloc_count       byte     ; 
  log_blocks_per_band   byte     ; 1 << log_blocks_per_band = blocks_per_band. Valid values are 12, 13, 14, ...
  state                 dword    ; bit 0 = unmounted?, bit 1 = error?
  guid                  dup 16   ; Globally Unique IDentifier
  volume_label          dup 64   ; can be modified by the LABEL command
  block_count           qword    ; The total number of blocks that form a file system volume
  free_block_count      qword    ; The number of free blocks in the volume. A value of zero means disk full.
  primary_super         qword    ; block number of primary super block
  backup_super          qword    ; block number of backup super block
  bitmap_start          qword    ; address of the block where the first bands bitmap starts
  root_start            qword    ; address of the block where the root directory of the volume starts, the inode number of the root directory.
  bad_start             qword    ; address of the block where the pseudo-file to track bad blocks starts.
  journal_inode         qword    ; address of the block where the pseudo-file for the journal starts (optional).
  log_block_size        byte     ; 1 << log_block_size = block size. Valid values are 8, 9, 10, ...
  padding               dup 7    ; padding
  reserved              dup 344  ; zeros
S_LEAN_SUPER ends

; this assumes that the block size is 512 bytes
; if you adjust the block size, the 'extent_size' member needs to be adjusted as well.
S_LEAN_INDIRECT struct
  checksum              dword    ; CRC of all fields in this structure
  magic                 dword    ; 0x58444E49 ('INDX')
  block_count           qword    ; total number of blocks addressable using this indirect block.
  inode                 qword    ; the inode number of this file this indirect block belongs to.
  this_block            qword    ; The address of the block storing this indirect block.
  prev_indirect         qword    ; the address of the previous indirect block.
  next_indirect         qword    ; the address of the next indirect block.
  extent_count          word     ; The number of valid extent specifications storing in the indirect block.
  resv0                 word     ; reserved
  resv1                 dword    ; reserved
  extent_start          dup (38 * sizeof(qword)) ; array of direct extents of the file.
  extent_size           dup (38 * sizeof(dword))
S_LEAN_INDIRECT ends

; 176 bytes each
LEAN_INODE_SIZE  equ  176
S_LEAN_INODE struct
  checksum              dword    ; dword sum of all fields before this one.
  magic                 dword    ; 0x45444F4E  ('NODE')
  extent_count          byte     ; count of extents in this inode struct
  reserved              dup 3    ; 
  indirect_count        dword    ; 
  links_count           dword    ; The number of hard links (the count of directory entries) referring to this file, at least 1
  uid                   dword    ; currently reserved, set to 0
  gid                   dword    ; currently reserved, set to 0
  attributes            dword    ; see below
  file_size             qword    ; file size
  block_count           qword    ; 
  acc_time              qword    ; last accessed: number of seconds elapsed since midnight of 1970-01-01
  sch_time              qword    ;              : number of seconds elapsed since midnight of 1970-01-01
  mod_time              qword    ; last modified: number of seconds elapsed since midnight of 1970-01-01
  cre_time              qword    ;       created: number of seconds elapsed since midnight of 1970-01-01
  first_indirect        qword    ; address of the first indirect block of the file.
  last_indirect         qword    ; address of the last indirect block of the file.
  fork                  qword    ; 
  extent_start          dup (6 * sizeof(qword)) ; array of direct extents of the file.
  extent_size           dup (6 * sizeof(dword))
S_LEAN_INODE ends

LEAN_ATTR_INLINEXTATTR  equ (1<<19) ; Remaining bytes after the inode structure are reserved for inline extended attributes

S_DIRENTRY_TYPE_FILE    equ  1
S_DIRENTRY_TYPE_DIR     equ  2

S_LEAN_DIRENTRY struct
  inode                 qword    ; The inode number of the file linked by this directory entry, the address of the first cluster of the file.
  type                  byte     ; see table above (0 = deleted)
  rec_len               byte     ; len of total record in 16 byte units.
  name_len              word     ; total length of name.
  name                  dup 4    ; (UTF-8) must *not* be null terminated, remaining bytes undefined if not a multiple of 8. UTF-8
S_LEAN_DIRENTRY ends

; dummy BPB to pass stuff along to loader.sys
S_BOOTBPB  struct
           OEM_Name     dup 8  ; 8 bytes for OEM Name and Version
           nBytesPerSec word   ; 512 bytes per Sector
           nSecPerClust byte   ; Sectors per Cluster
           nSecRes      word   ; Sectors reserved for Boot Record
           nFATs        byte   ; Number of FATs
           nRootEnts    word   ; Max Root Directory Entries allowed
           nSecs        word   ; Number of Logical Sectors (0B40h)
           mDesc        byte   ; Medium Descriptor Byte
           nSecPerFat   word   ; Sectors per FAT
           nSecPerTrack word   ; Sectors per Track
           nHeads       word   ; Number of Heads
           nSecHidden   dword  ; Number of Hidden Sectors
           nSecsExt     dword  ; This value used when there are more
           DriveNum     byte   ; Physical drive number
           nResByte     byte   ; Reserved (we use for FAT type (12- 16-bit)
           sig          byte   ; Signature for Extended Boot Record
           SerNum       dword  ; Volume Serial Number
           VolName      dup 11 ; Volume Label
           FSType       dup 8  ; File system type
S_BOOTBPB  ends

.end
