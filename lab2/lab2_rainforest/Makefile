CFLAGS = -I ./include
LIB    = ./lib/fmod/libfmodex64.so ./libggfonts.so
LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: lab2_rainforest

lab2_rainforest: lab2_rainforest.cpp ppm.cpp fmod.c log.cpp
	g++ $(CFLAGS) lab2_rainforest.cpp ppm.cpp log.cpp fmod.c -Wall -Wextra $(LFLAGS) -o lab2_rainforest

clean:
	rm -f lab2_rainforest
	rm -f *.o

