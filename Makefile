CC = emcc
SRC = *.cpp

all: run

web:
	$(CC) -std=c++11 $(SRC) -O2 -s USE_WEBGL2=1 --preload-file data -o html/game.html
#	$(CC) main.cpp -O2 -s TOTAL_MEMORY=67108864 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file assets -o sdl_hello.js

native:
	g++ -g $(SRC) -O2 -lSDL2 -lGL -lGLU

run: native
	./a.out
