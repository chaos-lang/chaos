#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    # rm "$DIR/$testname.out"
    # chaos "tests/$filename" > "$DIR/$testname.out" 2>&1

    if ! echo "syntax_error" | grep -w $testname > /dev/null; then
        chaos -a "tests/$filename" > "$DIR/$testname.json"
    fi

    echo "Updated: ${testname}"
done

$DIR/shell/update.sh
