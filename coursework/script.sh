#!/bin/bash
set -e
export LLVM_INSTALL_PATH=/modules/cs325/llvm-18.1.8
export PATH=$LLVM_INSTALL_PATH/bin:$PATH
export LD_LIBRARY_PATH=$LLVM_INSTALL_PATH/lib:$LD_LIBRARY_PATH
CLANG=$LLVM_INSTALL_PATH/bin/clang++

module load GCC/13.3.0

DIR="$(pwd)"

### Build mccomp compiler
echo "Cleanup *****"
rm -rf ./mccomp

echo "Compile *****"

make clean
make -j mccomp

COMP=$DIR/mccomp
echo $COMP
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
          echo "Running: ./mccomp $test"
          ./mccomp "$test"
        done
      elif [[ "$selected_test" == "quit" ]]; then
        echo "Exiting."
        break
      else
        echo "Running: ./mccomp $selected_test"
        ./mccomp "$selected_test"
      fi
    else
      echo "Invalid choice. Please try again."
    fi
  done
}

run_tests