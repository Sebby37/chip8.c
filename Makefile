CC = gcc
CFLAGS = -std=c99 -O0
ERRFLAGS = -Wall -Werror
LDLIBS = -lform -lncurses
DEBUGFLAGS = -g -DDEBUG
SRC = main.c chip8.c
OUT = chip8

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(ERRFLAGS) $(LDLIBS) -o $(OUT)

debug: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDLIBS) $(DEBUGFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)