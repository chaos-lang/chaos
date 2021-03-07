#!/bin/sh

make clean && \
make bench && \
sudo make install && \

echo "Running benchmark... (Fibonacci)" && \
chaos tests/rosetta/fibonacci_perf.kaos && \
gprof /usr/local/bin/chaos gmon.out > prof_fibonacci.bench && \
less prof_fibonacci.bench

echo "Running benchmark... (Tak)" && \
chaos tests/rosetta/tak.kaos && \
gprof /usr/local/bin/chaos gmon.out > prof_tak.bench && \
less prof_tak.bench
