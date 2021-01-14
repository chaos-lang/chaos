#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SUB="$( basename $DIR )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    rm "$DIR/$testname.out"
    cat "$DIR/$filename" | chaos > "$DIR/$testname.out" 2>&1
    sed -i "s|.\[1;41m\s*||g" "$DIR/$testname.out"
    sed -i "s|.\[0;41m\s*||g" "$DIR/$testname.out"
    sed -i "s|.\[1;44m\s*||g" "$DIR/$testname.out"
    sed -i "s|\s*.\[0m||g" "$DIR/$testname.out"
    sed -i "s|.\[5;42m\s*||g" "$DIR/$testname.out"
    sed -i "s|.\[0;90m.*||g" "$DIR/$testname.out"
    sed -i "s|.*\/chaos|File: \"~/chaos|g" "$DIR/$testname.out"

    echo "Updated: ${SUB}/${testname}"
done
