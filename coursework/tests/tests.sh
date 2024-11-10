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

function validate {
  $1 > perf_out
  echo
  echo $1
  grep "Result" perf_out; grep "PASSED" perf_out
  rc=$?; if [[ $rc != 0 ]]; then echo "TEST FAILED *****"; exit $rc; fi; rm perf_out
}

function run_test {
  local test_dir=$1
  local test_name=$2
  local test_exec=$3

  cd $test_dir
  pwd
  rm -rf output.ll $test_exec
  "$COMP" ./$test_name.c
  $CLANG driver.cpp output.ll -o $test_exec
  validate "./$test_exec"
  cd $DIR
}

function list_options {
  echo "Select a test to run:"
  echo "1) addition"
  echo "2) factorial"
  echo "3) fibonacci"
  echo "4) pi"
  echo "5) while"
  echo "6) void"
  echo "7) cosine"
  echo "8) unary"
  echo "9) recurse"
  echo "10) rfact"
  echo "11) palindrome"
  echo "12) Run all tests"
  echo "q) Quit"
}

function run_all_tests {
  run_test "tests/addition" "addition" "add"
  run_test "tests/factorial" "factorial" "fact"
  run_test "tests/fibonacci" "fibonacci" "fib"
  run_test "tests/pi" "pi" "pi"
  run_test "tests/while" "while" "while"
  run_test "tests/void" "void" "void"
  run_test "tests/cosine" "cosine" "cosine"
  run_test "tests/unary" "unary" "unary"
  run_test "tests/recurse" "recurse" "recurse"
  run_test "tests/rfact" "rfact" "rfact"
  run_test "tests/palindrome" "palindrome" "palindrome"
}

while true; do
  list_options
  read -p "Enter your choice: " choice

  case $choice in
    1) run_test "tests/addition" "addition" "add" ;;
    2) run_test "tests/factorial" "factorial" "fact" ;;
    3) run_test "tests/fibonacci" "fibonacci" "fib" ;;
    4) run_test "tests/pi" "pi" "pi" ;;
    5) run_test "tests/while" "while" "while" ;;
    6) run_test "tests/void" "void" "void" ;;
    7) run_test "tests/cosine" "cosine" "cosine" ;;
    8) run_test "tests/unary" "unary" "unary" ;;
    9) run_test "tests/recurse" "recurse" "recurse" ;;
    10) run_test "tests/rfact" "rfact" "rfact" ;;
    11) run_test "tests/palindrome" "palindrome" "palindrome" ;;
    12) run_all_tests ;;
    q) echo "Exiting."; exit 0 ;;
    *) echo "Invalid choice. Please try again." ;;
  esac
done