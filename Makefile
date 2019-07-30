CC=g++
CFLAGS=-std=c++17 -pedantic -Wall -Wextra
LDFLAGS=-lSDL2

all: snake

snake:
	$(CC) $(CFLAGS) Snake.cpp -o snake $(LDFLAGS)

clean:
	rm snake
