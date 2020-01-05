#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    echo "Running test: ${testname}"

    test=$(cat $DIR/$filename | chaos | tail -n +5)
    if [ "$test" == "$out" ]
    then
        echo "OK"
    else
        echo "Fail"
        exit 1
    fi
done
