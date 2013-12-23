CC       = gcc -std=c99
CC_OPT   = -g3
CWD      = $(shell pwd)
INC_DIR  = /usr/local/include
LIB_DIR  = /usr/local/lib/

all: test 

test: test.c libPicoScope.a
	$(CC) -I$(INC_DIR) -L$(LIB_DIR) $(CC_OPT) -lusb-1.0 test.c libPicoScope.a -o test

clean:
	@rm -rf test
	@rm -rf test.dSYM

