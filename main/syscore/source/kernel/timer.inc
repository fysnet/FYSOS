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

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the timer stuff

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
