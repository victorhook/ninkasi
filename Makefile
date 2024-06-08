# Compiler
#CC = g++
CC = aarch64-linux-gnu-g++

INCLUDES = \
	-Iinclude/mavlink/minimal  \
 	-I$(SYSROOT)/usr/include  \
	-I$(SYSROOT)/usr/include/aarch64-linux-gnu  \
	-I./mavlink/include  \
	-I./include

SYSROOT = /home/victor/coding/projects/ninkasi/sysroot

# Compiler flags
CCFLAGS = -Wall -g -std=c++17 -O0 --sysroot=$(SYSROOT) $(INCLUDES)


# Linker flags
LDFLAGS = -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib -L$(SYSROOT)/usr/lib/aarch64-linux-gnu -lpthread


# Source files directory and specific source files
SRC = src
SRCS = \
	$(SRC)/main.cpp \
	$(SRC)/mavcom.cpp \
	$(SRC)/utils.cpp \
	$(SRC)/ap.cpp


# Object files directory
BUILD_DIR = build
OBJS = $(patsubst $(SRC)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET = ninkasi

# Default rule
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	@echo
	@echo "Build finished: $(TARGET)"
	@echo
	@$(CC) $(CCFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files to object files in build directory
$(BUILD_DIR)/%.o: $(SRC)/%.cpp | $(BUILD_DIR)
	@echo "  CC $<"
	@$(CC) $(CCFLAGS) -c $< -o $@

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean rule
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

upload:
	scp $(TARGET) victor@ninkasi:/home/victor/ninkasi/

sync:
	rsync -a include victor@ninkasi:/home/victor/ninkasi/

# Phony targets
.PHONY: all clean upload sync
