# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c2x -Icommands -Iobjects -I. -I/opt/homebrew/opt/openssl/include -Iutils -g
LDFLAGS = -lz -L/opt/homebrew/opt/openssl/lib -lcrypto -lc

# Target executable and source files
TARGET = tig
SRC = main.c path.c error.c strbuf.c alloc.c tig.c
COMMANDS_DIR = commands
COMMANDS_SRC = $(wildcard $(COMMANDS_DIR)/*.c)
UTILS_DIR = utils
UTILS_SRC = $(wildcard $(UTILS_DIR)/*.c)
OBJECTS_DIR = objects
OBJECTS_SRC = $(wildcard $(OBJECTS_DIR)/*.c)

OBJ = $(SRC:.c=.o) $(COMMANDS_SRC:.c=.o) $(UTILS_SRC:.c=.o) $(STRING_SRC:.c=.o) $(OBJECTS_SRC:.c=.o)

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Rule for compiling .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Rebuild the project
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)
