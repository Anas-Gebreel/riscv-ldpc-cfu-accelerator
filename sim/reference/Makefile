CC      ?= gcc
CFLAGS  ?= -O2 -Wall -std=c11
LDLIBS  ?= -lm

ldpc_minsum: ldpc_minsum.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

run: ldpc_minsum
	./ldpc_minsum

clean:
	rm -f ldpc_minsum
.PHONY: run clean
