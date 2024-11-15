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
  echo "1) implicit"
  echo "2) scope"
  echo "3) associativity"
  echo "4) global"
  echo "5) returns"
  echo "6) unary2"
  echo "7) while"
  echo "8) assign"
  echo "9) lazyeval"
  echo "10) infinite"
  echo "11) Run all tests"
  echo "q) Quit"
}

function run_all_tests {
  run_test "cult-tests/implicit" "implicit" "imp"
  run_test "cult-tests/scope" "scope" "scope"
  run_test "cult-tests/associativity" "associativity" "assoc"
  run_test "cult-tests/global" "global" "global"
  run_test "cult-tests/returns" "returns" "returns"
  run_test "cult-tests/unary2" "unary2" "unary2"
  run_test "cult-tests/while" "while" "while"
  run_test "cult-tests/assign" "assign" "assign"
  run_test "cult-tests/lazyeval" "lazyeval" "lazyeval"
  run_test "cult-tests/infinite" "infinite" "infinite"
}

while true; do
  list_options
  read -p "Enter your choice: " choice

  case $choice in
    1) run_test "cult-tests/implicit" "implicit" "imp" ;;
    2) run_test "cult-tests/scope" "scope" "scope" ;;
    3) run_test "cult-tests/associativity" "associativity" "assoc" ;;
    4) run_test "cult-tests/global" "global" "global" ;;
    5) run_test "cult-tests/returns" "returns" "returns" ;;
    6) run_test "cult-tests/unary2" "unary2" "unary2" ;;
    7) run_test "cult-tests/while" "while" "while" ;;
    8) run_test "cult-tests/assign" "assign" "assign" ;;
    9) run_test "cult-tests/lazyeval" "lazyeval" "lazyeval" ;;
    10) run_test "cult-tests/infinite" "infinite" "infinite" ;;
    11) run_all_tests ;;
    q) echo "Exiting."; exit 0 ;;
    *) echo "Invalid choice. Please try again." ;;
  esac
done