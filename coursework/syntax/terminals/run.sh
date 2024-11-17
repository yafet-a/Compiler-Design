#!/bin/bash

COMPILER=./mccomp

# Arrays of test files and expected outputs
declare -a test_files
declare -a expected_outputs

# Initialize test files and expected outputs
test_files[1]="./syntax/terminals/literal_1_f.c"
expected_outputs[1]="Expected some literal, got ? on line 6"

test_files[2]="./syntax/terminals/literal_2_f.c"
expected_outputs[2]="Expected some literal, got ? on line 6"

test_files[3]="./syntax/terminals/literal_3_f.c"
expected_outputs[3]="Expected some literal, got ? on line 6"

test_files[4]="./syntax/terminals/literal_4_f.c"
expected_outputs[4]="Expected some literal, got ? on line 6"

test_files[5]="./syntax/terminals/literal_5_f.c"
expected_outputs[5]="Expected float literal, got ? on line 5"

test_files[6]="./syntax/terminals/ident_1_f.c"
expected_outputs[6]="Expected identifier, got ) on line 1"

test_files[7]="./syntax/terminals/ident_2_f.c"
expected_outputs[7]="Expected ;, got { on line 1"

test_files[8]="./syntax/terminals/ident_3_f.c"
expected_outputs[8]="Expected identifier, got ) on line 1"

test_files[9]="./syntax/terminals/ident_4_f.c"
expected_outputs[9]="Expected type, got ) on line 1"

test_files[10]="./syntax/terminals/type_1_f.c"
expected_outputs[10]="Expected type, got ) on line 1"

test_files[11]="./syntax/terminals/empty_1_f.c"
expected_outputs[11]="Empty file"

test_files[12]="./syntax/terminals/late_decl_1_f.c"
expected_outputs[12]="Expected identifier or bracket, got int"

test_files[13]="./syntax/terminals/no_semicolon_1_f.c"
expected_outputs[13]="Expected semicolon"

test_files[14]="./syntax/terminals/no_semicolon_2_f.c"
expected_outputs[14]="Expected semicolon"

test_files[15]="./syntax/terminals/no_semicolon_3_f.c"
expected_outputs[15]="Expected semicolon"

test_files[16]="./syntax/terminals/no_semicolon_4_f.c"
expected_outputs[16]="Expected semicolon"

test_files[17]="./syntax/terminals/various_1_f.c"
expected_outputs[17]="Extra semicolon"

test_files[18]="./syntax/terminals/various_2_f.c"
expected_outputs[18]="Extra semicolon"

test_files[19]="./syntax/terminals/various_3_f.c"
expected_outputs[19]="Extra semicolon"

test_files[20]="./syntax/terminals/various_4_f.c"
expected_outputs[20]="Extra block"

function run_test {
    local index=$1
    local test_file=${test_files[$index]}
    local expected_output=${expected_outputs[$index]}
    
    echo
    echo "$test_file"
    $COMPILER "$test_file"
    echo "Expected output: $expected_output"
}

function list_tests {
    echo
    echo "TERMINAL SYNTACTICAL ERRORS"
    echo
    echo "Select a test to run:"
    echo "1) literal_1_f.c"
    echo "2) literal_2_f.c"
    echo "3) literal_3_f.c"
    echo "4) literal_4_f.c"
    echo "5) literal_5_f.c"
    echo "6) ident_1_f.c"
    echo "7) ident_2_f.c"
    echo "8) ident_3_f.c"
    echo "9) ident_4_f.c"
    echo "10) type_1_f.c"
    echo "11) empty_1_f.c"
    echo "12) late_decl_1_f.c"
    echo "13) no_semicolon_1_f.c"
    echo "14) no_semicolon_2_f.c"
    echo "15) no_semicolon_3_f.c"
    echo "16) no_semicolon_4_f.c"
    echo "17) various_1_f.c"
    echo "18) various_2_f.c"
    echo "19) various_3_f.c"
    echo "20) various_4_f.c"
    echo "a) Run all tests"
    echo "q) Quit"
}

while true; do
    list_tests
    read -p "Enter your choice: " choice
    case $choice in
        [1-9]|1[0-9]|20)
            run_test "$choice"
            ;;
        a)
            # Loop over indices 1 to 20
            for i in {1..20}; do
                run_test "$i"
            done
            ;;
        q) echo "Exiting."; exit 0;;
        *) echo "Invalid choice. Please try again.";;
    esac
done