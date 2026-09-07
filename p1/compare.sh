#!/bin/bash

langs=("java" "python")

# Print instructions on using this script
function usage() {
    echo "usage: ./compare.sh <lang>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done;
    echo
    echo " Diff results/<lang>/* (from ./test.sh <lang>) against the expected"
    echo " outputs in tests/outputs/*."
}

# Make sure the language option is legal
function check_lang() {
    for l in ${langs[@]}; do if [ "$1" == "$l" ]; then return; fi; done
    usage; exit 1;
}

# Validate
check_lang "$1"

if [ ! -d "results/$1" ]; then
    echo "No results found in results/$1.  Be sure to run ./test.sh $1 first."
    exit 1
fi

# Compare each generated result against the golden output
pass=0
fail=0
for f in tests/outputs/*
do
    name=`basename "$f"`
    if diff -q "results/$1/$name" "$f" > /dev/null 2>&1; then
        pass=$((pass+1))
    else
        echo "FAIL: $name"
        diff "results/$1/$name" "$f"
        fail=$((fail+1))
    fi
done

echo "--- $pass/$((pass+fail)) tests passed ---"
[ "$fail" -eq 0 ]
