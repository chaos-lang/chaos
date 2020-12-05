#!/bin/bash

echo -e "\nINFO: Test the positional arguments (interpreter)\n"
chaos tests/number.kaos && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test no arguments (REPL)\n"
echo "exit" | chaos && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with short options\n"
chaos -c tests/number.kaos && echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number && echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" && echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" -k && echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e "-ggdb" -kd && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test compilation variants with long options\n"
chaos --compile tests/number.kaos && echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number && echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" && echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" --keep && echo -e "\nOK\n\n" && \
chaos --compile tests/number.kaos --output number --extra "-ggdb" --keep --debug && echo -e "\nOK\n\n" && \

echo -e "\nINFO: Test invalid argument messages with short options\n"
chaos -c || echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o || echo -e "\nOK\n\n" && \
chaos -o number || echo -e "\nOK\n\n" && \
chaos -c tests/number.kaos -o number -e || echo -e "\nOK\n\n" && \

echo -e "\nINFO: CLI arguments are OK."
