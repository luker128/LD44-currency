PROJECT = currency_editor
CC = g++
EMCC = emcc
SRC = *.cpp
INC = *.h

.PHONY: native
.PHONY: run
.PHONY: all
.PHONY: web

all: run

web:
	$(EMCC) -std=c++11 $(SRC) -O2 -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file data -o html/game.html
#	$(EMCC) main.cpp -O2 -s TOTAL_MEMORY=67108864 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file assets -o sdl_hello.js

native: $(PROJECT)
$(PROJECT): $(SRC) $(INC)
	$(CC) -g $(SRC) -O2 -lSDL2 -lGL -lGLU -o $@

run: native
	./$(PROJECT)
