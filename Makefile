.ONESHELL:

default:
	export CHAOS_COMPILER=gcc
	${MAKE} chaos

clang:
	export CHAOS_COMPILER=clang
	${MAKE} chaos

dev:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

clang-dev:
	export CHAOS_COMPILER=clang
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

chaos.tab.c chaos.tab.h: chaos.y
	bison -Wconflicts-rr -Wno-conflicts-sr --report=all --report-file=bison_report.txt --graph=bison_graph.txt --xml=bison_xml.xml -d chaos.y

lex.yy.c: chaos.l chaos.tab.h
	flex chaos.l

chaos: lex.yy.c chaos.tab.c chaos.tab.h
	${CHAOS_COMPILER} -Iloops -Ifunctions -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c utilities/*.c symbol.c errors.c -lreadline ${CHAOS_EXTRA_FLAGS}

clean:
	rm -rf chaos chaos.tab.c lex.yy.c chaos.tab.h

install:
	mv chaos /usr/bin/
