#!/bin/sh

OSTYPE=$(uname --kernel-name)
DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find "$DIR" -maxdepth 1 -name '*.kaos'); do
    filename=$(basename "$filepath")
    testname="${filename%.*}"

    if [ "$testname" = "exit_*" ]; then
        continue
    fi

    SKIP_TESTS="syntax_error preemptive"

    if echo "$SKIP_TESTS" | grep -w "$testname" > /dev/null; then
        continue
    fi

    printf "\n\n(interpreter) Running memcheck: %s\n" "$testname"

    if [ "$OSTYPE" = "Linux" ]; then
        valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos "$DIR/$filename" || exit 1
    elif [ "$OSTYPE" = "Darwin" ]; then
        /tmp/DrMemory/bin64/drmemory -exit_code_if_errors 1 -suppress tests/shell/suppress.mac.txt -- chaos "$DIR/$filename" || exit 1
    fi
done

for dirpath in $(find "$DIR" -mindepth 1 -maxdepth 1 -type d); do
    if [ -f "$dirpath/memcheck.sh" ]; then
        "$dirpath/memcheck.sh"
    fi
done
