#This is a very hackish makefile. 
#I'm still a bit confused how to write them... but it works for now
#on my 64-bit Ubuntu system with proper libs installed

CC = gcc
MINGW = ./usr/bin/i586-mingw32msvc-gcc
MINGWFLAGS = -mwindows -lSDL -lSDL_ttf -I/usr/i586-mingw32msvc/include/SDL
CFLAGS32 = -g -m32 -lSDL-1.2 -lSDL_gfx -lSDL_ttf-2.0 -lSDL_image-1.2 -lSDL_mixer-1.2 -I/usr/include/SDL
CFLAGS64 = -g -m64 -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer -I/usr/include/SDL
OBJECTS32 = main32.o Sound32.o 
OBJECTS64 = main64.o Sound64.o
OBJECTS = main.o Sound.o

all : pong_linux64 pong_linux32 clean

pong_linux32 : $(OBJECTS32)
	$(CC) $(CFLAGS32) $(OBJECTS) -o pong_linux32
	
pong_linux64 : $(OBJECTS64)
	$(CC) $(CFLAGS64) $(OBJECTS) -o pong_linux64
	
main32.o : main.c main.h 
	$(CC) $(CFLAGS32) -c main.c 
	
main64.o : main.c main.h 
	$(CC) $(CFLAGS64) -c main.c 
	
Sound32.o : Sound/Sound.c Sound/Sound.h
	$(CC) $(CFLAGS32) -c Sound/Sound.c
	
Sound64.o : Sound/Sound.c Sound/Sound.h
	$(CC) $(CFLAGS64) -c Sound/Sound.c
	
#trying to figure out mingw cross compilation in makefile
mainwin.o : main.c main.h
	$(MINGW) $(MINGWFLAGS) - c main.c /usr/i586-mingw32msvc/lib/SDL_gfx.dll
	
#pong_win32.exe : 

#main.o : main.c main.h
#	$(CC) $(CFLAGS64) -c main.c
	
clean : 
	rm *.o
