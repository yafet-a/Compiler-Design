#!/bin/bash

counter=0

echo "Starting the script..."

for dir in tests/*; do
    echo "Checking directory: $dir"
    if [ -d "$dir" ]; then
        echo "Directory exists: $dir"
        for file in "$dir"/*.c; do
            echo "Checking file: $file"
            if [ -f "$file" ]; then
                echo "File exists: $file"
                ./bin/mccomp "$file"
                let counter++
            else
                echo "File does not exist: $file"
            fi
        done
    else
        echo "Not a directory: $dir"
    fi
done

echo "Total tests run: $counter"