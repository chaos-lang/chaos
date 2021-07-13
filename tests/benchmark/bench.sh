#!/bin/bash

hyperfine --warmup 3 \
    'chaos dev.kaos' \
    'python3 dev.py' \
    'ruby dev.rb' \
    'php dev.php' \
    --export-json hyperfine.json

chaos_mean=$(jq '.[][0].mean' hyperfine.json)
python_mean=$(jq '.[][1].mean' hyperfine.json)
ruby_mean=$(jq '.[][2].mean' hyperfine.json)
php_mean=$(jq '.[][3].mean' hyperfine.json)

echo $chaos_mean
echo $python_mean
echo $ruby_mean
echo $php_mean

python_crit=$(bc -l <<< "13.5 * $chaos_mean" )
ruby_crit=$(bc -l <<< "4.5 * $chaos_mean" )
php_crit=$(bc -l <<< "3.4 * $chaos_mean" )

echo $python_crit
echo $ruby_crit
echo $php_crit

crits=($python_crit $ruby_crit $php_crit)
means=($python_mean $ruby_mean $php_mean)

for i in $(eval echo {0..2});
do
    if (( $(echo "${crits[${i}]} > ${means[${i}]}" | bc -l) )); then
        exit 1
    fi
done
