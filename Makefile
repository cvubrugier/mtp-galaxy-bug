CC=gcc
CFLAGS=-Wall -Werror -ggdb
LDFLAGS=-lmtp
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

all: mtp-galaxy-bug

mtp-galaxy-bug: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f *.o

mrproper: clean
	rm -f mtp-galaxy-bug
