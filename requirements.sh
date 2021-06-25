#!/bin/bash

if [[ "$OSTYPE" == "linux"* ]]; then
    APT_CMD=$(which apt)
    APT_GET_CMD=$(which apt-get)
    YUM_CMD=$(which yum)
    DNF_CMD=$(which dnf)
    PACMAN_CMD=$(which pacman)
    APK_CMD=$(which apk)

    if [[ -n $APT_CMD ]]; then
        apt update && \
        apt install -y build-essential rsync clang bison flex libreadline-dev
    elif [[ -n $APT_GET_CMD ]]; then
        apt-get update && \
        apt-get install -y build-essential rsync clang bison flex libreadline-dev
    elif [[ -n $YUM_CMD ]]; then
        yum install -y rsync clang flex bison readline-devel
    elif [[ -n $DNF_CMD ]]; then
        dnf install -y rsync clang flex bison readline-devel
    elif [[ -n $PACMAN_CMD ]]; then
        pacman -Syu --noconfirm && \
        pacman -S --noconfirm base-devel rsync clang bison flex readline
    elif [[ -n $APK_CMD ]]; then
        apk update && \
        apk add build-base rsync clang bison flex readline-dev
    else
        echo "Error: Automatic detection of default package manager is failed!"
        exit 1;
    fi

    # Install GNU Lightning
    curl http://ftp.gnu.org/gnu/lightning/lightning-2.1.3.tar.gz -o /tmp/lightning-2.1.3.tar.gz && \
    tar -xvf /tmp/lightning-2.1.3.tar.gz -C /tmp && \
    cd /tmp/lightning-2.1.3 && \
    ./configure && \
    make && \
    make check && \
    make install && \
    cd -
elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew update
    brew install make flex bison readline
    brew link readline --force
    echo 'export PATH="/usr/local/opt/flex/bin:$PATH"' >> ~/.bash_profile
    echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.bash_profile
    echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.bash_profile
    source ~/.bash_profile
    echo 'export PATH="/usr/local/opt/flex/bin:$PATH"' >> ~/.zshrc
    echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.zshrc
    echo 'export PATH="/usr/local/opt/make/libexec/gnubin:$PATH"' >> ~/.zshrc
    source ~/.zshrc
fi
