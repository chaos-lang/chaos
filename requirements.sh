#!/bin/bash

APT_CMD=$(which apt)
APT_GET_CMD=$(which apt-get)
DNF_CMD=$(which dnf)
YUM_CMD=$(which yum)
PACMAN_CMD=$(which pacman)
PKG_CMD=$(which pkg)

if [[ ! -z $APT_CMD ]]; then
    apt install -y build-essential clang bison flex libreadline-dev valgrind
elif [[ ! -z $APT_GET_CMD ]]; then
    apt-get install -y build-essential clang bison flex libreadline-dev valgrind
elif [[ ! -z $DNF_CMD ]]; then
    dnf install -y make clang flex bison readline-devel valgrind
elif [[ ! -z $YUM_CMD ]]; then
    yum install -y make clang flex bison readline-devel valgrind
else
    echo "Error: Automatic detection of default package manager is failed!"
    exit 1;
fi
