#!/bin/bash

echo -e "\nINFO: Test the positional arguments (interpreter)\n"
chaos tests/number.kaos && echo -e "\nOK\n\n" && \
chaos -d tests/number.kaos && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test no arguments (REPL)\n"
echo "exit" | chaos && echo -e "\nOK\n\n" && \
echo "exit" | chaos -d && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with short options\n"
chaos -c tests/number.kaos && \
    [ -f build/main ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number && \
    [ -f build/number ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" && \
    [ -f build/number ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" -k && \
    [ -f build/number ] && \
    [ -f build/number.c ] && \
    [ -f build/number.h ] && \
    echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" -kd && \
    [ -f build/number ] && \
    [ -f build/number.c ] && \
    [ -f build/number.h ] && \
    echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with long options\n"
chaos --compile tests/number.kaos && \
    [ -f build/main ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number && \
    [ -f build/number ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" && \
    [ -f build/number ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" --keep && \
    [ -f build/number ] && \
    [ -f build/number.c ] && \
    [ -f build/number.h ] && \
    echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" --keep --debug && \
    [ -f build/number ] && \
    [ -f build/number.c ] && \
    [ -f build/number.h ] && \
    echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test invalid argument messages with short options\n"
chaos -c || echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o || echo -e "\nOK\n\n" && \
chaos -o number || echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e || echo -e "\nOK\n\n" && \

echo -e "\nINFO: CLI arguments are OK."
