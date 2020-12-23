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

    echo "(compiler) Compiling test: ${testname}"

    if [ "$EXTRA_FLAGS_ENABLED" = true ] ; then
        cout=$(chaos -c tests/rosetta/$filename -o $testname -e "$extra_flags")
    else
        cout=$(chaos -c tests/rosetta/$filename -o $testname)
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

    echo "(compiler) Running test: ${testname}"

    test=$(build/$testname)
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
