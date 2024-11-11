echo
echo "TERMINAL SYNTACTICAL ERRORS"

echo
echo ./tests/syntax/terminals/literal_1_f.c
./mccomp ./tests/syntax/terminals/literal_1_f.c
echo "Expected some literal, got ? on line 6"

echo
echo ./tests/syntax/terminals/literal_2_f.c
./mccomp ./tests/syntax/terminals/literal_2_f.c
echo "Expected some literal, got ? on line 6"

echo
echo ./tests/syntax/terminals/literal_3_f.c
./mccomp ./tests/syntax/terminals/literal_3_f.c
echo "Expected some literal, got ? on line 6"

echo
echo ./tests/syntax/terminals/literal_4_f.c
./mccomp ./tests/syntax/terminals/literal_4_f.c
echo "Expected some literal, got ? on line 6"

echo
echo ./tests/syntax/terminals/literal_5_f.c
./mccomp ./tests/syntax/terminals/literal_5_f.c
echo "Expected float litearal, got ? on line 5"

echo
echo ./tests/syntax/terminals/ident_1_f.c
./mccomp ./tests/syntax/terminals/ident_1_f.c
echo "Expected identifier, got ) line 1"

echo
echo ./tests/syntax/terminals/ident_2_f.c
./mccomp ./tests/syntax/terminals/ident_2_f.c
echo "Expected ;, got { line 1"

echo
echo ./tests/syntax/terminals/ident_3_f.c
./mccomp ./tests/syntax/terminals/ident_3_f.c
echo "Expected identifier, got ) line 1"


echo
echo ./tests/syntax/terminals/ident_4_f.c
./mccomp ./tests/syntax/terminals/ident_4_f.c
echo "Expected type, got ) line 1"


echo
echo ./tests/syntax/terminals/type_1_f.c
./mccomp ./tests/syntax/terminals/type_1_f.c
echo "Expected type, got ) line 1"


echo
echo ./tests/syntax/terminals/empty_1_f.c
./mccomp ./tests/syntax/terminals/empty_1_f.c
echo "Empty file"

echo
echo ./tests/syntax/terminals/late_decl_1_f.c
./mccomp ./tests/syntax/terminals/late_decl_1_f.c
echo "expected identifier or bracket got int"

echo
echo ./tests/syntax/terminals/no_semicolon_1_f.c
./mccomp ./tests/syntax/terminals/no_semicolon_1_f.c
echo "expected semicolon"

echo
echo ./tests/syntax/terminals/no_semicolon_2_f.c
./mccomp ./tests/syntax/terminals/no_semicolon_2_f.c
echo "expected semicolon"

echo
echo ./tests/syntax/terminals/no_semicolon_3_f.c
./mccomp ./tests/syntax/terminals/no_semicolon_3_f.c
echo "expected semicolon"

echo
echo ./tests/syntax/terminals/no_semicolon_4_f.c
./mccomp ./tests/syntax/terminals/no_semicolon_4_f.c
echo "expected semicolon"

echo
echo ./tests/syntax/terminals/various_1_f.c
./mccomp ./tests/syntax/terminals/various_1_f.c
echo "extra semicolon"


echo
echo ./tests/syntax/terminals/various_2_f.c
./mccomp ./tests/syntax/terminals/various_2_f.c
echo "extra semicolon"


echo
echo ./tests/syntax/terminals/various_3_f.c
./mccomp ./tests/syntax/terminals/various_3_f.c
echo "extra semicolon"


echo
echo ./tests/syntax/terminals/various_4_f.c
./mccomp ./tests/syntax/terminals/various_4_f.c
echo "extra block"
