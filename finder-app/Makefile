# Default target: build the writer application
.DEFAULT_GOAL := writer

# Define source files for the application
SRCS = writer.c
OBJS = $(SRCS:.c=.o)

# Set the default compiler and linker
CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

# Check if CROSS_COMPILE is set (for cross-compilation)
ifeq ($(origin CROSS_COMPILE), undefined)
    # Native compilation
    CROSS_COMPILE =
    CC = $(CROSS_COMPILE)gcc
    AR = $(CROSS_COMPILE)ar
else
    # Cross-compilation for ARM
    CROSS_COMPILE = aarch64-none-linux-gnu-
    CC = $(CROSS_COMPILE)gcc
    AR = $(CROSS_COMPILE)ar
endif

# Define the output binary name
TARGET = writer

# Default target to build the writer application
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete for $(TARGET)"

# Rule to build object files (.o) from .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Clean complete"

# Phony targets (not actual files)
.PHONY: clean

