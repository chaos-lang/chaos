#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

TESTS_DIR=tests
BUILD_DIR=build

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"

    if [[ "$testname" == "exit_"* ]]; then
        continue
    fi

    SKIP_TESTS="syntax_error"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo -e "\n\n(compiler) Running memcheck: chaos -c ${TESTS_DIR}/${testname}\n"

    if [[ "$OSTYPE" == "linux"* ]]; then
        valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos -c $DIR/$filename -o $testname || exit 1
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        /tmp/DrMemory/bin64/drmemory -no_follow_children -exit_code_if_errors 1 -- chaos -c $DIR/$filename -o $testname || exit 1
    fi
done

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"

    if [[ "$testname" == "exit_"* ]]; then
        continue
    fi

    SKIP_TESTS="syntax_error"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo -e "\n\n(compiler) Running memcheck: ${BUILD_DIR}/${testname}\n"

    if [[ "$OSTYPE" == "linux"* ]]; then
        valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos $BUILD_DIR/$testname || exit 1
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        /tmp/DrMemory/bin64/drmemory -no_follow_children -exit_code_if_errors 1 -- chaos $BUILD_DIR/$testname || exit 1
    fi
done

