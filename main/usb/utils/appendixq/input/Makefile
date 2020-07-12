# WinAVR cross-compiler toolchain is used here
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude
RM = del

# If you are not using ATtiny2313 and the USBtiny programmer, 
# update the lines below to match your configuration
CFLAGS = -Wall -Os -Iusbdrv -mmcu=attiny2313 -DF_CPU=12000000
OBJFLAGS = -j .text -j .data -O ihex
DUDEFLAGS = -p attiny2313 -c usbtiny

# Object files for the firmware
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o main.o

# By default, build the firmware, but do not flash
all: main.hex

# With this, you can flash the firmware by just typing "make flash" on command-line
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -v -U flash:w:$<

# This will set the FUSE of the chip
fuse:
	$(DUDE) $(DUDEFLAGS) -U lfuse:w:0xef:m

# Housekeeping if you want it
clean:
	$(RM) *.o *.hex *.elf usbdrv\*.o

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

# Main.elf requires additional objects to the firmware, not just main.o
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Without this dependance, .o files will not be recompiled if you change the config!
$(OBJECTS): usbdrv/usbconfig.h

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
