CC = clang
CFLAGS = -Wall -pedantic
LDFLAGS = 
DEBUG = -g

all : mytar

mytar: mytar.o tarFuncs.o
	$(CC) $(DEBUG) $(LDFLAGS) -o $@ $^

%.o : %.c
	$(CC) $(DEBUG) $(LDFLAGS) -c -o $@ $^

clean : 
	rm -f *.o core
