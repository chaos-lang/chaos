#!/bin/sh

DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"

for filepath in $(find "$DIR" -maxdepth 1 -name '*.kaos'); do
    filename=$(basename "$filepath")
    testname="${filename%.*}"
    rm "$DIR/$testname.out"
    chaos "tests/$filename" > "$DIR/$testname.out" 2>&1

    echo "Updated: ${testname}"
done

"$DIR/shell/update.sh"
