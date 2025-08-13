SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(BUILD_DIR)/$(LIB_NAME)/%.o, $(SOURCES))
LIB = $(BUILD_DIR)/$(LIB_NAME).a

.PHONY: all always

all: always $(LIB)

$(LIB): $(OBJECTS)
	@ar -crs $@ $^
	@echo "--> Created: " $@

$(BUILD_DIR)/$(LIB_NAME)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

always:
	@mkdir -p $(BUILD_DIR)/$(LIB_NAME)
