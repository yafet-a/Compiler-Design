#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

extern TOKEN CurTok;

TOKEN getNextToken();
void putBackToken(TOKEN tok);

bool parseProgram();
bool parseExternList();
bool parseExternListPrime();
bool parseExtern();
bool parseDeclList();
bool parseDeclListPrime();
bool parseDecl();
bool parseVarDecl();
bool parseTypeSpec();
bool parseVarType();
bool parseFunDecl();
bool parseParams();
bool parseParamList();
bool parseParam();
bool parseBlock();
bool parseLocalDecls();
bool parseLocalDecl();
bool parseStmtList();
bool parseStmt();
bool parseExprStmt();
bool parseWhileStmt();
bool parseIfStmt();
bool parseElseStmt();
bool parseReturnStmt();
bool parseExpr();
bool parseAssignExpr();
bool parseLogicOr();
bool parseLogicOrPrime();
bool parseLogicAnd();
bool parseLogicAndPrime();
bool parseEquality();
bool parseEqualityPrime();
bool parseRelation();
bool parseRelationPrime();
bool parseAdditive();
bool parseAdditivePrime();
bool parseMultiply();
bool parseMultiplyPrime();
bool parseUnary();
bool parsePrimary();
bool parseArgs();
bool parseArgList();
bool parseArgListPrime();

void parser();

#endif