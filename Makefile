all: chaos

dev: lex.yy.c chaos.tab.c chaos.tab.h
	gcc -o chaos chaos.tab.c lex.yy.c -ggdb

chaos.tab.c chaos.tab.h: chaos.y
	bison -d chaos.y

lex.yy.c: chaos.l chaos.tab.h
	flex chaos.l

chaos: lex.yy.c chaos.tab.c chaos.tab.h
	gcc -o chaos chaos.tab.c lex.yy.c

clean:
	rm chaos chaos.tab.c lex.yy.c chaos.tab.h

install:
	mv chaos /usr/bin/
