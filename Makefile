# CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

func.o: func.c
	$(CC) -c func.c

$(OBJS): 9cc.h

test: 9cc func.o
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*
