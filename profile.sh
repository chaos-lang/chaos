#!/bin/bash

echo "Compiling with -fprofile-generate"
make clean && \
make prof && \
sudo make install && \

echo "Generating profile... (Fibonacci)" && \
chaos tests/rosetta/fibonacci_perf.kaos && \


echo "Re-compiling with -fprofile-use" && \
make clean && \
make prof-use && \
sudo make install
