CC := clang
CFLAGS := -g

all: player-one player-two

clean:
	rm -rf player-one player-two

player-one: player-one.c socket.h
	$(CC) $(CFLAGS) -o player-one player-one.c -lpthread

player-two: player-two.c socket.h
	$(CC) $(CFLAGS) -o player-two player-two.c