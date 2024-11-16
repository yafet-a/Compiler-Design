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
    return_tok.filename = currentFilename;  // Add this
    return return_tok;
}

std::string tokenTypeToString(int type) {
  switch (type) {
  case IDENT:
    return "IDENT";
  case ASSIGN:
    return "ASSIGN";
  case LBRA:
    return "LBRA";
  case RBRA:
    return "RBRA";
  case LPAR:
    return "LPAR";
  case RPAR:
    return "RPAR";
  case SC:
    return "SC";
  case COMMA:
    return "COMMA";
  case INT_TOK:
    return "INT_TOK";
  case VOID_TOK:
    return "VOID_TOK";
  case FLOAT_TOK:
    return "FLOAT_TOK";
  case BOOL_TOK:
    return "BOOL_TOK";
  case EXTERN:
    return "EXTERN";
  case IF:
    return "IF";
  case ELSE:
    return "ELSE";
  case WHILE:
    return "WHILE";
  case RETURN:
    return "RETURN";
  case INT_LIT:
    return "INT_LIT";
  case FLOAT_LIT:
    return "FLOAT_LIT";
  case BOOL_LIT:
    return "BOOL_LIT";
  case AND:
    return "AND";
  case OR:
    return "OR";
  case PLUS:
    return "PLUS";
  case MINUS:
    return "MINUS";
  case ASTERIX:
    return "ASTERIX";
  case DIV:
    return "DIV";
  case MOD:
    return "MOD";
  case NOT:
    return "NOT";
  case EQ:
    return "EQ";
  case NE:
    return "NE";
  case LE:
    return "LE";
  case LT:
    return "LT";
  case GE:
    return "GE";
  case GT:
    return "GT";
  case EOF_TOK:
    return "EOF_TOK";
  case INVALID:
    return "INVALID";
  default:
    return "UNKNOWN";
  }
}