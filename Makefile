CC = cc
CFLAGS = -Wall -Wextra -std=c11

rloc: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f rloc

fmt:
	clang-format -i *.c *.h

.PHONY: clean fmt
