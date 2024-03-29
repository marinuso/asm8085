CC = gcc

CFLAGS = -Wall -Wextra -O2

CFILES = $(shell ls *.c | grep -v asm8085.c)
OBJ = $(CFILES:.c=.o)
TESTS = $(shell ls tests/*.h)

all: asm8085 test

install: asm8085
	install asm8085 /usr/bin

asm8085: asm8085.o $(OBJ)
	$(CC) $(CFLAGS) -o asm8085 $(OBJ) asm8085.o

asm8085.o: asm8085.c asm8085.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: tests/tests
	cd tests && ./tests

tests/tests: tests/tests.c $(OBJ) $(TESTS)
	$(CC) $(CFLAGS) -g -I./tests -o tests/tests tests/tests.c $(OBJ)

parser_types.h: instructions.h

parser.h: parser_types.h

expression.h: parser_types.h operators.h


%.c: %.h 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -f *.o asm8085 tests/tests
	

