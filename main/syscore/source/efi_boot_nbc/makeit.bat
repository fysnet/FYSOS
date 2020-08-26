@echo off
del BOOTIA32.EFI
nbc boot.c -fasm -efi
fasm boot.asm
ren boot.efi BOOTIA32.EFI