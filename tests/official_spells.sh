#!/bin/bash

UNAME_S=$(uname -s)

test_spell() {
    if [ -d "$1" ]; then
        printf '%s\n' "Removing directory: $1"
        rm -rf "$1"
    fi
    git clone https://github.com/chaos-lang/$1.git --depth 1 && \
    cd $1/ && \
    make && \
    make test && \
    make test-compiler && \
    if [ "$UNAME_S" = "Darwin" ]; then
        make memcheck && \
        make memcheck-compiler
    fi
    make clang && \
    make test && \
    make test-compiler && \
    if [ "$UNAME_S" = "Darwin" ]; then
        make memcheck && \
        make memcheck-compiler
    fi
    cd ..
}

echo "Testing all official spells"

SPELL_LIST=(math string array json)

mkdir -p .build && \
cd .build && \

for spell in ${SPELL_LIST[*]}
do
    test_spell $spell || exit 1
done
