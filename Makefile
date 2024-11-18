CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c2x -g -Icommands -I.
TARGET = tig
SRC = main.c path.c object-file.c error.c strbuf.c alloc.c tig.c
COMMANDS_DIR = commands
COMMANDS_SRC = $(wildcard $(COMMANDS_DIR)/*.c)
OBJ = $(SRC:.c=.o) $(COMMANDS_SRC:.c=.o)
LDFLAGS = -lz

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)
