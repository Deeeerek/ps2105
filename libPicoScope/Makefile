CC        = gcc
CC_FLAGS += -std=c99
INC_DIR  += /usr/local/include

all: libPicoScope.a

libPicoScope.a: libPicoScope-data.o libPicoScope.o
	ar -r libPicoScope.a libPicoScope.o libPicoScope-data.o

%.o: %.c
	$(CC) $(CC_FLAGS) -I $(INC_DIR) -c $<

clean:
	@$(RM) *.o
	@$(RM) *.a

