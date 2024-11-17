#!/bin/bash
set -e
export LLVM_INSTALL_PATH=/modules/cs325/llvm-18.1.8
export PATH=$LLVM_INSTALL_PATH/bin:$PATH
export LD_LIBRARY_PATH=$LLVM_INSTALL_PATH/lib:$LD_LIBRARY_PATH
CLANG=$LLVM_INSTALL_PATH/bin/clang++

module load GCC/13.3.0

DIR="$(pwd)"
TEST_DIR="$DIR/Alex-Tests/scope_errors"

### Build mccomp compiler
echo "Cleanup *****"
rm -rf ./mccomp

echo "Compile *****"

make clean || true  # Ignore error if 'clean' doesn't exist
make -j mccomp

COMP=$DIR/mccomp
echo "Compiler: $COMP"

function run_test {
  local test_dir=$1
  local test_name=$2
  local expected_outcome=$3

  cd $test_dir
  pwd
  rm -rf output.ll $test_name

  echo "Compiling $test_name.c with mccomp..."
  if "$COMP" ./$test_name.c; then
    echo "Compilation succeeded."
    echo "$expected_outcome"
  else
    echo "Compilation failed."
    echo "$expected_outcome"
  fi

  cd $DIR
}

function list_options {
  echo "Select a test to run:"
  echo "1) scope_error01"
  echo "2) scope_error02"
  echo "3) scope_error03"
  echo "4) scope_error04"
  echo "5) scope_error05"
  echo "6) scope_error06"
  echo "7) scope_error07"
  echo "8) Run all tests"
  echo "q) Quit"
}

function run_all_tests {
  run_test "$TEST_DIR/scope_error01" "scope_error01" "Should succeed"
  run_test "$TEST_DIR/scope_error02" "scope_error02" "Should fail as y is not defined"
  run_test "$TEST_DIR/scope_error03" "scope_error03" "Should fail as function is not defined"
  run_test "$TEST_DIR/scope_error04" "scope_error04" "Should fail as y is not defined"
  run_test "$TEST_DIR/scope_error05" "scope_error05" "Should fail as y is not defined in this scope"
  run_test "$TEST_DIR/scope_error06" "scope_error06" "Should fail as y is not defined"
  run_test "$TEST_DIR/scope_error07" "scope_error07" "Should succeed"
}

while true; do
  list_options
  read -p "Enter your choice: " choice

  case $choice in
    1) run_test "$TEST_DIR/scope_error01" "scope_error01" "Should succeed" ;;
    2) run_test "$TEST_DIR/scope_error02" "scope_error02" "Should fail as y is not defined" ;;
    3) run_test "$TEST_DIR/scope_error03" "scope_error03" "Should fail as function is not defined" ;;
    4) run_test "$TEST_DIR/scope_error04" "scope_error04" "Should fail as y is not defined" ;;
    5) run_test "$TEST_DIR/scope_error05" "scope_error05" "Should fail as y is not defined in this scope" ;;
    6) run_test "$TEST_DIR/scope_error06" "scope_error06" "Should fail as y is not defined" ;;
    7) run_test "$TEST_DIR/scope_error07" "scope_error07" "Should succeed" ;;
    8) run_all_tests ;;
    q) echo "Exiting."; exit 0 ;;
    *) echo "Invalid choice. Please try again." ;;
  esac
done