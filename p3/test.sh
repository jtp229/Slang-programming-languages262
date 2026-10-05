#!/bin/bash

langs=("rust" "python")

# Print instructions on using this script
function usage() {
    echo "usage: ./test.sh <lang>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done
    echo
    echo " Interprets the XML AST in every file in tests/inputs/ and writes the resulting"
    echo " output (or run-time error) to results/<lang>/, one output file per input file."
    echo " You should run ./compare.sh <lang> afterward to check results."
}

# Make sure the language option is legal
function check_lang() {
    for l in ${langs[@]}; do if [ "$1" == "$l" ]; then return; fi; done
    usage; exit 1;
}

# Validate
check_lang "$1"

# Clear the output folder
rm -rf results/$1
mkdir -p results/$1

# Build the binary and get the execution command
cmd=""
if [ "$1" == "rust" ]; then
    (cd rust && cargo build) || exit 1
    cmd="rust/target/debug/slang"
fi
if [ "$1" == "python" ]; then
    cmd="python3 python/slang.py"
fi

# Run the tests
for f in tests/inputs/*
do
    name=`basename "$f"`
    echo "$name"
    $cmd -interpret "$f" > results/$1/"$name"
done
