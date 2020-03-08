#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    echo "Running test: ${testname}"

    test=$(chaos tests/$filename)
    if [ "$test" == "$out" ]
    then
        echo "OK"
    else
        echo "$test"
        echo "Fail"
        exit 1
    fi
done

for dirpath in $(find $DIR -mindepth 1 -maxdepth 1 -type d); do
    $dirpath/run.sh
done
