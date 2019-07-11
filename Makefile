CC = gcc

CFILES = $(shell ls *.c)
OBJ = $(CFILES:.c=.o)
TESTS = $(shell ls tests/*.h)

test: tests/tests
	./tests/tests

tests/tests: tests/tests.c $(OBJ) $(TESTS)
	$(CC) -I./tests -o tests/tests tests/tests.c $(OBJ)

parser.c: parser.h instructions.h 

expression.c: expression.h operators.h

%.c: %.h 

%.o: %.c
	$(CC) -c -o $@ $<
	
clean:
	rm -f *.o asm8085 tests/tests
	

