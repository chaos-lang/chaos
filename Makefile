SHELL=/bin/bash

.ONESHELL:

default:
	export CHAOS_COMPILER=gcc
	${MAKE} chaos

requirements:
	./requirements.sh

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

clang-dev-sanitizer-memory:
	export CHAOS_COMPILER='clang -fsanitize=memory -fsanitize-memory-track-origins=2 -O1 -fno-optimize-sibling-calls'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

clang-dev-sanitizer-address:
	export CHAOS_COMPILER='clang -fsanitize=address -fno-omit-frame-pointer'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

clang-dev-sanitizer-undefined_behavior:
	export CHAOS_COMPILER='clang -fsanitize=undefined'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

chaos.tab.c chaos.tab.h: chaos.y
	bison -Wconflicts-rr -Wno-conflicts-sr --report=all --report-file=bison_report.txt --graph=bison_graph.txt --xml=bison_xml.xml -d chaos.y

lex.yy.c: chaos.l chaos.tab.h
	flex chaos.l

chaos: lex.yy.c chaos.tab.c chaos.tab.h
	${CHAOS_COMPILER} -Werror -Iloops -Ifunctions -Imodules -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c modules/*.c utilities/*.c symbol.c errors.c chaos.c -lreadline -L/usr/local/opt/readline/lib -I/usr/local/opt/readline/include -ldl ${CHAOS_EXTRA_FLAGS}

clean:
	rm -rf chaos chaos.tab.c lex.yy.c chaos.tab.h

install:
	mv chaos /usr/local/bin/

lint:
	cppcheck --force .

test:
	./tests/run.sh

test-no-shell:
	./tests/run.sh --no-shell

test-extensions-linux-gcc:
	gcc -shared -fPIC tests/extensions/spells/example.c -o tests/extensions/spells/example.so
	chaos tests/extensions/test.kaos

test-extensions-linux-clang:
	clang -shared -fPIC tests/extensions/spells/example.c -o tests/extensions/spells/example.so
	chaos tests/extensions/test.kaos

test-extensions-macos-gcc:
	gcc -shared -fPIC -undefined dynamic_lookup tests/extensions/spells/example.c -o tests/extensions/spells/example.dylib
	chaos tests/extensions/test.kaos

test-extensions-macos-clang:
	clang -shared -fPIC -undefined dynamic_lookup tests/extensions/spells/example.c -o tests/extensions/spells/example.dylib
	chaos tests/extensions/test.kaos

memcheck:
	./tests/memcheck.sh
