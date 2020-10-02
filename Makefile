C = gcc -std=c99
CCFLAGS = -Wall -g -I./include


EXEC = main

GIT_HOOKS := .git/hooks/applied

all: external_sort.o
	$(CC) -o $(EXEC) $(CCFLAGS) $(EXEC).c external_sort.o
	rm -rf *.o

external_sort.o:
	$(CC) -c external_sort.c -I./include

clean:
	rm -rf $(EXEC) *.o
