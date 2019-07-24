CC = gcc

CFLAGS = -Wall -Wextra

CFILES = $(shell ls *.c)
OBJ = $(CFILES:.c=.o)
TESTS = $(shell ls tests/*.h)

test: tests/tests
	./tests/tests

tests/tests: tests/tests.c $(OBJ) $(TESTS)
	$(CC) $(CFLAGS) -I./tests -o tests/tests tests/tests.c $(OBJ)

parser_types.h: instructions.h

parser.h: parser_types.h

expression.h: parser_types.h operators.h


%.c: %.h 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -f *.o asm8085 tests/tests
	

