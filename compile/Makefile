SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst src/%.cpp, build/%.o, $(SOURCES))  ../lib/build/*.o
EXE = build/compile
CXX = g++
CXXFLAGS = -fPIC -std=c++23 -g

.PHONY: all always

all: always $(EXE)

$(EXE): $(OBJECTS)
	@$(CXX) -o $@ $^
	@echo "--> Created: " $@

build/%.o: src/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

always:
	@mkdir -p build
