CC=g++
CFLAGS=-std=c++17 -pedantic -Wall -Wextra -O3
LDFLAGS=-lSDL2

all: snake

snake:
	$(CC) $(CFLAGS) Snake.cpp -o snake $(LDFLAGS)

clean:
	rm snake
