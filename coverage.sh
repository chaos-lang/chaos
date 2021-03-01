#!/bin/bash

export LLVM_PROFILE_FILE="$PWD/.coverage/chaos-%p.profraw" && \
make clean && \
make clang-coverage && \
sudo make install && \
make test && \
make test-compiler && \
make test-extensions-linux-clang && \
make test-compiler-extensions-linux-clang && \
make test-cli-args && \
make test-official-spells && \
make rosetta-install-clang && \
make rosetta-test && \
make rosetta-test-compiler && \
llvm-profdata merge -output=chaos.profdata $PWD/.coverage/chaos-*.profraw && \
rm $PWD/.coverage/chaos-*.profraw && \
llvm-cov show /usr/local/bin/chaos -instr-profile=chaos.profdata > coverage.txt && \
llvm-cov report /usr/local/bin/chaos -instr-profile=chaos.profdata -use-color && \
llvm-cov show /usr/local/bin/chaos -instr-profile=chaos.profdata \
    ./Chaos.c \
    ./ast/*.c \
    ./compiler/*.c \
    ./interpreter/*.c \
    ./lexer/*.c \
    ./parser/*.c \
    ./utilities/*.c \
    -path-equivalence -use-color --format html > /tmp/coverage.html
