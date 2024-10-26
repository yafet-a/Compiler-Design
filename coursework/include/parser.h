#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "ast.h"
#include <memory>

extern TOKEN CurTok;

TOKEN getNextToken();
void putBackToken(TOKEN tok);

std::unique_ptr<ASTnode> parseProgram();
std::unique_ptr<ASTnode> parseExternList();
std::unique_ptr<ASTnode> parseExternListPrime();
std::unique_ptr<ASTnode> parseExtern();
std::unique_ptr<ASTnode> parseDeclList();
std::unique_ptr<ASTnode> parseDeclListPrime();
std::unique_ptr<ASTnode> parseDecl();
std::unique_ptr<ASTnode> parseVarDecl();
std::unique_ptr<ASTnode> parseTypeSpec();
std::unique_ptr<ASTnode> parseVarType();
std::unique_ptr<ASTnode> parseFunDecl();
std::unique_ptr<ASTnode> parseParams();
std::vector<std::unique_ptr<VarDeclASTnode>> parseParamList();std::unique_ptr<ASTnode> parseParam();
std::unique_ptr<ASTnode> parseBlock();
std::unique_ptr<ASTnode> parseLocalDecls();
std::unique_ptr<ASTnode> parseLocalDecl();
std::unique_ptr<ASTnode> parseStmtList();
std::unique_ptr<ASTnode> parseStmt();
std::unique_ptr<ASTnode> parseExprStmt();
std::unique_ptr<ASTnode> parseWhileStmt();
std::unique_ptr<ASTnode> parseIfStmt();
std::unique_ptr<ASTnode> parseElseStmt();
std::unique_ptr<ASTnode> parseReturnStmt();
std::unique_ptr<ASTnode> parseExpr();
std::unique_ptr<ASTnode> parseAssignExpr();
std::unique_ptr<ASTnode> parseLogicOr();
std::unique_ptr<ASTnode> parseLogicOrPrime();
std::unique_ptr<ASTnode> parseLogicAnd();
std::unique_ptr<ASTnode> parseLogicAndPrime();
std::unique_ptr<ASTnode> parseEquality();
std::unique_ptr<ASTnode> parseEqualityPrime();
std::unique_ptr<ASTnode> parseRelation();
std::unique_ptr<ASTnode> parseRelationPrime();
std::unique_ptr<ASTnode> parseAdditive();
std::unique_ptr<ASTnode> parseAdditivePrime();
std::unique_ptr<ASTnode> parseMultiply();
std::unique_ptr<ASTnode> parseMultiplyPrime();
std::unique_ptr<ASTnode> parseUnary();
std::unique_ptr<ASTnode> parsePrimary();
std::unique_ptr<ASTnode> parseArgs();
std::unique_ptr<ASTnode> parseArgList();
std::unique_ptr<ASTnode> parseArgListPrime();

void parser();

#endif