#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include <cstdio>
#include <string>

extern FILE *pFile;
extern int lineNo, columnNo;
extern std::string IdentifierStr;
extern int IntVal;
extern bool BoolVal;
extern float FloatVal;
extern std::string StringVal;

// Lexer function
TOKEN gettok();

#endif // LEXER_H