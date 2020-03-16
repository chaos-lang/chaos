#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SUB="$( basename $DIR )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"

    echo -e "\n\nRunning memcheck: ${SUB}/${testname}\n"

    if [[ "$OSTYPE" == "linux"* ]]; then
        cat $DIR/$filename | valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos || exit 1
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        cat $DIR/$filename | /tmp/DrMemory/bin64/drmemory -exit_code_if_errors 1 -suppress suppress.mac.txt -- chaos || exit 1
    fi
done
