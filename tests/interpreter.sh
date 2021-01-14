#!/bin/bash

OPTS=`getopt \
-o ns --long no-shell \
-- "$@"`
if [ $? != 0 ] ; then echo "Failed parsing options." >&2 ; exit 1 ; fi
eval set -- "$OPTS"

SHELL=true
while true; do
    case "$1" in
        -ns | --no-shell )
            SHELL=false;
            shift
        ;;
        -- )
            shift;
            break
        ;;
        * )
            break
        ;;
    esac
done


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

failed=false

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    echo "(interpreter) Running test: ${testname}"

    test=$(chaos tests/$filename 2>&1)
    if [ "$test" == "$out" ]
    then
        echo "OK"
    else
        echo "$test"
        echo "Fail"
        failed=true
    fi
done

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    SKIP_TESTS="nonewline function decision everything"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo "(interpreter) Interactive test: ${testname}"

    cd tests/

    test=$(cat $filename | chaos 2>&1 | sed "s|.\[1;41m\s*||g" | sed "s|.\[0;41m\s*||g" \
    | sed "s|.\[1;44m\s*||g" | sed "s|\s*.\[0m||g" | sed "s|.\[5;42m\s*||g" | sed "s|.\[0;90m.*||g" \
    | sed "s|.*\/chaos|File: \"~/chaos|g")

    if [ "$test" == "$out" ]
    then
        echo "OK"
    else
        echo "$test"
        echo "Fail"
        failed=true
    fi

    cd ..
done

if [ "$SHELL" = true ] ; then
    $DIR/shell/interpreter.sh
fi

if [ "$failed" = true ] ; then
    exit 1
fi
