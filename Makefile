ifndef CC
CC=gcc
endif

ifndef MLGCC
MLGCC=mini-gcc
endif



FEATURES=
# FEATURES=-DIPV6 -DNETDB

CFLAGS=-Os -s -static

tinydns: *.c *.h
	$(CC) $(CFLAGS) -o tinydns src.c

withminilib: *.c *.h minilib.conf
	$(MLGCC) --config minilib.conf -o tinydns src.c

main: main.o cache.o config.o parse.o log.o help.o
	$(CC) $(CFLAGS) -o tinydns *.o 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o tinydns
