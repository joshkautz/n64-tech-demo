BUILD_DIR = build
SOURCE_DIR = src
include $(N64_INST)/include/n64.mk

# Project name - this will be the name of your .z64 ROM file
PROJECT_NAME = n64-tech-demo

# Source files (relative to SOURCE_DIR)
SRCS = main.c parallax.c

# Compiler flags
# Libdragon requires GNU extensions
N64_CFLAGS += -std=gnu11

# Build mode: release (default) or debug
ifndef DEBUG
  N64_CFLAGS += -O2
else
  N64_CFLAGS += -g -O0
endif

# Build the ROM
all: $(PROJECT_NAME).z64

# Build the ELF from object files
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Configure the ROM
$(PROJECT_NAME).z64: N64_ROM_TITLE="N64 Tech Demo"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).elf

# Debug build target
debug:
	$(MAKE) DEBUG=1

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(PROJECT_NAME).z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean debug
