CC = gcc
CFLAGS = -std=c99 -O0
LDLIBS = -lncursesw -lasound
ERRFLAGS = -Wall -Wextra
DEBUGFLAGS = -g -DDEBUG
SRC = main.c chip8.c ui.c
OUT = chip8

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDLIBS) $(ERRFLAGS) -o $(OUT)

debug: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDLIBS) $(ERRFLAGS) $(DEBUGFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)
