# =========================================================
# STM32 Project Top-Level Makefile
# Supports Debug and Release builds
# =========================================================

# Default build type (can override with make BUILD=Release)
BUILD ?= Debug

# Valid options: Debug or Release
BUILD_DIR := $(BUILD)

# Make sure it exists
ifeq ("$(wildcard $(BUILD_DIR))","")
$(error Build directory '$(BUILD_DIR)' not found!)
endif

# =========================================================
# Targets
# =========================================================

# Default target — build everything
all:
	@echo ">>> Building $(BUILD_DIR) configuration..."
	@$(MAKE) -C $(BUILD_DIR) all

# Clean target
clean:
	@echo ">>> Cleaning $(BUILD_DIR) configuration..."
	@$(MAKE) -C $(BUILD_DIR) clean

# Rebuild (clean + build)
rebuild:
	@$(MAKE) -C $(BUILD_DIR) clean
	@$(MAKE) -C $(BUILD_DIR) all

# Size information
size:
	@$(MAKE) -C $(BUILD_DIR) default.size.stdout

# Print help
help:
	@echo "Available targets:"
	@echo "  make [BUILD=Debug|Release]      - Build project (default: Debug)"
	@echo "  make clean [BUILD=Debug|Release] - Clean build"
	@echo "  make rebuild [BUILD=Debug|Release] - Rebuild all"
	@echo "  make size [BUILD=Debug|Release]   - Show binary size"
	@echo "Example: make BUILD=Release"

.PHONY: all clean rebuild size help
