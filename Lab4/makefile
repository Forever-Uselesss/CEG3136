# =========================================================
# STM32 Project Top-Level Makefile
# Supports Debug and Release builds
# =========================================================

# Default build type (can override with make BUILD=Release)
BUILD ?= Debug

# Valid options: Debug or Release
BUILD_DIR := $(BUILD)

# Make sure the specified build directory exists
ifeq ("$(wildcard $(BUILD_DIR))","")
$(error Build directory '$(BUILD_DIR)' not found!)
endif

# =========================================================
# Targets
# =========================================================

# Default target — build both Debug and Release
all:
	@echo ">>> Building Debug configuration..."
	@$(MAKE) -C Debug all
	@echo ">>> Building Release configuration..."
	@$(MAKE) -C Release all

# Clean target
clean:
	@echo ">>> Cleaning $(BUILD_DIR) configuration..."
	@$(MAKE) -C $(BUILD_DIR) clean

# Rebuild (clean + build)
rebuild:
	@$(MAKE) -C $(BUILD_DIR) clean
	@$(MAKE) -C $(BUILD_DIR) all

#get ASM file
asm:
	@echo ">>> Generating assembly files in Debug..."
	@$(MAKE) -C Debug all
	@for file in Debug/Src/*.o ; do \
		arm-none-eabi-objdump -S -d $$file > Debug/Src/$$(basename $$file .o).asm ; \
	done
	@echo ">>> Generating assembly files in Release..."
	@$(MAKE) -C Release all
	@for file in Release/Src/*.o ; do \
		arm-none-eabi-objdump -S -d $$file > Release/Src/$$(basename $$file .o).asm ; \
	done

# Print help
help:
	@echo "Available targets:"
	@echo "  make [BUILD=Debug|Release]      - Build project (default: Debug)"
	@echo "  make clean [BUILD=Debug|Release] - Clean build"
	@echo "  make rebuild [BUILD=Debug|Release] - Rebuild all"
	@echo "  make size [BUILD=Debug|Release]   - Show binary size"
	@echo "Example: make BUILD=Release"

.PHONY: all clean rebuild size help
