 ;
 ;                             Copyright (c) 1984-2020
 ;                              Benjamin David Lunt
 ;                             Forever Young Software
 ;                            fys [at] fysnet [dot] net
 ;                              All rights reserved
 ; 
 ; Redistribution and use in source or resulting in  compiled binary forms with or
 ; without modification, are permitted provided that the  following conditions are
 ; met.  Redistribution in printed form must first acquire written permission from
 ; copyright holder.
 ; 
 ; 1. Redistributions of source  code must retain the above copyright notice, this
 ;    list of conditions and the following disclaimer.
 ; 2. Redistributions in printed form must retain the above copyright notice, this
 ;    list of conditions and the following disclaimer.
 ; 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 ;    this list of  conditions and the following  disclaimer in the  documentation
 ;    and/or other materials provided with the distribution.
 ; 
 ; THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 ; AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 ; ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 ; WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 ; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 ; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 ; ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 ; PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 ; USES AS THEIR OWN RISK.
 ; 
 ; Any inaccuracy in source code, code comments, documentation, or other expressed
 ; form within Product,  is unintentional and corresponding hardware specification
 ; takes precedence.
 ; 
 ; Let it be known that  the purpose of this Product is to be used as supplemental
 ; product for one or more of the following mentioned books.
 ; 
 ;   FYSOS: Operating System Design
 ;    Volume 1:  The System Core
 ;    Volume 2:  The Virtual File System
 ;    Volume 3:  Media Storage Devices
 ;    Volume 4:  Input and Output Devices
 ;    Volume 5:  ** Not yet published **
 ;    Volume 6:  The Graphical User Interface
 ;    Volume 7:  ** Not yet published **
 ;    Volume 8:  USB: The Universal Serial Bus
 ; 
 ; This Product is  included as a companion  to one or more of these  books and is
 ; not intended to be self-sufficient.  Each item within this distribution is part
 ; of a discussion within one or more of the books mentioned above.
 ; 
 ; For more information, please visit:
 ;             http://www.fysnet.net/osdesign_book_series.htm

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Listing 5-14 from Volume 1, "The System Core"
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Get VESA Info service
; On entry: 
;   es:edi-> segmented address to store the information
;   es:esi-> segmented address to store the supported modes
;   es:ebx-> segmented address to store the OEM Name
; On return:
;   Carry clear if service supported
;
vesa_service proc near uses eax ebx ecx edx esi edi

    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; first, clear it out.
    push edi
    xor  al,al
    mov  ecx,512
    rep
      stosb
    pop  edi
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; If we want only the VESA info, we put 'VESA' in the
    ;  first dword of the buffer.  If we want VBE2+ (VBE3),
    ;  then we put 'VBE2'.
    mov  eax,00004F00h
    mov  dword es:[edi],'2EBV'
    int  10h
    jc   short vesa_done
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; on return, if the carry is clear, but ax != 0x004F
    ;  then this wasn't a good call.
    cmp  ax,004Fh
    jne  short vesa_done
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; else it was a good call, so we have a valid buffer.
    ; let's extract the supported modes. (we just 
    ; extract 128 words. The print routine stops at 0xFFFF)
    xchg edi,esi  ; es:edi -> passed pointer to buffer
                  ; es:esi -> vesa info buffer
    push ds
    push esi
    lds  si,es:[esi + 0Eh]
    mov  cx,TOT_MODES
    rep
    movsw
    pop  esi
    pop  ds
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; get the oem name.  We have to do this incase the BIOS
    ; builds the list on the fly.
    mov  edi,ebx      ; es:ebx -> buffer to hold OEM name
    push ds
    lds  si,es:[esi + 06h]
    mov  cx,32
    rep
    movsb
    xor  al,al
    stosb
    pop  ds
    
vesa_done:
    clc
    ret
vesa_service endp
