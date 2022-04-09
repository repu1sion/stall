# remember to place *.c files before libs!!!

CC = gcc
HEADERS = -I/usr/include/SDL
LIBS = -L/usr/lib/x86_64-linux-gnu/ -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image
#LIBS = -L/usr/lib -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image 
TARGET = stall

all:
	$(CC) $(HEADERS) stall_main.c stall_ai.c stall_render.c $(LIBS) -o $(TARGET)

clean:
	rm $(TARGET)
#	rm *.o 
