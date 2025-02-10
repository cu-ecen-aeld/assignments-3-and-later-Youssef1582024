#!/bin/bash


if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Error: Missing arguments. Please provide both file path and content."
    exit 1
fi


writefile=$1
writestr=$2


dirpath=$(dirname "$writefile")
if [ ! -d "$dirpath" ]; then
    echo "Creating directory path: $dirpath"
    mkdir -p "$dirpath"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create directory path $dirpath"
        exit 1
    fi
fi


echo "$writestr" > "$writefile"


if [ $? -ne 0 ]; then
    echo "Error: Failed to write to the file $writefile"
    exit 1
fi

echo "File created successfully at $writefile with content: $writestr"

