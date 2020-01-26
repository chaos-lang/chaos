all: chaos

dev: lex.yy.c chaos.tab.c chaos.tab.h
	gcc -Iloops -Ifunctions -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c utilities/*.c symbol.c errors.c -lreadline -ggdb

chaos.tab.c chaos.tab.h: chaos.y
	bison -Wconflicts-rr -Wno-conflicts-sr --report=all --report-file=bison_report.txt --graph=bison_graph.txt --xml=bison_xml.xml -d chaos.y

lex.yy.c: chaos.l chaos.tab.h
	flex chaos.l

chaos: lex.yy.c chaos.tab.c chaos.tab.h
	gcc -Iloops -Ifunctions -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c utilities/*.c symbol.c errors.c -lreadline

clean:
	rm chaos chaos.tab.c lex.yy.c chaos.tab.h

install:
	mv chaos /usr/bin/
