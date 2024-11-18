#include "tokens.h"
#include "lexer.h"
#include <iostream>

extern int lineNo, columnNo;

TOKEN returnTok(std::string lexVal, int tok_type) {
    TOKEN return_tok;
    return_tok.lexeme = lexVal;
    return_tok.type = tok_type;
    return_tok.lineNo = lineNo;
    return_tok.columnNo = columnNo - lexVal.length() - 1;
    return_tok.lineContent = currentLineContent;
    return_tok.filename = currentFilename;
    return return_tok;
}