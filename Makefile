CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c2x -g
TARGET = tig
SRC = main.c path.c object-file.c error.c
OBJ = $(SRC:.c=.o)
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
