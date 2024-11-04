#!/bin/bash

# Function to list all test directories and files
list_tests() {
  echo "Available tests:"
  index=1
  tests=()

  for test_file in minic-medium-tests/*/*.c; do
    echo "$index) $test_file"
    tests+=("$test_file")
    ((index++))
  done

  echo "$index) Run all tests"
  tests+=("all")

  echo "$((index + 1))) View fail tests"
  tests+=("view_fail")

  echo "$((index + 2))) Quit"
  tests+=("quit")
}

list_fail_tests() {
  echo "Available fail tests:"
  index=1
  fail_tests=()

  for test_file in minic-medium-tests/fail/*.c; do
    echo "$index) $test_file"
    fail_tests+=("$test_file")
    ((index++))
  done

  echo "$index) Run all fail tests"
  fail_tests+=("all_fail")

  echo "$((index + 1))) Back to main menu"
  fail_tests+=("back")
}

run_tests() {
  while true; do
    list_tests

    echo -n "Enter the number of the test you want to run: "
    read -r choice

    if [[ "$choice" -gt 0 && "$choice" -le "${#tests[@]}" ]]; then
      selected_test="${tests[$((choice - 1))]}"
      
      if [[ "$selected_test" == "all" ]]; then
        echo "Running all tests..."
        for test in minic-medium-tests/*/*.c; do
          echo "Running: ./bin/mccomp $test"
          ./bin/mccomp "$test"
        done
      elif [[ "$selected_test" == "view_fail" ]]; then
        while true; do
          list_fail_tests

          echo -n "Enter the number of the fail test you want to run: "
          read -r fail_choice

          if [[ "$fail_choice" -gt 0 && "$fail_choice" -le "${#fail_tests[@]}" ]]; then
            selected_fail_test="${fail_tests[$((fail_choice - 1))]}"
            
            if [[ "$selected_fail_test" == "all_fail" ]]; then
              echo "Running all fail tests..."
              for test in minic-medium-tests/fail/*.c; do
                echo "Running: ./bin/mccomp $test"
                ./bin/mccomp "$test"
              done
              break
            elif [[ "$selected_fail_test" == "back" ]]; then
              break
            else
              echo "Running: ./bin/mccomp $selected_fail_test"
              ./bin/mccomp "$selected_fail_test"
            fi
          else
            echo "Invalid choice. Please try again."
          fi
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