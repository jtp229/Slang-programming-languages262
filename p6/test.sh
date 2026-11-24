#!/bin/bash

# You should not need to edit this file

langs=("c" "cc")
modes=("interpret" "valgrind" "memory")

# The most memory (in KB) that a memory test may use
memory_limit_kb=65536

# Print instructions on using this script
function usage() {
    echo "usage: ./test.sh <lang> <mode>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done
    echo
    echo -n " <mode>:"
    for m in ${modes[@]}; do echo -n " $m"; done
    echo
    echo " interpret: Runs <lang>'s slang on every XML AST in tests/interpret/inputs/,"
    echo "            and writes the output to results/interpret/<lang>/, one output"
    echo "            file per input file.  Run ./compare.sh <lang> interpret afterward"
    echo "            to check the results."
    echo " valgrind:  Runs the same tests under valgrind, and writes valgrind's report"
    echo "            for each test to results/valgrind/<lang>/.  Prints the name of"
    echo "            each test whose report is not clean."
    echo " memory:    Runs <lang>'s slang on every XML AST in tests/memory/inputs/,"
    echo "            writes the output to results/memory/<lang>/, and prints each"
    echo "            test's peak memory use.  Run ./compare.sh <lang> memory"
    echo "            afterward to check the results."
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

# Build the binary
make -C $1 || exit 1
cmd="$1/obj64/slang.exe -interpret"

# Clear the output folder
rm -rf results/$2/$1
mkdir -p results/$2/$1

# Run the tests
if [ "$2" == "interpret" ]; then
    for f in tests/interpret/inputs/*
    do
        name=`basename "$f"`
        echo "$name"
        $cmd "$f" > results/interpret/$1/"$name"
    done
fi

if [ "$2" == "valgrind" ]; then
    clean=0
    total=0
    for f in tests/interpret/inputs/*
    do
        name=`basename "$f"`
        log=results/valgrind/$1/"$name".log
        valgrind --leak-check=full --error-exitcode=99 $cmd "$f" > /dev/null 2> "$log"
        total=$((total+1))
        if grep -q "ERROR SUMMARY: 0 errors" "$log" && grep -q "All heap blocks were freed" "$log"; then
            clean=$((clean+1))
        else
            echo "NOT CLEAN: $name (see $log)"
        fi
    done
    echo "--- valgrind: $clean/$total clean ---"
fi

if [ "$2" == "memory" ]; then
    pass=0
    total=0
    for f in tests/memory/inputs/*
    do
        name=`basename "$f"`
        kb=$( { /usr/bin/time -f "%M" $cmd "$f" > results/memory/$1/"$name"; } 2>&1 | tail -1)
        # NB: In C++, programs that make cycles are not expected to stay under the
        #     limit
        if [ "$1" == "cc" ] && [ "$name" == "churn_cycles.xml" ]; then
            echo "$name: $kb KB (limit not checked for cc)"
            continue
        fi
        total=$((total+1))
        if [ "$kb" -le "$memory_limit_kb" ] 2>/dev/null; then
            pass=$((pass+1))
            echo "$name: $kb KB"
        else
            echo "$name: $kb KB (OVER LIMIT of $memory_limit_kb KB)"
        fi
    done
    echo "--- memory: $pass/$total within $memory_limit_kb KB ---"
fi
