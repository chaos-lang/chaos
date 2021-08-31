#!/bin/bash

hyperfine --warmup 3 \
    'chaos tests/benchmark/fib.kaos' \
    'python3 tests/benchmark/fib.py' \
    'ruby tests/benchmark/fib.rb' \
    'php tests/benchmark/fib.php' \
    --export-json hyperfine.json

chaos_mean=$(jq '.[][0].mean' hyperfine.json)
python_mean=$(jq '.[][1].mean' hyperfine.json)
ruby_mean=$(jq '.[][2].mean' hyperfine.json)
php_mean=$(jq '.[][3].mean' hyperfine.json)

echo "Chaos mean: ${chaos_mean}"
echo "Python mean: ${python_mean}"
echo "Ruby mean: ${ruby_mean}"
echo "PHP mean: ${php_mean}"

if [ "$CI" == "true" ]
then
    python_crit=$(bc -l <<< "10.0 * $chaos_mean" )
    ruby_crit=$(bc -l <<< "3.5 * $chaos_mean" )
    php_crit=$(bc -l <<< "30.0 * $chaos_mean" )
else
    python_crit=$(bc -l <<< "13.5 * $chaos_mean" )
    ruby_crit=$(bc -l <<< "4.5 * $chaos_mean" )
    php_crit=$(bc -l <<< "3.4 * $chaos_mean" )
fi

echo "Python criteria: ${python_crit}"
echo "Ruby criteria: ${ruby_crit}"
echo "PHP criteria: ${php_crit}"

crits=($python_crit $ruby_crit $php_crit)
means=($python_mean $ruby_mean $php_mean)

for i in $(eval echo {0..2});
do
    if (( $(echo "${crits[${i}]} > ${means[${i}]}" | bc -l) )); then
        exit 1
    fi
done
