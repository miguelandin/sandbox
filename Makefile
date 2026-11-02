# Get the architecture of the system using shell command
ARCH := $(shell uname -m)

# Basic project structure directories
BIN_DIR = bin
OBJ_DIR = obj
INC_DIR = include
SRC_DIR = src

# Compiler and standard flags
CXX = g++
# We enforce C++23 as requested and require basic POSIX libraries
CXXFLAGS = -std=c++2b -Wall -Wextra -I$(INC_DIR)
LDFLAGS = 

# Common source files that compile on all architectures
COMMON_SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Tracer.cpp
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Arch-specific variables
ARCH_SRCS =
ARCH_OBJS =

# Conditional compilation based on architecture
ifeq ($(ARCH),x86_64)
    ARCH_SRCS += $(SRC_DIR)/ArchHandler_x86_64.cpp
    # Add any specific compiler flags for x86_64 if needed
    # CXXFLAGS += -m64
else ifeq ($(ARCH),aarch64)
    ARCH_SRCS += $(SRC_DIR)/ArchHandler_arm64.cpp
    # Add any specific compiler flags for ARM64 if needed
else
    $(error Unsupported architecture: $(ARCH). Only x86_64 and aarch64 are currently supported for this sandbox.)
endif

# Convert arch-specific sources to object files
ARCH_OBJS = $(ARCH_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Target binary
TARGET = $(BIN_DIR)/sandbox

# Default rule
all: directories $(TARGET)

# Rule to create necessary directories
directories:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Rule to link the target binary
$(TARGET): $(COMMON_OBJS) $(ARCH_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile common object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories
