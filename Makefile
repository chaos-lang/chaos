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
	rsync -av utilities/ /usr/local/include/chaos/utilities/
	rsync -av lexer/ /usr/local/include/chaos/lexer/
	rsync -av parser/ /usr/local/include/chaos/parser/
	rsync -av interpreter/ /usr/local/include/chaos/interpreter/
	rsync -av compiler/ /usr/local/include/chaos/compiler/
	rsync -av ast/ /usr/local/include/chaos/ast/
	cp lex.yy.c /usr/local/include/chaos/
	cp parser.tab.h /usr/local/include/chaos/
	cp parser.tab.c /usr/local/include/chaos/
	cp enums.h /usr/local/include/chaos/
	cp Chaos.c /usr/local/include/chaos/
	cp Chaos.h /usr/local/include/
	cp LICENSE /usr/local/include/chaos/
ifeq ($(UNAME_S), Darwin)
	echo 'export C_INCLUDE_PATH="/usr/local/include"' >> ~/.bash_profile
	source ~/.bash_profile
	echo 'export C_INCLUDE_PATH="/usr/local/include"' >> ~/.zshrc
	source ~/.zshrc
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
	export CHAOS_COMPILER='clang -fsanitize=memory -fsanitize-memory-track-origins=2 -g -O0 -fno-optimize-sibling-calls'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

clang-dev-sanitizer-address:
	export CHAOS_COMPILER='clang -fsanitize=address -fno-omit-frame-pointer -g -O0'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

clang-dev-sanitizer-undefined_behavior:
	export CHAOS_COMPILER='clang -fsanitize=undefined -g -O0'
	export CHAOS_EXTRA_FLAGS=-ggdb
	${MAKE} chaos

free-debug:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS=-DCHAOS_DEBUG -ggdb
	${MAKE} chaos

help.h:
	xxd -i help.txt > help.h

parser.tab.c parser.tab.h:
	bison -Wconflicts-rr -Wno-conflicts-sr --report=state --report-file=bison_report.txt --graph=bison_graph.txt --xml=bison_xml.xml -d parser/parser.y

lex.yy.c:
	flex lexer/lexer.l

chaos: lex.yy.c parser.tab.c parser.tab.h
ifeq ($(UNAME_S), Darwin)
	export CHAOS_STACK_SIZE=-Wl,-stack_size,0x100000000
endif
	${CHAOS_COMPILER} -Werror -Wall -pedantic -fcommon ${CHAOS_STACK_SIZE} -DCHAOS_INTERPRETER -o chaos parser.tab.c lex.yy.c parser/*.c utilities/*.c ast/*.c preemptive/*.c interpreter/*.c compiler/*.c Chaos.c -lreadline -L/usr/local/opt/readline/lib -I/usr/local/opt/readline/include -ldl ${CHAOS_EXTRA_FLAGS}

clean:
	rm -rf chaos parser.tab.c lex.yy.c parser.tab.h

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
	./tests/interpreter.sh

test-no-shell:
	./tests/interpreter.sh --no-shell

test-compiler:
	./tests/compiler.sh

test-compiler-dev:
	./tests/compiler.sh -e "-ggdb"

test-compiler-dev-sanitizer-memory:
	./tests/compiler.sh -e "-fsanitize=memory -fsanitize-memory-track-origins=2 -O1 -fno-optimize-sibling-calls -ggdb"

test-compiler-dev-sanitizer-address:
	./tests/compiler.sh -e "-fsanitize=address -fno-omit-frame-pointer -g -ggdb"

test-compiler-dev-sanitizer-undefined_behavior:
	./tests/compiler.sh -e "-fsanitize=undefined -ggdb"

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

test-compiler-extensions-linux-gcc: test-extensions-linux-gcc
	chaos -c tests/extensions/test.kaos && build/main

test-compiler-extensions-linux-clang: test-extensions-linux-clang
	chaos -c tests/extensions/test.kaos && build/main

test-compiler-extensions-macos-gcc: test-extensions-macos-gcc
	chaos -c tests/extensions/test.kaos && build/main

test-compiler-extensions-macos-clang: test-extensions-macos-clang
	chaos -c tests/extensions/test.kaos && build/main

test-cli-args:
	./tests/cli_args.sh

test-official-spells:
	./tests/official_spells.sh

memcheck:
	./tests/memcheck.sh

memcheck-compiler:
	./tests/memcheck_compiler.sh

compile-dev:
	gcc -DCHAOS_COMPILER -o build/main build/main.c /usr/local/include/chaos/utilities/helpers.c /usr/local/include/chaos/ast/ast.c /usr/local/include/chaos/interpreter/errors.c /usr/local/include/chaos/interpreter/extension.c /usr/local/include/chaos/interpreter/function.c /usr/local/include/chaos/interpreter/module.c /usr/local/include/chaos/interpreter/symbol.c /usr/local/include/chaos/compiler/compiler.c /usr/local/include/chaos/compiler/lib/alternative.c /usr/local/include/chaos/Chaos.c -lreadline -L/usr/local/opt/readline/lib -ldl -I/usr/local/include/chaos/ -ggdb

rosetta-install:
	./tests/rosetta/install.sh

rosetta-install-clang:
	./tests/rosetta/install.sh clang

rosetta-test:
	./tests/rosetta/interpreter.sh

rosetta-test-compiler:
	./tests/rosetta/compiler.sh

rosetta-update:
	./tests/rosetta/update.sh
