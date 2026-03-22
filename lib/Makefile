SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, build/%.o, $(SOURCES))
LIB = build/components.a
CXX = g++
CXXFLAGS = -fPIC -std=c++23 -g

.PHONY: all always

all: always $(LIB)

$(LIB): $(OBJECTS)
	@ar -crs $@ $^
	@echo "--> Created: " $@

build/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

always:
	@mkdir -p build
