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
 
 ;  Last updated: 19 July 2020

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set_timer(count)
; Sets the internal Timer Tick
; On entry:  (count)
; On return: nothing
set_timer  proc near
           
           push ebp
           mov  ebp,esp
           
           xor  edx,edx
           mov  eax,1193182
           mov  ecx,PARAM0
           div  ecx
           
           push 34h              ; channel 0, R/W, 0-7 first, then 8-15, mode 2 (rate generator), binary
           push 43h              ; 
           call outpb            ; saves all registers used
           
           push eax              ; low 8 bits first
           push 40h
           call outpb            ; saves all registers used
           
           shr  eax,8            ; high 8 bits last
           push eax
           push 40h
           call outpb            ; saves all registers used
           
           pop  ebp
           ret  4
set_timer  endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set_clock(S_TIME *time)
; Sets our internal clock tick counter
; On entry:  (S_TIMET *time)
; On return: nothing
set_clock  proc near
           
           push ebp
           mov  ebp,esp
           push ebx
           
           push esi
           mov  esi,PARAM0
           
           ; ebx = year - 1980
           movzx eax,word [esi+S_TIME->year]
           sub  eax,1980
           mov  ebx,eax
           
           ; time_stamp = (year * 365)
           mov  ecx,365
           mul  ecx
           mov  [time_stamp+0],eax
           mov  [time_stamp+4],edx
           
           ; time_stamp += (year / 4) + 1 /*1980*/ - 1 /*2000*/ ; leap days
           push ebx
           shr  ebx,2
           add  [time_stamp+0],ebx
           adc  dword [time_stamp+4],0
           pop  ebx
           
           ; ecx=0, while (ecx < (month - 1)), ecx++
           xor  ecx,ecx
startloop: movzx eax,byte [esi+S_TIME->month]
           dec  eax
           cmp  ecx,eax
           jnb  short doneloop
           
           ; time_stamp += month_days[ecx];
           mov  eax,offset month_days
           add  eax,ecx
           movzx eax,byte [eax]
           add  [time_stamp+0],eax
           adc  dword [time_stamp+4],0
           
           ; if (((year mod 4) == 0) && (ecx == febuary))
           test ebx,03    ; ebx = year
           jnz  short @f
           cmp  ecx,1
           jne  short @f
           
           ;  time_stamp++
           add  dword [time_stamp+0],1
           adc  dword [time_stamp+4],0
           
@@:        inc  ecx
           jmp  short startloop

doneloop:  ;	time_stamp += day
           movzx eax,byte [esi+S_TIME->day]
           add  [time_stamp+0],eax
           adc  dword [time_stamp+4],0
           
           ; time_stamp *= jiffies in a day
           mov  eax,[time_stamp+0]
           mov  edx,[time_stamp+4]
           mov  ecx,8640000    ; jiffy's in a day
           call mul64_32       ; mul edx:eax * ecx
           mov  [time_stamp+0],eax
           mov  [time_stamp+4],edx
           
           ; time_stamp += (hour * 360000)
           movzx eax,byte [esi+S_TIME->hour]
           mov  ecx,360000
           mul  ecx
           add  [time_stamp+0],eax
           adc  [time_stamp+4],edx
           
           ; time_stamp += (min * 6000)
           movzx eax,byte [esi+S_TIME->min]
           mov  ecx,6000
           mul  ecx
           add  [time_stamp+0],eax
           adc  [time_stamp+4],edx
           
           ; time_stamp += (sec * 100)
           movzx eax,byte [esi+S_TIME->sec]
           mov  ecx,100
           mul  ecx
           add  [time_stamp+0],eax
           adc  [time_stamp+4],edx
           
           ; time_stamp += jiffy
           movzx eax,byte [esi+S_TIME->jiffy]
           add  [time_stamp+0],eax
           adc  dword [time_stamp+4],0
           
           pop  esi
           pop  ebx
           pop  ebp
           ret  4
set_clock  endp

month_days  db 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31


.end   
