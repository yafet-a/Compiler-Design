#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include <iostream>
#include <deque>

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
TOKEN CurTok;
std::deque<TOKEN> tok_buffer;

TOKEN getNextToken() {
  if (tok_buffer.size() == 0)
    tok_buffer.push_back(gettok());

  TOKEN temp = tok_buffer.front();
  tok_buffer.pop_front();

  fprintf(stderr, "getNextToken: \"%s\" with type %s (%d)\n", temp.lexeme.c_str(), tokenTypeToString(temp.type).c_str(), temp.type);
  return CurTok = temp;
}

void putBackToken(TOKEN tok) {
  fprintf(stderr, "putBackToken: \"%s\" with type %s (%d)\n", tok.lexeme.c_str(), tokenTypeToString(tok.type).c_str(), tok.type);
  tok_buffer.push_front(tok);
}

//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//

std::unique_ptr<ASTnode> parsePrimary() {
  fprintf(stderr, "parsePrimary: Current token: \"%s\" with type %s (%d)\n", CurTok.lexeme.c_str(), tokenTypeToString(CurTok.type).c_str(), CurTok.type);
  if (CurTok.type == INT_LIT) {
    auto Result = std::make_unique<IntASTnode>(CurTok, std::stoi(CurTok.lexeme));
    getNextToken(); // consume the integer literal
    return std::move(Result);
  } else if (CurTok.type == IDENT) {
    auto Result = std::make_unique<IdentifierASTnode>(CurTok, CurTok.lexeme);
    getNextToken(); // consume the identifier
    return std::move(Result);
  }
  // Handle other primary expressions
  return nullptr;
}

std::unique_ptr<ASTnode> parseUnary() {
  if (CurTok.type == MINUS || CurTok.type == NOT) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '-' or '!'
    auto Operand = parseUnary();
    if (!Operand) return nullptr;
    return std::make_unique<UnaryExprASTnode>(Tok, std::move(Operand));
  }
  return parsePrimary();
}

std::unique_ptr<ASTnode> parseMultiply() {
  auto LHS = parseUnary();
  if (!LHS) return nullptr;

  while (CurTok.type == ASTERIX || CurTok.type == DIV || CurTok.type == MOD) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '*' or '/' or '%'
    auto RHS = parseUnary();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseAdditive() {
  auto LHS = parseMultiply();
  if (!LHS) return nullptr;

  while (CurTok.type == PLUS || CurTok.type == MINUS) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '+' or '-'
    auto RHS = parseMultiply();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseRelation() {
  auto LHS = parseAdditive();
  if (!LHS) return nullptr;

  while (CurTok.type == LE || CurTok.type == LT || CurTok.type == GE || CurTok.type == GT) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '<=' or '<' or '>=' or '>'
    auto RHS = parseAdditive();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseEquality() {
  auto LHS = parseRelation();
  if (!LHS) return nullptr;

  while (CurTok.type == EQ || CurTok.type == NE) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '==' or '!='
    auto RHS = parseRelation();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseLogicAnd() {
  auto LHS = parseEquality();
  if (!LHS) return nullptr;

  while (CurTok.type == AND) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '&&'
    auto RHS = parseEquality();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseLogicOr() {
  auto LHS = parseLogicAnd();
  if (!LHS) return nullptr;

  while (CurTok.type == OR) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume '||'
    auto RHS = parseLogicAnd();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprASTnode>(Tok, std::move(LHS), std::move(RHS));
  }
  return LHS;
}

std::unique_ptr<ASTnode> parseAssignExpr() {
  if (CurTok.type == IDENT) {
    TOKEN Tok = CurTok;
    getNextToken(); // consume identifier
    if (CurTok.type == ASSIGN) {
      getNextToken(); // consume '='
      auto RHS = parseAssignExpr();
      if (!RHS) return nullptr;
      return std::make_unique<BinaryExprASTnode>(Tok, std::make_unique<IdentifierASTnode>(Tok, Tok.lexeme), std::move(RHS));
    }
    putBackToken(Tok);
  }
  return parseLogicOr();
}

std::unique_ptr<ASTnode> parseExpr() {
  return parseAssignExpr();
}

std::unique_ptr<ASTnode> parseReturnStmt() {
  TOKEN Tok = CurTok;
  getNextToken(); // consume 'return'
  if (CurTok.type == SC) {
    getNextToken(); // consume ';'
    return std::make_unique<ReturnASTnode>(Tok, nullptr);
  }
  auto Expr = parseExpr();
  if (!Expr) return nullptr;
  if (CurTok.type != SC) {
    std::cerr << "Expected ';' after return statement, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume ';'
  return std::make_unique<ReturnASTnode>(Tok, std::move(Expr));
}

std::unique_ptr<ASTnode> parseStmt() {
  if (CurTok.type == RETURN) {
    return parseReturnStmt();
  }
  return parseExprStmt();
}

std::unique_ptr<ASTnode> parseStmtList() {
  std::vector<std::unique_ptr<ASTnode>> Stmts;
  while (CurTok.type != RBRA) {
    auto Stmt = parseStmt();
    if (!Stmt) return nullptr;
    Stmts.push_back(std::move(Stmt));
  }
  return std::make_unique<StmtListASTnode>(std::move(Stmts));
}

std::unique_ptr<ASTnode> parseLocalDecls() {
  std::vector<std::unique_ptr<ASTnode>> Decls;
  while (CurTok.type == INT_TOK || CurTok.type == FLOAT_TOK || CurTok.type == BOOL_TOK) {
    auto Decl = parseLocalDecl();
    if (!Decl) return nullptr;
    Decls.push_back(std::move(Decl));
  }
  return std::make_unique<LocalDeclsASTnode>(std::move(Decls));
}

std::unique_ptr<ASTnode> parseBlock() {
  if (CurTok.type != LBRA) {
    std::cerr << "Expected '{' at the beginning of block, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume '{'
  auto LocalDecls = parseLocalDecls();
  if (!LocalDecls) return nullptr;
  auto StmtList = parseStmtList();
  if (!StmtList) return nullptr;
  if (CurTok.type != RBRA) {
    std::cerr << "Expected '}' at the end of block, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume '}'
  return std::make_unique<BlockASTnode>(std::move(LocalDecls), std::move(StmtList));
}

std::unique_ptr<ASTnode> parseParam() {
  if (CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK) {
    std::cerr << "Expected type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  TOKEN typeTok = CurTok;
  std::string type = CurTok.lexeme;
  getNextToken(); // consume type specifier
  if (CurTok.type != IDENT) {
    std::cerr << "Expected identifier after type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  std::string name = CurTok.lexeme;
  TOKEN nameTok = CurTok;
  getNextToken(); // consume identifier
  return std::make_unique<VarDeclASTnode>(nameTok, type, name);
}

std::vector<std::unique_ptr<VarDeclASTnode>> parseParamList() {
  std::vector<std::unique_ptr<VarDeclASTnode>> params;
  if (CurTok.type != RPAR) {
    while (true) {
      auto param = parseParam();
      if (!param) {
        return {};
      }
      params.push_back(std::move(param));
      if (CurTok.type == RPAR) {
        break;
      }
      if (CurTok.type != COMMA) {
        std::cerr << "Expected ',' or ')', got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
        return {};
      }
      getNextToken(); // consume ','
    }
  }
  return params;
}

std::unique_ptr<ASTnode> parseParams() {
  if (CurTok.type == VOID_TOK) {
    getNextToken(); // consume 'void'
    return nullptr;
  }
  auto params = parseParamList();
  return std::make_unique<ParamListASTnode>(std::move(params));
}

std::unique_ptr<ASTnode> parseFunDecl() {
  if (CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK && CurTok.type != VOID_TOK) {
    std::cerr << "Expected type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  TOKEN typeTok = CurTok;
  std::string returnType = CurTok.lexeme;
  getNextToken(); // consume type specifier
  if (CurTok.type != IDENT) {
    std::cerr << "Expected function name after type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  std::string name = CurTok.lexeme;
  TOKEN nameTok = CurTok;
  getNextToken(); // consume identifier
  if (CurTok.type != LPAR) {
    std::cerr << "Expected '(' after function name, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume '('
  auto params = parseParams();
  if (CurTok.type != RPAR) {
    std::cerr << "Expected ')' after parameters, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume ')'
  auto body = parseBlock();
  if (!body) return nullptr;
  return std::make_unique<FunctionASTnode>(typeTok, returnType, name, std::move(params), std::move(body));
}

std::unique_ptr<ASTnode> parseVarDecl() {
  if (CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK) {
    std::cerr << "Expected type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  TOKEN typeTok = CurTok;
  std::string type = CurTok.lexeme;
  getNextToken(); // consume type specifier
  if (CurTok.type != IDENT) {
    std::cerr << "Expected identifier after type specifier, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  std::string name = CurTok.lexeme;
  TOKEN nameTok = CurTok;
  getNextToken(); // consume identifier
  if (CurTok.type != SC) {
    std::cerr << "Expected ';' after variable declaration, got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume ';'
  return std::make_unique<VarDeclASTnode>(nameTok, type, name);
}

std::unique_ptr<ASTnode> parseDecl() {
  if (CurTok.type == INT_TOK || CurTok.type == FLOAT_TOK || CurTok.type == BOOL_TOK) {
    return parseVarDecl();
  }
  return parseFunDecl();
}

std::unique_ptr<ASTnode> parseDeclList() {
  std::vector<std::unique_ptr<ASTnode>> Decls;
  while (CurTok.type == INT_TOK || CurTok.type == FLOAT_TOK || CurTok.type == BOOL_TOK || CurTok.type == VOID_TOK) {
    auto Decl = parseDecl();
    if (!Decl) return nullptr;
    Decls.push_back(std::move(Decl));
  }
  return std::make_unique<DeclListASTnode>(std::move(Decls));
}

std::unique_ptr<ASTnode> parseExtern() {
  if (CurTok.type != EXTERN) {
    std::cerr << "Expected 'extern', got: \"" << CurTok.lexeme << "\" with type " << tokenTypeToString(CurTok.type) << " (" << CurTok.type << ")\n";
    return nullptr;
  }
  getNextToken(); // consume 'extern'
  auto decl = parseDecl();
  if (!decl) return nullptr;
  return decl;
}

std::unique_ptr<ASTnode> parseExternList() {
  std::vector<std::unique_ptr<ASTnode>> Externs;
  while (CurTok.type == EXTERN) {
    auto Extern = parseExtern();
    if (!Extern) return nullptr;
    Externs.push_back(std::move(Extern));
  }
  return std::make_unique<ExternListASTnode>(std::move(Externs));
}

std::unique_ptr<ASTnode> parseProgram() {
  auto externList = parseExternList();
  auto declList = parseDeclList();
  if (!externList && !declList) return nullptr;
  return std::make_unique<ProgramASTnode>(std::move(externList), std::move(declList));
}

void parser() {
  auto AST = parseProgram();
  if (AST) {
    std::cout << *AST << "\n";
  } else {
    std::cerr << "Parsing failed" << std::endl;
  }
}