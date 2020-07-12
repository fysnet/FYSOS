; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;    Copyright (c) 1984-2015    Forever Young Software  Benjamin David Lunt
;
; This code is intended for use with the book it accompanies.
; You may use this code for that purpose only.  You may modify it,
;  and/or include it within your own code as long as you do not
;  distribute it.
; You may not distribute this code to anyone with out permission
;  from the author.
;
;             -- All rights reserved -- Use at your own risk --
;
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

comment /******************************************************************\
*                            FYS OS version 2.0                            *
* FILE: mem_info.inc                                                       *
*                                                                          *
*  Built with:  NBASM ver 00.26.44                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update: 11 May 2015                                                 *
*                                                                          *
***************************************************************************/


; NBASM currently does not support nested structures, so we hardcode
;  the below structure in S_MEMORY as block[128].

S_MEMORY_ENTRY struct
               base     qword   ; the base address of this block
               size     qword   ; the size in bytes of this block
               type     dword   ; type of block
S_MEMORY_ENTRY ends

S_MEMORY   struct
           size         qword   ; size of memory in bytes
           blocks       word    ; count of bases returned
           block        dup 640 ; nbasm currently doesn't allow nested structs
S_MEMORY   ends

.end
