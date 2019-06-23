PROJECT = currency_editor
CXX = g++
EMCC = emcc
SRC = $(wildcard *.cpp) $(wildcard sys/*.cpp) $(wildcard gfx/*.cpp)
OBJ = $(SRC:%.cpp=%.o)
DEPFILES = $(SRC:%.cpp=%.d)
INC = *.h
CXXFLAGS= -g -O2 -std=c++17 -Isys #-Wall -Wextra
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
	rm -f $(OBJ) $(DEPFILES) html/game.* $(PROJECT)

%.o: %.cpp %.d
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(WEB_TARGET): $(OBJ)
	$(CXX) $(OBJ) $(WEB_LDFLAGS) -o $@

$(PROJECT): $(OBJ)
	$(CXX) $(OBJ) $(NATIVE_LDFLAGS) -o $@

#.PHONY: $(DEPFILES)
$(DEPFILES):
	$(CXX) -MM $(CXXFLAGS) $(@:%.d=%.cpp) -MT "$(@:%.d=%.o) $@" -MF $@

include $(DEPFILES)
