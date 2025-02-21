# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g  # Add -g for debugging info if needed
LDFLAGS = -pthread          # pthread support for multi-threading

# Directories
SRCDIR = .
OBJDIR = .
BINDIR = .

# Source files and object files
SRCS = $(SRCDIR)/aesdsocket.c
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = $(BINDIR)/aesdsocket

# Default targets
all: $(TARGET)

# Build the target (executable)
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Clean up the build
clean:
	rm -f $(OBJS) $(TARGET)

# For cross-compilation (optional, just an example)
# Uncomment the line below if you're cross-compiling and specify a prefix for cross-toolchain
CROSS_COMPILE=aarch64-none-linux-gnu-

# Install (optional, if you want to install the binary)
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin

# Ensure 'clean' and 'install' are always considered as separate targets
.PHONY: all clean install

