# Makefile

# Compiler settings
CC := gcc
CFLAGS := -Wall -g -O0

# Source file and build directories
SRC := src
BIN := bin

# Target executable
TARGET := $(BIN)/ipc

# Source file paths
SOURCES := $(shell find $(SRC) -name '*.c')

# Header file paths
HEADERS := $(shell find $(SRC) -name '*.h')

# Object file paths
OBJECTS := $(patsubst $(SRC)/%.c, $(BIN)/%.o, $(SOURCES))

# Dependency file paths
DEPENDENCIES := $(patsubst $(BIN)/%.o, $(BIN)/%.d, $(OBJECTS)) 

# Default rule to build everything
all: bin $(TARGET)

# Rule to create bin directory
bin:
	mkdir -p bin

# Rule to link object files into executable target
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

#  Rule to compile source code files into object files
$(BIN)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP

# Tells make to include the dependecy files 
-include $(DEPENDENCIES)

# Rule to remove all object files and dependency files
.PHONY: clean
clean:
	rm -rf $(BIN)