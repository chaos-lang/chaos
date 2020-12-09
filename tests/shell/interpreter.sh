#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SUB="$( basename $DIR )"

if [[ "$OSTYPE" == "darwin"* ]]; then
    shopt -s expand_aliases
    alias sed='gsed'
fi

failed=false

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    echo "(interpreter) Running test: ${SUB}/${testname}"

    test=$(cat $DIR/$filename | chaos | sed "s|.\[1;41m\s*||g" | sed "s|.\[0;41m\s*||g" \
    | sed "s|.\[1;44m\s*||g" | sed "s|\s*.\[0m||g" | sed "s|.\[5;42m\s*||g" | sed "s|.\[0;90m.*||g" \
    | sed "s|.*\/chaos|Module: ~/chaos|g")
    if [ "$test" == "$out" ]
    then
        echo "OK"
    else
        echo "$test"
        echo "Fail"
        failed=true
    fi
done

if [ "$failed" = true ] ; then
    exit 1
fi
