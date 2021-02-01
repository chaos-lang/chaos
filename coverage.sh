#!/bin/bash

export LLVM_PROFILE_FILE="chaos-%p.profraw" && \
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
mv tests/chaos-*.profraw . && \
llvm-profdata merge -output=chaos.profdata chaos-*.profraw && \
rm chaos-*.profraw && \
llvm-cov show /usr/local/bin/chaos -instr-profile=chaos.profdata > coverage.txt && \
llvm-cov report /usr/local/bin/chaos -instr-profile=chaos.profdata -use-color
