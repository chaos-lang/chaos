SHELL=/bin/bash

.ONESHELL:

UNAME_S := $(shell uname -s)

default:
	export CHAOS_COMPILER=gcc
	${MAKE} chaos

requirements:
	./requirements.sh

requirements-dev:
	mkdir -p /usr/local/include/chaos
	cp utilities/language.h /usr/local/include/chaos/
	cp utilities/platform.h /usr/local/include/chaos/
	cp enums.h /usr/local/include/chaos/
	cp Chaos.h /usr/local/include/
ifeq ($(UNAME_S), Darwin)
	echo 'export C_INCLUDE_PATH="/usr/local/include"' >> ~/.bash_profile
	source ~/.bash_profile
	gcc -xc -E -v -
endif

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
	${CHAOS_COMPILER} -Werror -Iloops -Ifunctions -Imodules -fcommon -DCHAOS_INTERPRETER -o chaos chaos.tab.c lex.yy.c loops/*.c functions/*.c modules/*.c utilities/*.c symbol.c errors.c Chaos.c -lreadline -L/usr/local/opt/readline/lib -I/usr/local/opt/readline/include -ldl ${CHAOS_EXTRA_FLAGS}

clean:
	rm -rf chaos chaos.tab.c lex.yy.c chaos.tab.h

install: requirements-dev
	mv chaos /usr/local/bin/

uninstall:
	rm /usr/local/bin/chaos
	rm /usr/local/include/Chaos.h
	rm -rf /usr/local/include/chaos/
ifeq ($(UNAME_S), Darwin)
	$(eval GCC_VERSION=$(shell gcc -dumpversion))
	$(eval GCC_MAJOR_VERSION=$(shell gcc -dumpversion | cut -d. -f1))
	rm /usr/local/Cellar/gcc@$(GCC_MAJOR_VERSION)/$(GCC_VERSION)/lib/gcc/$(GCC_MAJOR_VERSION)/gcc/x86_64-apple-darwin19/$(GCC_VERSION)/include/Chaos.h
	rm -rf /usr/local/Cellar/gcc@$(GCC_MAJOR_VERSION)/$(GCC_VERSION)/lib/gcc/$(GCC_MAJOR_VERSION)/gcc/x86_64-apple-darwin19/$(GCC_VERSION)/include/chaos/
endif

lint:
	cppcheck --force .

test:
	./tests/run.sh

test-no-shell:
	./tests/run.sh --no-shell

test-extensions-linux-gcc:
	gcc -shared -fPIC tests/extensions/spells/example/example.c -o tests/extensions/spells/example/example.so && \
	chaos tests/extensions/test.kaos && \
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos tests/extensions/test.kaos || exit 1

test-extensions-linux-clang:
	clang -shared -fPIC tests/extensions/spells/example/example.c -o tests/extensions/spells/example/example.so && \
	chaos tests/extensions/test.kaos && \
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos tests/extensions/test.kaos || exit 1

test-extensions-macos-gcc:
	gcc -shared -fPIC -undefined dynamic_lookup tests/extensions/spells/example/example.c -o tests/extensions/spells/example/example.dylib && \
	chaos tests/extensions/test.kaos

test-extensions-macos-clang:
	clang -shared -fPIC -undefined dynamic_lookup tests/extensions/spells/example/example.c -o tests/extensions/spells/example/example.dylib && \
	chaos tests/extensions/test.kaos

memcheck:
	./tests/memcheck.sh
