#!/bin/sh

printf "\nINFO: Test the positional arguments (interpreter)\n"
chaos tests/everything.kaos && printf "\nOK\n\n" && \
chaos -d tests/everything.kaos && printf "\nOK\n\n" && \

printf "\nINFO: Test no arguments (REPL)\n"
echo "exit" | chaos && printf "\nOK\n\n" && \
echo "exit" | chaos -d && printf "\nOK\n\n" && \

printf "\nINFO: Test compilation variants with short options\n"
chaos -c tests/everything.kaos && \
    [ -f build/main ] && \
    printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything && \
    [ -f build/everything ] && \
    printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" && \
    [ -f build/everything ] && \
    printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" -k && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" -kd && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    printf "\nOK\n\n" && \

printf "\nINFO: Test compilation variants with long options\n"
chaos --compile tests/everything.kaos && \
    [ -f build/main ] && \
    printf "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything && \
    [ -f build/everything ] && \
    printf "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" && \
    [ -f build/everything ] && \
    printf "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" --keep && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    printf "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" --keep --debug && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    printf "\nOK\n\n" && \

printf "\nINFO: Test other arguments\n"
chaos -h && chaos --help && \
chaos -v && chaos --version && \
chaos -l && chaos --license && \
chaos -u tests/everything.kaos && chaos --unsafe tests/everything.kaos && \
printf "\nOK\n\n" && \

printf "\nINFO: Test invalid argument messages with short options\n"
chaos -c || printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o || printf "\nOK\n\n" && \
chaos -o everything || printf "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e || printf "\nOK\n\n" && \

printf "\nINFO: Test other erroring arguments\n"
chaos --no_such_arg || \
chaos no_such_file.kaos || \
printf "\nOK\n\n" && \

printf "\nINFO: CLI arguments are OK."
