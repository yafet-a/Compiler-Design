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
  rc=$?; if [[ $rc != 0 ]]; then echo "TEST FAILED *****"; return $rc; fi; rm perf_out
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
  echo "1) example_scope"
  echo "2) long"
  echo "3) widening"
  echo "4) truthiness"
  echo "5) mutual"
  echo "6) Run all tests"
  echo "7) Run failed tests"
  echo "q) Quit"
}

function list_failed_tests {
  echo "Select a failed test to run:"
  echo "1) call_undefined"
  echo "2) double_func"
  echo "3) double_func_mismatched"
  echo "4) double_global"
  echo "5) double_local"
  echo "6) extern_fun_decl_mismatch"
  echo "7) no_init_assign"
  echo "8) scope_bleed"
  echo "9) undefined_var"
  echo "10) unexpected_ret"
  echo "11) wrong_num_args"
  echo "12) wrong_type_args"
  echo "13) wrong_type_ret"
  echo "a) Run all failed tests"
  echo "b) Back"
}

function run_all_tests {
  run_test "minic-medium-tests/example_scope" "example_scope" "example_scope"
  run_test "minic-medium-tests/long" "long" "long"
  run_test "minic-medium-tests/widening" "widening" "widening"
  run_test "minic-medium-tests/truthiness" "truthiness" "truthiness"
  run_test "minic-medium-tests/mutual" "mutual" "mutual"
}

function run_all_failed_tests {
  run_test "minic-medium-tests/fail" "call_undefined" "call_undefined"
  run_test "minic-medium-tests/fail" "double_func" "double_func"
  run_test "minic-medium-tests/fail" "double_func_mismatched" "double_func_mismatched"
  run_test "minic-medium-tests/fail" "double_global" "double_global"
  run_test "minic-medium-tests/fail" "double_local" "double_local"
  run_test "minic-medium-tests/fail" "extern_fun_decl_mismatch" "extern_fun_decl_mismatch"
  run_test "minic-medium-tests/fail" "no_init_assign" "no_init_assign"
  run_test "minic-medium-tests/fail" "scope_bleed" "scope_bleed"
  run_test "minic-medium-tests/fail" "undefined_var" "undefined_var"
  run_test "minic-medium-tests/fail" "unexpected_ret" "unexpected_ret"
  run_test "minic-medium-tests/fail" "wrong_num_args" "wrong_num_args"
  run_test "minic-medium-tests/fail" "wrong_type_args" "wrong_type_args"
  run_test "minic-medium-tests/fail" "wrong_type_ret" "wrong_type_ret"
}

while true; do
  list_options
  read -p "Enter your choice: " choice

  case $choice in
    1) run_test "minic-medium-tests/example_scope" "example_scope" "example_scope" || true ;;
    2) run_test "minic-medium-tests/long" "long" "long" || true ;;
    3) run_test "minic-medium-tests/widening" "widening" "widening" || true ;;
    4) run_test "minic-medium-tests/truthiness" "truthiness" "truthiness" || true ;;
    5) run_test "minic-medium-tests/mutual" "mutual" "mutual" || true ;;
    6) run_all_tests || true ;;
    7)
      while true; do
        list_failed_tests
        read -p "Enter your choice: " fail_choice

        case $fail_choice in
          1) run_test "minic-medium-tests/fail" "call_undefined" "call_undefined" || true ;;
          2) run_test "minic-medium-tests/fail" "double_func" "double_func" || true ;;
          3) run_test "minic-medium-tests/fail" "double_func_mismatched" "double_func_mismatched" || true ;;
          4) run_test "minic-medium-tests/fail" "double_global" "double_global" || true ;;
          5) run_test "minic-medium-tests/fail" "double_local" "double_local" || true ;;
          6) run_test "minic-medium-tests/fail" "extern_fun_decl_mismatch" "extern_fun_decl_mismatch" || true ;;
          7) run_test "minic-medium-tests/fail" "no_init_assign" "no_init_assign" || true ;;
          8) run_test "minic-medium-tests/fail" "scope_bleed" "scope_bleed" || true ;;
          9) run_test "minic-medium-tests/fail" "undefined_var" "undefined_var" || true ;;
          10) run_test "minic-medium-tests/fail" "unexpected_ret" "unexpected_ret" || true ;;
          11) run_test "minic-medium-tests/fail" "wrong_num_args" "wrong_num_args" || true ;;
          12) run_test "minic-medium-tests/fail" "wrong_type_args" "wrong_type_args" || true ;;
          13) run_test "minic-medium-tests/fail" "wrong_type_ret" "wrong_type_ret" || true ;;
          a) run_all_failed_tests || true; break ;;
          b) break ;;
          *) echo "Invalid choice. Please try again." ;;
        esac
      done
      ;;
    q) echo "Exiting."; exit 0 ;;
    *) echo "Invalid choice. Please try again." ;;
  esac
done