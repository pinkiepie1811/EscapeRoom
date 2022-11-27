CC := clang
CFLAGS := -g

all: player-one player-two

clean:
	rm -rf player-one player-two

player-one: player-one.c message.h message.c socket.h ui.c ui.h
	$(CC) $(CFLAGS) -o player-one player-one.c message.c ui.c -lform -lncurses -lpthread

player-two: player-two.c message.h message.c socket.h ui.c ui.h
	$(CC) $(CFLAGS) -o player-two player-two.c message.c ui.c -lform -lncurses -lpthread