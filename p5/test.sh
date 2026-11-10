#!/bin/bash

langs=("go" "scheme")
modes=("typecheck" "optimize")

# Print instructions on using this script
function usage() {
    echo "usage: ./test.sh <lang> <mode>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done
    echo
    echo -n " <mode>:"
    for m in ${modes[@]}; do echo -n " $m"; done
    echo
    echo " Runs <lang>'s slang in -<mode> on every XML AST in tests/<mode>/inputs/, and"
    echo " writes the resulting output (type warnings, or an optimized XML AST) to"
    echo " results/<mode>/<lang>/, one output file per input file."
    echo " You should run ./compare.sh <lang> <mode> afterward to check results."
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

# Clear the output folder
rm -rf results/$2/$1
mkdir -p results/$2/$1

# Build the binary and get the execution command
cmd=""
if [ "$1" == "go" ]; then
    (cd go && go build) || exit 1
    cmd="go/slang"
fi
if [ "$1" == "scheme" ]; then
    cmd="gsi scheme/slang.scm"
fi

# Run the tests
for f in tests/$2/inputs/*
do
    name=`basename "$f"`
    echo "$name"
    $cmd -$2 "$f" > results/$2/$1/"$name"
done
