CC := gcc

TARGET := bin/emclaw
SRC := src/main.c src/functions.c
INC := -Iinc
OBJ := obj/main.o obj/functions.o

CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -D_DEFAULT_SOURCE $(INC)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) $(OBJ) -o $(TARGET)

obj/main.o: src/main.c inc/functions.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.c -o $@

obj/functions.o: src/functions.c inc/functions.h inc/drivers.h inc/pca_regs.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/functions.c -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf obj bin
