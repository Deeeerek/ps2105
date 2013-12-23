CC     = gcc -std=c99
CC_OPT = -g3
CWD    = $(shell pwd)

all: test interpose.so dump_urb.o

test: test.c
	$(CC) $(CC_OPT) -lps2000 test.c -otest

interpose.so: interpose.c dump_urb.o
	$(CC) -shared -ldl -fPIC interpose.c dump_urb.o -o interpose.so 

dump_urb.o: dump_urb.c
	$(CC) -c dump_urb.c

.PHONY: run
run: test interpose.so
	@LD_PRELOAD=$(CWD)/interpose.so ./test > trace.txt

clean:
	@rm -rf test
	@rm -rf *.so
	@rm -rf *.o

