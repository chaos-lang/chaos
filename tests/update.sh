#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    rm "$DIR/$testname.out"
    chaos "tests/$filename" > "$DIR/$testname.out"

    echo "Updated: ${testname}"
done

for dirpath in $(find $DIR -mindepth 1 -maxdepth 1 -type d); do
    $dirpath/update.sh
done
