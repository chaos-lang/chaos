#!/bin/bash

install_spell() {
    if [ -d "$1" ]; then
        printf '%s\n' "Removing directory: $1"
        rm -rf "$1"
    fi
    git clone https://github.com/chaos-lang/$1.git --depth 1 && \
    cd $1/ && \
    make && \
    cd ..
}

echo "Installing all official spells"

SPELL_LIST=(math string array json)

mkdir -p tests/rosetta/spells && \
cd tests/rosetta/spells && \

for spell in ${SPELL_LIST[*]}
do
    install_spell $spell || exit 1
done

