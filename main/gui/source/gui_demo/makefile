CC = gpp
CFLAGS = -O2
OBJECTS = atom.o bitline.o bitmap.o bitpixel.o button_bar.o checkbox.o                \
  core.o decorate.o drs.o examples.o filesel.o font.o gui.o gui_mb.o gui_vesa.o       \
  image_bmp.o image_gif.o image_pcx.o images.o list.o menu.o mouse.o                  \
  progress.o rectatom.o region.o scroll.o slider.o sliderbar.o strings.o taskbar.o    \
  textedit.o textual.o updown.o vector.o video.o

all: gui.exe

gui.exe : $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS) 

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del *.o
	del gui.exe
