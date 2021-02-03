#!/bin/bash

extra_flags=""
EXTRA_FLAGS_ENABLED=false
while getopts ":e:" opt; do
    case ${opt} in
    e )
        EXTRA_FLAGS_ENABLED=true;
        extra_flags=$OPTARG
        ;;
    \? )
        echo "Invalid Option: -$OPTARG" 1>&2
        exit 1
        ;;
    : )
        echo "Invalid Option: -$OPTARG requires an argument" 1>&2
        exit 1
        ;;
    esac
done

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

failed=false

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    SKIP_TESTS="syntax_error preemptive"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo "(compiler) Compiling test: ${testname}"

    if [ "$EXTRA_FLAGS_ENABLED" = true ] ; then
        cout=$(chaos -c tests/$filename -o $testname -e "$extra_flags" 2>&1)
    else
        cout=$(chaos -c tests/$filename -o $testname 2>&1)
    fi
    status=$?

    if [ $status -eq 0 ]
    then
        echo "OK"
    else
        echo "$cout"
        echo "Fail"
        failed=true
    fi
done

for filepath in $(find $DIR -maxdepth 1 -name '*.kaos'); do
    filename=$(basename $filepath)
    testname="${filename%.*}"
    out=$(<"$DIR/$testname.out")

    SKIP_TESTS="syntax_error preemptive"

    if echo $SKIP_TESTS | grep -w $testname > /dev/null; then
        continue
    fi

    echo "(compiler) Running test: ${testname}"

    test=$(build/$testname 2>&1)
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
