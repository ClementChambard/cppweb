SOURCES := $(wildcard components/*.cpp)
OBJECTS := $(patsubst components/%.cpp, build/%.o, $(SOURCES))
LIB_OBJECTS := ../build/lib/html/libhtml.a
SO = build/components.so
CXX = g++
INCLUDES := -I../lib/html/include/ -I../common
CXXFLAGS = -fPIC -std=c++23 -g ${INCLUDES}

.PHONY: all always

all: always $(SO)

$(SO): $(OBJECTS) ${LIB_OBJECTS} build/_html_components_impl.o
	@$(CXX) -shared -o $@ $^
	@echo "--> Created: " $@

build/_html_components_impl.o: _html_components_impl.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

build/%.o: components/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

always:
	@mkdir -p build
