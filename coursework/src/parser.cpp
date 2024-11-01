#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include <iostream>
#include <deque>
#include <unordered_set>
#include <string>
#include <memory>

TOKEN CurTok;
std::deque<TOKEN> tok_buffer;

// Keep the token management functions the same
TOKEN getNextToken() {
    if (tok_buffer.empty())
        tok_buffer.push_back(gettok());

    TOKEN temp = tok_buffer.front();
    tok_buffer.pop_front();

    fprintf(stderr, "getNextToken: \"%s\" with type %s (%d)\n", temp.lexeme.c_str(),
            tokenTypeToString(temp.type).c_str(), temp.type);
    CurTok = temp;
    return CurTok;
}

void putBackToken(TOKEN tok) {
    fprintf(stderr, "putBackToken: \"%s\" with type %s (%d)\n", tok.lexeme.c_str(),
            tokenTypeToString(tok.type).c_str(), tok.type);
    tok_buffer.push_front(tok);
}

// First sets (keep only the ones we need)
std::unordered_set<int> FIRST_program = {EXTERN, INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_decl = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_extern = {EXTERN};
std::unordered_set<int> FIRST_type_spec = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_expr = {IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT, LPAR, NOT, MINUS};

// Debug function to print the current function being called
void debugPrint(const std::string& functionName) {
    std::cerr << "Entering " << functionName << "\n";
}

// Modified parser functions to return AST nodes
std::unique_ptr<ProgramNode> parseProgram() {
    debugPrint("parseProgram");
    getNextToken(); // Initialize the first token
    
    std::vector<std::unique_ptr<ASTnode>> externs;
    std::vector<std::unique_ptr<ASTnode>> declarations;

    // Parse external declarations
    while (CurTok.type == EXTERN) {
        auto ext = parseExtern();
        if (!ext) return nullptr;
        externs.push_back(std::move(ext));
    }

    // Parse declarations
    while (FIRST_decl.count(CurTok.type)) {
        auto decl = parseDecl();
        if (!decl) return nullptr;
        declarations.push_back(std::move(decl));
    }

    return std::make_unique<ProgramNode>(std::move(externs), std::move(declarations));
}

std::unique_ptr<ExternNode> parseExtern() {
    debugPrint("parseExtern");
    if (CurTok.type != EXTERN) return nullptr;
    getNextToken();
    
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) return nullptr;
    
    if (CurTok.type != IDENT) return nullptr;
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type != LPAR) return nullptr;
    getNextToken();
    
    auto params = parseParams();
    if (!params) return nullptr;
    
    if (CurTok.type != RPAR) return nullptr;
    getNextToken();
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<ExternNode>(returnType, name, std::move(*params));
}

std::unique_ptr<ASTnode> parseDecl() {
    debugPrint("parseDecl");
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) return nullptr;
    
    if (CurTok.type != IDENT) return nullptr;
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type == LPAR) {
        // Function declaration
        getNextToken();
        auto params = parseParams();
        if (!params) return nullptr;
        
        if (CurTok.type != RPAR) return nullptr;
        getNextToken();
        
        // Check if this is a function definition (has a body) or just a declaration
        if (CurTok.type == LBRA) {
            auto body = parseBlock();
            if (!body) return nullptr;
            return std::make_unique<FunctionNode>(returnType, name, std::move(*params), std::move(body));
        } else if (CurTok.type == SC) {
            getNextToken(); // Consume ';'
            return std::make_unique<FunctionNode>(returnType, name, std::move(*params), nullptr);
        } else {
            std::cerr << "Expected either function body or semicolon\n";
            return nullptr;
        }
    } else {
        // Variable declaration
        if (CurTok.type != SC) return nullptr;
        getNextToken();
        return std::make_unique<VarDeclNode>(returnType, name);
    }
}

std::unique_ptr<BlockNode> parseBlock() {
    debugPrint("parseBlock");
    if (CurTok.type != LBRA) return nullptr;
    getNextToken();
    
    std::vector<std::unique_ptr<ASTnode>> declarations;
    std::vector<std::unique_ptr<ASTnode>> statements;
    
    // Parse local declarations
    while (FIRST_type_spec.count(CurTok.type)) {
        auto decl = parseLocalDecl();
        if (!decl) return nullptr;
        declarations.push_back(std::move(decl));
    }
    
    // Parse statements
    while (CurTok.type != RBRA) {
        auto stmt = parseStmt();
        if (!stmt) return nullptr;
        statements.push_back(std::move(stmt));
    }
    
    getNextToken(); // consume RBRA
    return std::make_unique<BlockNode>(std::move(declarations), std::move(statements));
}

std::unique_ptr<ASTnode> parseStmt() {
    debugPrint("parseStmt");
    switch (CurTok.type) {
        case IF:
            return parseIfStmt();
        case WHILE:  // Add this case
            return parseWhile();
        case RETURN:
            return parseReturnStmt();
        default:
            if (FIRST_expr.count(CurTok.type))
                return parseExprStmt();
            return nullptr;
    }
}

std::unique_ptr<IfNode> parseIfStmt() {
    debugPrint("parseIfStmt");
    getNextToken(); // consume IF
    
    if (CurTok.type != LPAR) return nullptr;
    getNextToken();
    
    auto condition = parseExpr();
    if (!condition) return nullptr;
    
    if (CurTok.type != RPAR) return nullptr;
    getNextToken();
    
    auto thenBlock = parseBlock();
    if (!thenBlock) return nullptr;
    
    std::unique_ptr<ASTnode> elseBlock = nullptr;
    if (CurTok.type == ELSE) {
        getNextToken();
        elseBlock = parseBlock();
        if (!elseBlock) return nullptr;
    }
    
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBlock), std::move(elseBlock));
}

std::unique_ptr<ASTnode> parseExprStmt() {
    debugPrint("parseExprStmt");
    auto expr = parseExpr();
    if (!expr) return nullptr;
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<ExprStmtNode>(std::move(expr));
}

std::unique_ptr<ReturnNode> parseReturnStmt() {
    debugPrint("parseReturnStmt");
    getNextToken(); // consume RETURN
    
    if (CurTok.type == SC) {
        getNextToken(); // consume ';'
        return std::make_unique<ReturnNode>(nullptr); // void return
    }
    
    auto expr = parseExpr();
    if (!expr) return nullptr;
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<ReturnNode>(std::move(expr));
}

std::unique_ptr<ASTnode> parseRelational() {
    debugPrint("parseRelational");
    auto left = parseAdditive();
    if (!left) return nullptr;
    
    while (CurTok.type == LT || CurTok.type == GT || 
           CurTok.type == LE || CurTok.type == GE) {
        std::string op;
        switch (CurTok.type) {
            case LT: op = "<"; break;
            case GT: op = ">"; break;
            case LE: op = "<="; break;
            case GE: op = ">="; break;
        }
        getNextToken();
        auto right = parseAdditive();
        if (!right) return nullptr;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ASTnode> parseEquality() {
    debugPrint("parseEquality");
    auto left = parseRelational();  // Changed from parseAdditive
    if (!left) return nullptr;
    
    while (CurTok.type == EQ || CurTok.type == NE) {
        std::string op = CurTok.type == EQ ? "==" : "!=";
        getNextToken();
        auto right = parseRelational();  // Changed from parseAdditive
        if (!right) return nullptr;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

// Modify parseExpr to use parseEquality instead of parseAdditive
std::unique_ptr<ASTnode> parseExpr() {
    debugPrint("parseExpr");
    if (CurTok.type == IDENT) {
        TOKEN savedTok = CurTok;
        getNextToken();
        if (CurTok.type == ASSIGN) {
            getNextToken();
            auto rhs = parseExpr();
            if (!rhs) return nullptr;
            return std::make_unique<AssignNode>(savedTok.lexeme, std::move(rhs));
        }
        putBackToken(CurTok);
        CurTok = savedTok;
    }
    return parseEquality();  // Changed from parseAdditive to parseEquality
}

std::unique_ptr<ASTnode> parseAdditive() {
    debugPrint("parseAdditive");
    auto left = parseMultiply();
    if (!left) return nullptr;
    
    while (CurTok.type == PLUS || CurTok.type == MINUS) {
        std::string op = CurTok.type == PLUS ? "+" : "-";
        getNextToken();
        auto right = parseMultiply();
        if (!right) return nullptr;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ASTnode> parseMultiply() {
    debugPrint("parseMultiply");
    auto left = parseUnary(); // Changed from parsePrimary
    if (!left) return nullptr;
    
    while (CurTok.type == ASTERIX || CurTok.type == DIV || CurTok.type == MOD) {
        std::string op;
        switch (CurTok.type) {
            case ASTERIX: op = "*"; break;
            case DIV: op = "/"; break;
            case MOD: op = "%"; break;
        }
        getNextToken();
        auto right = parseUnary(); // Changed from parsePrimary
        if (!right) return nullptr;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ASTnode> parsePrimary() {
    debugPrint("parsePrimary");
    switch (CurTok.type) {
        case IDENT: {
            std::string name = CurTok.lexeme;
            getNextToken();
            if (CurTok.type == LPAR) {
                getNextToken();
                auto args = parseArgs();
                if (!args) return nullptr;
                if (CurTok.type != RPAR) {
                    std::cerr << "Expected ')' after function arguments\n";
                    return nullptr;
                }
                getNextToken();
                return std::make_unique<FunctionCallNode>(name, std::move(*args));
            }
            return std::make_unique<VariableNode>(name);
        }
        case INT_LIT: {
            int val = std::stoi(CurTok.lexeme);
            getNextToken();
            return std::make_unique<LiteralNode>(val);
        }
        case FLOAT_LIT: {
            float val = std::stof(CurTok.lexeme);
            getNextToken();
            return std::make_unique<LiteralNode>(val);
        }
        case BOOL_LIT: {
            bool val = (CurTok.lexeme == "true");
            getNextToken();
            return std::make_unique<LiteralNode>(val);
        }
        case LPAR: {
            getNextToken();
            auto expr = parseExpr();
            if (!expr) return nullptr;
            if (CurTok.type != RPAR) {
                std::cerr << "Expected ')'\n";
                return nullptr;
            }
            getNextToken();
            return expr;
        }
        default:
            std::cerr << "Unexpected token in primary expression\n";
            return nullptr;
    }
}

std::unique_ptr<ASTnode> parseUnary() {
    debugPrint("parseUnary");
    if (CurTok.type == MINUS || CurTok.type == NOT) {
        std::string op = (CurTok.type == MINUS) ? "-" : "!";
        getNextToken(); // Consume the unary operator
        
        auto operand = parseUnary(); // Recursive call to handle nested unary expressions
        if (!operand) return nullptr;
        
        return std::make_unique<UnaryOpNode>(op, std::move(operand));
    }
    
    return parsePrimary(); // If no unary operator, parse as primary expression
}

std::unique_ptr<ASTnode> parseLocalDecl() {
    debugPrint("parseLocalDecl");
    std::string type = parseTypeSpec();
    if (type.empty()) return nullptr;
    
    if (CurTok.type != IDENT) return nullptr;
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<VarDeclNode>(type, name);
}

// Helper functions
std::string parseTypeSpec() {
    debugPrint("parseTypeSpec");
    if (!FIRST_type_spec.count(CurTok.type)) return "";
    std::string type = CurTok.lexeme;
    getNextToken();
    return type;
}

std::unique_ptr<WhileNode> parseWhile() {
    debugPrint("parseWhile");
    getNextToken(); // consume WHILE
    
    if (CurTok.type != LPAR) {
        std::cerr << "Expected '(' after while\n";
        return nullptr;
    }
    getNextToken();
    
    auto condition = parseExpr();
    if (!condition) return nullptr;
    
    if (CurTok.type != RPAR) {
        std::cerr << "Expected ')' after while condition\n";
        return nullptr;
    }
    getNextToken();
    
    auto body = parseBlock();
    if (!body) return nullptr;
    
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseParams() {
    debugPrint("parseParams");
    std::vector<std::pair<std::string, std::string>> params;
    
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return params;
    }
    
    while (true) {
        if (!FIRST_type_spec.count(CurTok.type)) break;
        
        std::string type = parseTypeSpec();
        if (type.empty()) return std::nullopt;
        
        if (CurTok.type != IDENT) return std::nullopt;
        std::string name = CurTok.lexeme;
        getNextToken();
        
        params.push_back({type, name});
        
        if (CurTok.type != COMMA) break;
        getNextToken();
    }
    
    return params;
}

std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgs() {
    debugPrint("parseArgs");
    std::vector<std::unique_ptr<ASTnode>> args;
    
    if (!FIRST_expr.count(CurTok.type)) {
        return args;
    }
    
    while (true) {
        auto arg = parseExpr();
        if (!arg) return std::nullopt;
        args.push_back(std::move(arg));
        
        if (CurTok.type != COMMA) break;
        getNextToken();
    }
    
    return args;
}

void parser() {
    debugPrint("parser");
    auto program = parseProgram();
    if (program) {
        std::cout << "Parsing Finished\n";
        std::cout << program->to_string();
    } else {
        std::cerr << "Parsing failed\n";
    }
}