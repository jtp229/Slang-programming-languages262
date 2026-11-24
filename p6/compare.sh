#!/bin/bash

# You should not need to edit this file

langs=("c" "cc")
modes=("interpret" "memory")

# Print instructions on using this script
function usage() {
    echo "usage: ./compare.sh <lang> <mode>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done
    echo
    echo -n " <mode>:"
    for m in ${modes[@]}; do echo -n " $m"; done
    echo
    echo " Diff results/<mode>/<lang>/* (from ./test.sh <lang> <mode>) against the"
    echo " expected outputs in tests/<mode>/outputs/*."
}

# Make sure the language and mode options are legal
function check_lang() {
    for l in ${langs[@]}; do if [ "$1" == "$l" ]; then return; fi; done
    usage; exit 1;
}
function check_mode() {
    for m in ${modes[@]}; do if [ "$1" == "$m" ]; then return; fi; done
    usage; exit 1;
}

# Validate
check_lang "$1"
check_mode "$2"

if [ ! -d "results/$2/$1" ]; then
    echo "No results found in results/$2/$1.  Be sure to run ./test.sh $1 $2 first."
    exit 1
fi

# Compare each generated result against the golden output
pass=0
fail=0
for f in tests/$2/outputs/*
do
    name=`basename "$f"`
    if diff -q "results/$2/$1/$name" "$f" > /dev/null 2>&1; then
        pass=$((pass+1))
    else
        echo "FAIL: $name"
        diff "results/$2/$1/$name" "$f"
        fail=$((fail+1))
    fi
done

echo "--- $pass/$((pass+fail)) tests passed ---"
[ "$fail" -eq 0 ]
