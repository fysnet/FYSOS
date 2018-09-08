comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs_common.inc                                                   *
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
*   code for an fysfs file system                                          *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm exfat<enter>                                *
*                                                                          *
* Last Updated: 28 Sept 2017                                               *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*   These are the common routines that are included with all FYSFS         *
*   utilities.                                                             *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the name of the current chain in ds:si
;  on entry:
;   ds:di-> 256 byte buffer to place asciiz name
;   bx = current slot to start in
;  on return
;   if this slot was a SLOT, and no errors
;     ds:di-> is filled with asciiz name.
;     carry clear
;   else
;     carry set
get_name   proc near uses eax ebx ecx dx si edi es
           ; ds:di-> passed buffer
           mov  si,di              ;
           
           xchg bx,bx
           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           
           movzx ebx,bx
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_name_error
           
           cmp  dword es:[ebx + S_FYSFS_ROOT->sig + 0],S_FYSFS_ROOT_NEW
           jne  short get_name_error
           
           ; is a SLOT, so start the name
           movzx ecx,byte es:[ebx + S_FYSFS_ROOT->namelen]
           jcxz short next_slot
           
           xor  edi,edi
@@:        mov  al,es:[ebx + edi + S_FYSFS_ROOT->name_fat]
           mov  [si],al
           inc  si
           inc  edi
           loop @b
           
next_slot: mov  ebx,es:[ebx + S_FYSFS_ROOT->name_continue]
           or   ebx,ebx
           jz   short get_name_done
           
next_cont: shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_name_error
           
           cmp  dword es:[ebx + S_FYSFS_CONT->sig + 0],S_FYSFS_CONT_NAME
           jne  short get_name_error
           
           movzx cx,byte es:[ebx + S_FYSFS_CONT->count]
           jcxz short get_name_done
           
           xor  edi,edi
@@:        mov  al,es:[ebx + edi + S_FYSFS_CONT->name_fat]
           mov  [si],al
           inc  si
           inc  di
           loop @b
           
           mov  ebx,es:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont
           
get_name_done:
           mov  byte [si],0   ; asciiz it
           clc
           ret

get_name_error:
           stc
           ret
get_name   endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the fat entries of the current chain in ds:si
;  on entry:
;   ds:edi-> 512 byte buffer to place entries
;   ebx = current slot to start in
;  on return
;   if this slot was a SLOT, all found entries are 64-bit, and no errors
;     ds:di-> 64-bit fat entries
;     eax = count of entries found
;     carry clear
;   else
;     carry set
get_fat_entries proc near uses ebx ecx edx esi edi ebp es

           ; starting count
           xor  ebp,ebp

           ; ds:edi-> passed buffer
           mov  esi,edi            ;

           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  get_fat_error
           
           cmp  dword es:[ebx + S_FYSFS_ROOT->sig],S_FYSFS_ROOT_NEW
           jne  get_fat_error
           
           ; is a SLOT, so start the fat
           movzx ecx,byte es:[ebx + S_FYSFS_ROOT->fat_entries]
           .adsize
           jcxz short next_slot_fat
           
           add  ebp,ecx
           
           ; calculate start of entries.
           lea  edi,es:[ebx + S_FYSFS_ROOT->name_fat]
           movzx eax,byte es:[ebx + S_FYSFS_ROOT->namelen]
           add  eax,3
           and  eax,(~3)
           add  edi,eax
get_loop0: mov  eax,es:[edi]
           mov  [esi],eax
           add  edi,4
           add  esi,4
           xor  eax,eax
           test word es:[ebx + S_FYSFS_ROOT->flags],FYSFS_LARGE_FAT
           jz   short @f
           mov  eax,es:[edi]
           add  edi,4
@@:        mov  [esi],eax
           add  esi,4
           loop get_loop0

next_slot_fat:
           mov  ebx,es:[ebx + S_FYSFS_ROOT->fat_continue]
           or   ebx,ebx
           jz   short get_fat_done
           
next_cont_fat:
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_fat_error
           
           cmp  dword es:[ebx + S_FYSFS_CONT->sig],S_FYSFS_CONT_FAT
           jne  short get_fat_error
           
           movzx ecx,byte es:[ebx + S_FYSFS_CONT->count]
           jcxz short get_fat_done
           
           add  ebp,ecx
           
           xor  edi,edi
get_loop1: mov  eax,es:[ebx + edi + S_FYSFS_CONT->name_fat]
           mov  [esi],eax
           add  edi,4
           add  esi,4
           xor  eax,eax
           test word es:[ebx + S_FYSFS_CONT->flags],FYSFS_LARGE_FAT
           jz   short @f
           mov  eax,es:[edi]
           add  edi,4
@@:        mov  [esi],eax
           add  esi,4
           .adsize
           loop get_loop1
           
           mov  ebx,es:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont_fat

get_fat_done:
           mov  eax,ebp
           clc
           ret

get_fat_error:
           stc
           ret
get_fat_entries endp


crc32_table      dup (256*sizeof(dword)),0   ; CRC lookup table array.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Initialize the crc32 code
crc32_initialize proc near uses alld
           
           ; 256 values representing ASCII character codes.
           xor  ecx,ecx
           mov  edi,offset crc32_table
           
init_loop: mov  ebx,8
           call crc32_reflect
           shl  eax,24
           mov  [edi+ecx*4],eax
           
poly_loop: mov  eax,[edi+ecx*4]
           mov  edx,eax
           shl  edx,1
           mov  edx,04C11DB7h
           jc   short @f
           xor  edx,edx
@@:        shl  eax,1
           xor  eax,edx
           mov  [edi+ecx*4],eax
           dec  ebx
           jnz  short poly_loop
           
           push ecx
           mov  ecx,[edi+ecx*4]
           mov  ebx,32
           call crc32_reflect
           pop  ecx
           mov  [edi+ecx*4],eax
           
           inc  ecx
           cmp  ecx,256
           jb   short init_loop
           
           ret
crc32_initialize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Reflection is a requirement for the official CRC-32 standard.
;  You can create CRCs without it, but they won't conform to the standard.
; on entry:
;  ebx = char
;  ecx = reflect
; on exit
;  eax = result
crc32_reflect proc near uses ebx ecx edx

           mov  edx,ecx   ; edx = reflect
           mov  ecx,ebx   ; ecx = char
           mov  ebx,1
           
           xor  eax,eax
           
           ; swap bit 0 for bit 7 bit 1 For bit 6, etc....
reflect_l: shr  edx,1
           jnc  short @f
           
           push ecx
           push edx
           sub  ecx,ebx
           mov  edx,1
           shl  edx,cl
           or   eax,edx
           pop  edx
           pop  ecx
           
@@:        inc  ebx
           cmp  ebx,ecx
           jbe  short reflect_l
           
           ret
crc32_reflect endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; check the CRC of the slot
; on entry
;  es:ebx-> slot
;  edi = offset of crc field
; on exit
;  zero flag set if crc is okay
chk_crc    proc near uses eax ebx ecx
           
           mov  dl,es:[ebx + edi]
           mov  byte es:[ebx + edi],0
           
           mov  eax,0FFFFFFFFh
           mov  ecx,128
           call crc32_partial
           xor  eax,0FFFFFFFFh
           
           ; restore the original crc value
           mov  es:[ebx + edi],dl
           cmp  al,dl
           
           ret
chk_crc    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; on entry:
;  eax = crc
;  es:ebx -> buffer (slot)
;  ecx = length of buffer
; on exit
;  eax = new crc
crc32_partial proc near uses ebx edx edi
           
           mov  edi,offset crc32_table
           
@@:        mov  edx,eax
           and  edx,0FFh
           xor  dl,es:[ebx]
           inc  ebx
           
           shr  eax,8
           xor  eax,[edi+edx*4]
           
           .adsize
           loop @b
           
           ret
crc32_partial endp

.end
