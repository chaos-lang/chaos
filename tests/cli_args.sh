#!/bin/bash

echo -e "\nINFO: Test the positional arguments (interpreter)\n"
chaos tests/everything.kaos && echo -e "\nOK\n\n" && \
chaos -d tests/everything.kaos && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test no arguments (REPL)\n"
echo "exit" | chaos && echo -e "\nOK\n\n" && \
echo "exit" | chaos -d && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with short options\n"
chaos -c tests/everything.kaos && \
    [ -f build/main ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything && \
    [ -f build/everything ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" && \
    [ -f build/everything ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" -k && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e "-ggdb" -kd && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with long options\n"
chaos --compile tests/everything.kaos && \
    [ -f build/main ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything && \
    [ -f build/everything ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" && \
    [ -f build/everything ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" --keep && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/everything.kaos --output everything --extra "-ggdb" --keep --debug && \
    [ -f build/everything ] && \
    [ -f build/everything.c ] && \
    [ -f build/everything.h ] && \
    echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test other arguments\n"
chaos -h && chaos --help && \
chaos -v && chaos --version && \
chaos -l && chaos --license && \
chaos -u tests/everything.kaos && chaos --unsafe tests/everything.kaos && \
echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test invalid argument messages with short options\n"
chaos -c || echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o || echo -e "\nOK\n\n" && \
chaos -o everything || echo -e "\nOK\n\n" && \
chaos -c tests/everything.kaos -o everything -e || echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test other erroring arguments\n"
chaos --no_such_arg || \
chaos no_such_file.kaos || \
echo -e "\nOK\n\n" && \

echo -e "\nINFO: CLI arguments are OK."
