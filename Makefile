CC = gcc
CFLAGS = -Wall -Werror -g

SRC = src/main.c src/tokenizer.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = build/tokenizer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

