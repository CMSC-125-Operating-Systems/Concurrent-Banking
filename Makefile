CC := clang
CFLAGS := -Wall -Wextra -Werror -std=c11 -pedantic -pthread -Iinclude
LDFLAGS := -pthread

SRC := src/main.c src/utils.c src/lock_mgr.c src/transaction.c src/metrics.c src/bank.c src/timer.c src/buffer_pool.c
OBJ := $(SRC:.c=.o)
BIN := bank_sim

.PHONY: all clean run tsan

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) tests/accounts.txt tests/trace_simple.txt 4 4

tsan:
	$(CC) $(CFLAGS) -fsanitize=thread $(SRC) $(LDFLAGS) -o $(BIN)-tsan

clean:
	rm -f $(OBJ) $(BIN) $(BIN)-tsan
