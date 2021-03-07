#!/bin/sh

DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find "$DIR" -maxdepth 1 -name '*.kaos'); do
    filename=$(basename "$filepath")
    testname="${filename%.*}"
    rm "$DIR/$testname.out"
    chaos "tests/rosetta/$filename" > "$DIR/$testname.out"

    echo "Updated: ${testname}"
done
