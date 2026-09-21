#!/bin/bash

langs=("java" "go")

# Print instructions on using this script
function usage() {
    echo "usage: ./test.sh <lang>"
    echo -n " <lang>:"
    for l in ${langs[@]}; do echo -n " $l"; done
    echo
    echo " Parses the XML tokens in every file in tests/inputs/ and writes the resulting XML AST"
    echo " (or parse error) to results/<lang>/, one output file per input file."
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
if [ "$1" == "java" ]; then
    (cd java && ./gradlew build) || exit 1
    cmd="java -jar java/app/build/libs/app.jar"
fi
if [ "$1" == "go" ]; then
    (cd go && go build) || exit 1
    cmd="go/slang"
fi

# Run the tests
for f in tests/inputs/*
do
    name=`basename "$f"`
    echo "$name"
    $cmd -parse "$f" > results/$1/"$name"
done
