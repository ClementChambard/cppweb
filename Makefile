export PROJECT_ROOT=$(realpath .)
export BUILD_DIR = $(PROJECT_ROOT)/build

export EXE = $(PROJECT_ROOT)/server

CXX_STANDARD = gnu++23
WARNING_FLAGS = -Wall -Wextra -Werror -Wno-unused-parameter
INCLUDE_FLAGS = -I$(PROJECT_ROOT)/src

export CXXFLAGS_COMMON = -std=$(CXX_STANDARD) $(INCLUDE_FLAGS) $(WARNING_FLAGS)
export CXXFLAGS_DEBUG = $(CXXFLAGS_COMMON) -g
export CXXFLAGS_RELEASE = $(CXXFLAGS_COMMON) -O4 -DRELEASE
export CXXFLAGS = $(CXXFLAGS_DEBUG)
export LDFLAGS =

.PHONY: $(EXE) all clean run always bear release clean_release

all: $(EXE)

release:
	@mkdir -p $(PROJECT_ROOT)/build/release
	@mkdir -p $(PROJECT_ROOT)/build/release_obj
	@$(MAKE) -C src BUILD_DIR=$(PROJECT_ROOT)/build/release_obj EXE=$(PROJECT_ROOT)/build/release/server CXXFLAGS="$(CXXFLAGS_RELEASE)"
	@strip $(PROJECT_ROOT)/build/release/server
	@python src/build_scripts/make_release.py

$(EXE):
	@$(MAKE) always
	@mkdir -p $(BUILD_DIR)/obj
	@$(MAKE) -C src BUILD_DIR=$(BUILD_DIR)/obj

always:
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf build/obj

clean_release:
	@rm -rf build/release_obj
	@rm -rf build/release

run: $(EXE)
	$(EXE)

bear:
	@$(MAKE) clean
	@bear --output $(BUILD_DIR)/compile_commands.json -- $(MAKE)
