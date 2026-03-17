CC = gcc217
CFLAGS = -g

all: testsymtablelist

clean:
	rm -f testsymtablelist *.o

clobber: clean
	rm -f *~ \#*\#

testsymtablelist: testsymtable.o symtablelist.o
	$(CC) $(CFLAGS) testsymtable.o symtablelist.o -o testsymtablelist

testsymtable.o: testsymtable.c symtable.h
	$(CC) $(CFLAGS) -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	$(CC) $(CFLAGS) -c symtablelist.c
