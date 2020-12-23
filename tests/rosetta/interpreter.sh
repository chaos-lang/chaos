#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

failed=false

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    echo "(interpreter) Running test: ${testname}"

    test=$(chaos tests/rosetta/$filename)
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
