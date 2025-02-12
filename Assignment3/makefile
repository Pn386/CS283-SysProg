CC=gcc
CFLAGS=-Wall

dsh: dsh_cli.c dshlib.c dragon.c
	$(CC) $(CFLAGS) -o dsh dsh_cli.c dshlib.c dragon.c

clean:
	rm -f dsh