MAIN_O = $(BUILD_DIR)/main.o
DB_O = $(BUILD_DIR)/db.o

LIBS = http html sys routes
LIBS_FILES := $(patsubst %, $(BUILD_DIR)/%.a, $(LIBS))
.PHONY: all $(EXE) $(LIBS)

all: $(EXE)

$(EXE): $(LIBS) $(MAIN_O) $(DB_O)
	@for lib in $(LIBS); do $(MAKE) -C $$lib BUILD_DIR=$(BUILD_DIR); done
	@$(CXX) $(LDFLAGS) -o $@ $(MAIN_O) $(DB_O) $(LIBS_FILES)
	@echo "--> Linked: " $(EXE)

$(MAIN_O): main.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: main.cpp"

$(DB_O): db.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "--> Compiled: db.cpp"
