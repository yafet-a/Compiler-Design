#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "ast.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>
#include <unordered_set>

// Token management
extern TOKEN CurTok;
TOKEN getNextToken();
void putBackToken(TOKEN tok);

// Main parsing functions
std::unique_ptr<ProgramNode> parseProgram();
std::unique_ptr<ExternNode> parseExtern();
std::unique_ptr<ASTnode> parseDecl();
std::unique_ptr<BlockNode> parseBlock();
std::unique_ptr<ASTnode> parseStmt();
std::unique_ptr<IfNode> parseIfStmt();
std::unique_ptr<ReturnNode> parseReturnStmt();
std::unique_ptr<ASTnode> parseUnary();
std::unique_ptr<WhileNode> parseWhile();
std::unique_ptr<ASTnode> parseExpr();
std::unique_ptr<ASTnode> parseAdditive();
std::unique_ptr<ASTnode> parseMultiply();
std::unique_ptr<ASTnode> parseEquality();
std::unique_ptr<ASTnode> parseRelational();
std::unique_ptr<ASTnode> parsePrimary();
std::unique_ptr<ASTnode> parseExprStmt();
std::unique_ptr<ASTnode> parseLocalDecl();
std::unique_ptr<ExternListNode> parseExternList();
std::unique_ptr<DeclListNode> parseDeclList();
std::unique_ptr<DeclListNode> parseDeclListPrime();

// Helper functions
std::string parseTypeSpec();
std::optional<std::vector<std::pair<std::string, std::string>>> parseParams();
std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgs();

// First sets
extern std::unordered_set<int> FIRST_program;
extern std::unordered_set<int> FIRST_decl;
extern std::unordered_set<int> FIRST_extern;
extern std::unordered_set<int> FIRST_type_spec;
extern std::unordered_set<int> FIRST_expr;

void parser();

#endif // PARSER_H