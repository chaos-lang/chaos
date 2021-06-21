#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    json=$(<"$DIR/$testname.json")

    SKIP_TESTS="syntax_error"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo "Testing AST: ${testname}"

    test=$(chaos -a tests/$filename 2>&1)
    if [ "$test" == "$json" ]
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
