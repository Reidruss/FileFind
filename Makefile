# Compiler
CC = gcc

# The name of the executable
TARGET = main

# Source file
SRC = main.c gui.c ops.c

# GTK4 flags provided by pkg-config
CFLAGS = `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4`

# Default target
all: $(TARGET)

# How to compile the program
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean up compiled files
clean:
	rm -f $(TARGET)
