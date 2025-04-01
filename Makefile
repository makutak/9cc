CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
CTAGS := /usr/bin/ctags
CSCOPE := /usr/bin/cscope

9cc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

tags:
	$(CTAGS) -e -R .

cscope:
	$(CSCOPE) -Rbq

update: tags cscope

.PHONY: test clean tags cscope update
