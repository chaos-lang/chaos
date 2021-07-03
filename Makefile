SHELL=/bin/bash

.PHONY: help.h
.ONESHELL:

UNAME_S := $(shell uname -s)

default:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-s -O3'
	${MAKE} chaos

requirements:
	./requirements.sh

requirements-dev:
	mkdir -p /usr/local/include/chaos
	rsync -av utilities/ /usr/local/include/chaos/utilities/
	rsync -av lexer/ /usr/local/include/chaos/lexer/
	rsync -av parser/ /usr/local/include/chaos/parser/
	rsync -av vm/ /usr/local/include/chaos/vm/
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
	export CHAOS_EXTRA_FLAGS='-s -O3'
	${MAKE} chaos

dev:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-Og -ggdb'
	${MAKE} chaos

clang-dev:
	export CHAOS_COMPILER=clang
	export CHAOS_EXTRA_FLAGS='-Og -ggdb'
	${MAKE} chaos

prof:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-O3 -fprofile-generate'
	${MAKE} chaos

prof-use:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-s -O3 -fprofile-use'
	${MAKE} chaos

bench:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-pg'
	${MAKE} chaos

clang-dev-sanitizer-memory:
	export CHAOS_COMPILER='clang -fsanitize=memory -fsanitize-memory-track-origins=2 -fno-optimize-sibling-calls -g'
	export CHAOS_EXTRA_FLAGS='-Og -ggdb'
	${MAKE} chaos

clang-dev-sanitizer-address:
	export CHAOS_COMPILER='clang -fsanitize=address -fno-omit-frame-pointer -g '
	export CHAOS_EXTRA_FLAGS='-Og -ggdb'
	${MAKE} chaos

clang-dev-sanitizer-undefined_behavior:
	export CHAOS_COMPILER='clang -fsanitize=undefined -g'
	export CHAOS_EXTRA_FLAGS='-Og -ggdb'
	${MAKE} chaos

clang-coverage:
	export CHAOS_COMPILER='clang -fprofile-instr-generate -fcoverage-mapping'
	${MAKE} chaos

free-debug:
	export CHAOS_COMPILER=gcc
	export CHAOS_EXTRA_FLAGS='-DCHAOS_DEBUG -Og -ggdb'
	${MAKE} chaos

help.h:
	xxd -i help.txt > help.h

parser.tab.c parser.tab.h:
	bison -Wconflicts-rr -Wno-conflicts-sr --report=state --report-file=bison_report.txt --graph=bison_graph.txt --xml=bison_xml.xml -d parser/parser.y

lex.yy.c:
	flex lexer/lexer.l

jit-backend:
	cd myjit
	${MAKE} jitlib-core.o
	${MAKE} myjit-disassembler
	cp myjit-disasm ..

chaos: lex.yy.c parser.tab.c parser.tab.h myjit
	${CHAOS_COMPILER} -c -g -Werror -Wall -fcommon -DCHAOS_INTERPRETER parser.tab.c lex.yy.c parser/*.c utilities/*.c ast/*.c vm/*.c interpreter/*.c compiler/*.c Chaos.c ${CHAOS_EXTRA_FLAGS} && \
	${CHAOS_COMPILER} -o chaos -g -Wall -std=c99 -pedantic *.o myjit/jitlib-core.o -lreadline -lm -L/usr/local/opt/readline/lib -I/usr/local/opt/readline/include -ldl

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

coverage:
	./coverage.sh

test:
	./tests/interpreter.sh

test-no-shell:
	./tests/interpreter.sh --no-shell

test-compiler:
	./tests/compiler.sh

test-compiler-dev:
	./tests/compiler.sh -e "-ggdb"

test-compiler-dev-sanitizer-memory:
	./tests/compiler.sh -e "-fsanitize=memory -fsanitize-memory-track-origins=2 -fno-optimize-sibling-calls -g -ggdb"

test-compiler-dev-sanitizer-address:
	./tests/compiler.sh -e "-fsanitize=address -fno-omit-frame-pointer -g -O0 -ggdb"

test-compiler-dev-sanitizer-undefined_behavior:
	./tests/compiler.sh -e "-fsanitize=undefined -g -O0 -ggdb"

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

test-ast:
	./tests/ast.sh

memcheck:
	./tests/memcheck.sh

memcheck-compiler:
	./tests/memcheck_compiler.sh

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

benchmark:
	./benchmark.sh

profile:
	./profile.sh
