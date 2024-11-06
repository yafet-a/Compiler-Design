#!/bin/bash

# Function to list all test directories and files
list_tests() {
  echo "Available tests:"
  index=1
  tests=()

  for test_file in tests/*/*.c; do
    echo "$index) $test_file"
    tests+=("$test_file")
    ((index++))
  done

  echo "$index) Run all tests"
  tests+=("all")

  echo "$((index + 1))) Quit"
  tests+=("quit")
}

run_tests() {
  while true; do
    list_tests

    echo -n "Enter the number of the test you want to run (or 'q' to quit): "
    read -r choice

    if [[ "$choice" == "q" ]]; then
      echo "Exiting."
      break
    elif [[ "$choice" -gt 0 && "$choice" -le "${#tests[@]}" ]]; then
      selected_test="${tests[$((choice - 1))]}"
      
      if [[ "$selected_test" == "all" ]]; then
        echo "Running all tests..."
        for test in tests/*/*.c; do
          echo "Running: ./bin/mccomp $test"
          ./bin/mccomp "$test"
        done
      elif [[ "$selected_test" == "quit" ]]; then
        echo "Exiting."
        break
      else
        echo "Running: ./bin/mccomp $selected_test"
        ./bin/mccomp "$selected_test"
      fi
    else
      echo "Invalid choice. Please try again."
    fi
  done
}

run_tests