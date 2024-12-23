# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c2x -Icommands -I. -I/opt/homebrew/opt/openssl/include -Iutils/compression -g
LDFLAGS = -lz -L/opt/homebrew/opt/openssl/lib -lcrypto

# Target executable and source files
TARGET = tig
SRC = main.c path.c object-file.c error.c strbuf.c alloc.c tig.c tree-file.c
COMMANDS_DIR = commands
COMMANDS_SRC = $(wildcard $(COMMANDS_DIR)/*.c)
COMPRESSION_DIR = utils/compression
COMPRESSION_SRC = $(wildcard $(COMPRESSION_DIR)/*.c)

OBJ = $(SRC:.c=.o) $(COMMANDS_SRC:.c=.o) $(COMPRESSION_SRC:.c=.o)

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
