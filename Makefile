PROJECT = currency_editor
CXX = g++
EMCC = emcc
SRC = $(wildcard *.cpp) $(wildcard sys/*.cpp) $(wildcard gfx/*.cpp)
OBJ = $(SRC:%.cpp=%.o)
INC = *.h
DEPFILE = deps
CXXFLAGS= -O2 -std=c++11 -Isys #-Wall -Wextra
WEB_TARGET = html/game.js
WEB_LDFLAGS = -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file data --no-heap-copy #-lopenal
NATIVE_LDFLAGS = -lSDL2 -lGL -lGLU #-lopenal

.PHONY: native run all web clean

all: web

run: native
	./$(PROJECT)

web: CXX=emcc
web: $(WEB_TARGET)

native: $(PROJECT)

clean:
	rm -f $(OBJ) $(DEPFILE) html/game.* $(PROJECT)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(WEB_TARGET): $(OBJ)
	$(CXX) $(OBJ) $(WEB_LDFLAGS) -o $@

$(PROJECT): $(OBJ)
	$(CXX) $(OBJ) $(NATIVE_LDFLAGS) -o $@

.PHONY: $(DEPFILE)
$(DEPFILE):
	$(CXX) -MM $(CXXFLAGS) $(SRC) > $(DEPFILE)

include $(DEPFILE)
