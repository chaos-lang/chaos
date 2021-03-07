#!/bin/sh

OSTYPE=$(uname --kernel-name)
DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"
SUB="$( basename "$DIR" )"

for filepath in $(find "$DIR" -maxdepth 1 -name '*.kaos' ! -name '*_fail.kaos'); do
    filename=$(basename "$filepath")
    testname="${filename%.*}"

    printf "\n\nRunning memcheck: %s/%s\n" "$SUB" "$testname"

    if [ "$OSTYPE" = "Linux*" ]; then
        cat "$DIR/$filename" | valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes --error-exitcode=1 chaos || exit 1
    elif [ "$OSTYPE" = "Darwin*" ]; then
        cat "$DIR/$filename" | /tmp/DrMemory/bin64/drmemory -exit_code_if_errors 1 -suppress suppress.mac.txt -- chaos || exit 1
    fi
done
