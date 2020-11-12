#!/bin/bash

if [[ "$OSTYPE" == "linux"* ]]; then
    APT_CMD=$(which apt)
    APT_GET_CMD=$(which apt-get)
    YUM_CMD=$(which yum)
    DNF_CMD=$(which dnf)
    PACMAN_CMD=$(which pacman)
    PKG_CMD=$(which pkg)
    APK_CMD=$(which apk)

    if [[ ! -z $APT_CMD ]]; then
        apt update && \
        apt install -y build-essential rsync clang bison flex libreadline-dev
    elif [[ ! -z $APT_GET_CMD ]]; then
        apt-get update && \
        apt-get install -y build-essential rsync clang bison flex libreadline-dev
    elif [[ ! -z $YUM_CMD ]]; then
        yum install -y rsync clang flex bison readline-devel
    elif [[ ! -z $DNF_CMD ]]; then
        dnf install -y rsync clang flex bison readline-devel
    elif [[ ! -z $PACMAN_CMD ]]; then
        pacman -Syu --noconfirm && \
        pacman -S --noconfirm base-devel rsync clang bison flex readline
    elif [[ ! -z $APK_CMD ]]; then
        apk update && \
        apk add build-base rsync clang bison flex readline-dev
    else
        echo "Error: Automatic detection of default package manager is failed!"
        exit 1;
    fi
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
