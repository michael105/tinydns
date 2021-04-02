ifndef CC
CC=gcc
endif

CFLAGS=-Os -s -static

tinydns: *.c
	$(CC) $(CFLAGS) -o tinydns src.c

main: main.o cache.o config.o parse.o log.o help.o
	$(CC) $(CFLAGS) -o tinydns *.o 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o tinydns
