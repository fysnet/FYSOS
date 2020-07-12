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
* FILE: stdio.asm                                                          *
*                                                                          *
*  Built with:  NBASM ver 00.26.44                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update: 5 May 2013                                                  *
*                                                                          *
****************************************************************************
* This is the common stdio code that is included for most .asm utilities.  *
* Make sure you don't include it at the first of you source or it will be  *
*  assembled as the start of your code.                                    *
***************************************************************************/

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine prints a char to the screen (stdout)
; On entry: 
;   al = char
; On return:
;   nothing
print_char proc near uses eax edx
           mov  dl,al
           mov  ah,6
           int  21h
           ret
print_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine prints an asciiz string to the screen (stdout)
; On entry: 
;   ds:esi-> segmented address to string
; On return:
;   nothing
print_str  proc near uses eax esi

@@:        lodsb
           or   al,al
           jz   short @f
           call print_char
           jmp  short @b

@@:        ret
print_str  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a decimal value to the screen using a temp buffer to right align it.
; It also will put comma's at every third character
; On entry: 
;   eax = value to print
;   ebx = TRUE = right align
; On return:
;   nothing
prt_dec32  proc near uses eax ecx edx edi esi
           
           ; clear out the print buffer
           push eax
           mov  edi,offset _temp_buff
           mov  al,20h
           mov  ecx,14
           rep
             stosb
           xor  al,al                   ; put a null as the last character
           stosb                        ;
           dec  edi                     ; point to the position just before the '$'
           dec  edi                     ;
           pop  eax                     ; restore the value to print
           
           push ebx
           mov  ecx,3                   ; 3 characters per thousand
           mov  ebx,10
@@:        xor  edx,edx
           div  ebx                     ; Divide by 10
           add  dl,'0'                  ; Convert to ASCII
           mov  [edi],dl 
           dec  edi
           or   eax,eax                 ; Are we done?
           jz   short @f                ; 
           loop @b
           mov  byte [edi],','          ; comma every 3rd character
           dec  edi
           mov  ecx,3
           jmp  short @b
           
@@:        pop  ebx 
           mov  esi,offset _temp_buff   ; pointer to buffer
           or   ebx,ebx                 ; if bx != FALSE, print it
           jnz  short @f                ; 
           lea  esi,[edi+1]             ; else point to the left most character
@@:        call print_str               ; print it
           ret
prt_dec32  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a hexidecimal value to the screen
; On entry: 
;   eax = value to print
; On return:
;   nothing
prt_hex32  proc near uses eax ecx
           mov  ecx,8
@@:        rol  eax,4
           push eax
           and  al,0Fh
           daa
           add  al,0F0h
           adc  al,40h
           call print_char
           pop  eax
           loop @b
           ret
prt_hex32  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a hexidecimal value to the screen
; On entry: 
;   ax = value to print
; On return:
;   nothing
prt_hex16  proc near uses eax ecx
           mov  cx,4
@@:        rol  ax,4
           push ax
           and  al,0Fh
           daa
           add  al,0F0h
           adc  al,40h
           call print_char
           pop  ax
           loop @b
           ret
prt_hex16  endp

_temp_buff dup 16,0

.end
  