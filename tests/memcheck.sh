#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"

    if [[ "$testname" == "exit_"* ]]; then
        continue
    fi

    echo -e "\n\n(interpreter) Running memcheck: ${testname}\n"

    if [[ "$OSTYPE" == "linux"* ]]; then
        valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos $DIR/$filename || exit 1
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        /tmp/DrMemory/bin64/drmemory -exit_code_if_errors 1 -- chaos $DIR/$filename || exit 1
    fi
done

for dirpath in $(find $DIR -mindepth 1 -maxdepth 1 -type d); do
    $dirpath/memcheck.sh
done
