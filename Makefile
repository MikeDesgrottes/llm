CC = gcc
CFLAGS = -Wall -Werror -g -DDEBUG_LEVEL=31 -pg -fsanitize=address  -O1 -I./include 
LDFLAGS = -fsanitize=address
SRC = src/main.c src/tokenizer.c src/utils.c
OBJ = $(SRC:.c=.o)

# Source files for unit tests
TEST_SRC =   tests/test_BPE.c tests/test_dataset.c tests/test_hash_table.c tests/test_runner.c src/tokenizer.c src/utils.c src/dataset.c src/hash_table.c
TEST_OBJ = $(TEST_SRC:.c=.o)


TARGET = build/tokenizer
TEST_TARGET = build/test_runner

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) $(LDFLAGS) -o $@


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TARGET)

run_tests: $(TEST_TARGET)
#	./$(TEST_TARGET)
