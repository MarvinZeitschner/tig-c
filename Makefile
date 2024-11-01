CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c2x -g
TARGET = tig
SRC = main.c path.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)
