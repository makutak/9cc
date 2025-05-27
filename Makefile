CFLAGS=-std=c11 -g -static
SRCDIR=src
TESTDIR=test
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:$(SRCDIR)/%.c=$(SRCDIR)/%.o)
CTAGS := /usr/bin/ctags
CSCOPE := /usr/bin/cscope

9cc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): $(SRCDIR)/9cc.h

test: 9cc
	./9cc ./test/test.c > tmp.s
	gcc -static -o tmp tmp.s
	./tmp

clean:
	rm -f 9cc $(SRCDIR)/*.o *~ tmp*

tags:
	$(CTAGS) -e -R .

cscope:
	$(CSCOPE) -Rbq

update: tags cscope

.PHONY: test clean tags cscope update
