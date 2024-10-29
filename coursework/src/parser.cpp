#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include <iostream>
#include <deque>
#include <unordered_set>
#include <string>

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

TOKEN CurTok;
std::deque<TOKEN> tok_buffer;

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

inline void debugParserMsg(const std::string& parserFunc, const std::string& msg = "") {
    fprintf(stderr, "[%s] %s\n", parserFunc.c_str(), msg.c_str());
}

bool match(int expectedType) {
    if (CurTok.type == expectedType) {
        debugParserMsg("match", "Matched " + tokenTypeToString(expectedType));
        getNextToken();
        return true;
    } else {
        std::string error = "Expected token of type " + tokenTypeToString(expectedType) +
                           " (" + std::to_string(expectedType) + ")" +
                           ", got \"" + CurTok.lexeme + "\" with type " +
                           tokenTypeToString(CurTok.type) + " (" + 
                           std::to_string(CurTok.type) + ")";
        debugParserMsg("match", "ERROR: " + error);
        std::cerr << error << "\n";
        return false;
    }
}



//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//
std::unordered_set<int> FIRST_program = {EXTERN, INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_decl = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_extern = {EXTERN};
std::unordered_set<int> FIRST_type_spec = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_expr = {IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT, LPAR, NOT, MINUS};
std::unordered_set<int> FIRST_stmt = {LBRA, IF, WHILE, RETURN, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT, LPAR, NOT, MINUS, SC};
std::unordered_set<int> FIRST_block = {LBRA};

bool parseProgram() {
    getNextToken(); // Initialize the first token

    // Parse external declarations
    while (CurTok.type == EXTERN) {
        if (!parseExtern())
            return false;
    }

    // Parse declarations (must have at least one)
    if (!FIRST_decl.count(CurTok.type)) {
        std::cerr << "Expected declaration\n";
        return false;
    }

    while (FIRST_decl.count(CurTok.type)) {
        if (!parseDecl())
            return false;
    }

    return true;
}

bool parseDecl() {
    debugParserMsg("parseDecl", "Starting");
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        std::cerr << "Expected type specifier\n";
        return false;
    }

    TOKEN savedTok = CurTok;
    if (!parseTypeSpec()) return false;
    if (!match(IDENT)) return false;

    if (CurTok.type == LPAR) {
        // This is a function declaration
        if (!match(LPAR)) return false;
        if (!parseParams()) return false;
        if (!match(RPAR)) return false;
        
        // Check if this is a function definition (has a body) or just a declaration
        if (CurTok.type == LBRA) {
            if (!parseBlock()) return false;
        } else if (CurTok.type == SC) {
            if (!match(SC)) return false;
        } else {
            std::cerr << "Expected either function body or semicolon\n";
            return false;
        }
    } else {
        // This is a variable declaration
        if (!match(SC)) return false;
    }

    debugParserMsg("parseDecl", "Succeeded");
    return true;
}
bool parseLocalDecl() {
    if (!parseVarType()) return false;
    if (!match(IDENT)) return false;
    if (!match(SC)) return false;
    return true;
}

bool parseExprStmt() {
    // Empty statement
    if (CurTok.type == SC) {
        getNextToken();
        return true;
    }

    // Expression statement
    if (FIRST_expr.count(CurTok.type)) {
        if (!parseExpr()) return false;
        if (!match(SC)) return false;
        return true;
    }

    std::cerr << "Expected expression or semicolon\n";
    return false;
}

bool parseReturnStmt() {
    if (!match(RETURN)) return false;

    // Return with expression
    if (FIRST_expr.count(CurTok.type)) {
        if (!parseExpr()) return false;
    }

    if (!match(SC)) return false;
    return true;
}

bool parseExtern() {
    debugParserMsg("parseExtern", "Starting");
    
    if (!match(EXTERN)) return false;
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        std::cerr << "Expected type specifier\n";
        return false;
    }
    if (!parseTypeSpec()) return false;
    
    if (!match(IDENT)) return false;
    if (!match(LPAR)) return false;
    if (!parseParams()) return false;
    if (!match(RPAR)) return false;
    if (!match(SC)) return false;
    
    debugParserMsg("parseExtern", "Succeeded");
    return true;
}

bool parseTypeSpec() {
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return true;
    }
    return parseVarType();
}

bool parseVarType() {
    switch (CurTok.type) {
        case INT_TOK:
        case FLOAT_TOK:
        case BOOL_TOK:
            getNextToken();
            return true;
        default:
            std::cerr << "Expected var type\n";
            return false;
    }
}

bool parseParams() {
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return true;
    }
    if (FIRST_type_spec.count(CurTok.type)) {
        return parseParamList();
    }
    return true; // empty parameter list
}

bool parseParamList() {
    if (!parseParam()) return false;
    
    while (CurTok.type == COMMA) {
        getNextToken();
        if (!parseParam()) return false;
    }
    return true;
}

bool parseParam() {
    if (!parseVarType()) return false;
    if (!match(IDENT)) return false;
    return true;
}

bool parseBlock() {
    if (!match(LBRA)) return false;
    
    // Parse local declarations
    while (FIRST_type_spec.count(CurTok.type)) {
        if (!parseLocalDecl()) return false;
    }
    
    // Parse statements
    while (FIRST_stmt.count(CurTok.type)) {
        if (!parseStmt()) return false;
    }
    
    if (!match(RBRA)) return false;
    return true;
}

bool parseStmt() {
    if (CurTok.type == IF) {
        return parseIfStmt();
    } else if (CurTok.type == RETURN) {
        return parseReturnStmt();
    } else if (FIRST_expr.count(CurTok.type)) {
        return parseExprStmt();
    }
    std::cerr << "Invalid statement\n";
    return false;
}

bool parseIfStmt() {
    if (!match(IF)) return false;
    if (!match(LPAR)) return false;
    if (!parseExpr()) return false;
    if (!match(RPAR)) return false;
    if (!parseBlock()) return false;
    
    if (CurTok.type == ELSE) {
        getNextToken();
        if (!parseBlock()) return false;
    }
    return true;
}

bool parseExpr() {
    if (CurTok.type == IDENT) {
        TOKEN savedTok = CurTok;
        getNextToken();
        if (CurTok.type == ASSIGN) {
            getNextToken();
            return parseExpr();
        }
        putBackToken(CurTok);
        CurTok = savedTok;
    }
    return parseLogicOr();
}

bool parseLogicOr() {
    if (!parseLogicAnd()) return false;
    
    while (CurTok.type == OR) {
        getNextToken();
        if (!parseLogicAnd()) return false;
    }
    return true;
}

bool parseLogicAnd() {
    if (!parseEquality()) return false;
    
    while (CurTok.type == AND) {
        getNextToken();
        if (!parseEquality()) return false;
    }
    return true;
}

bool parseEquality() {
    if (!parseRelation()) return false;
    
    while (CurTok.type == EQ || CurTok.type == NOT) {
        getNextToken();
        if (!parseRelation()) return false;
    }
    return true;
}

bool parseRelation() {
    if (!parseAdditive()) return false;
    
    while (CurTok.type == LT || CurTok.type == GT || 
           CurTok.type == LE || CurTok.type == GE) {
        getNextToken();
        if (!parseAdditive()) return false;
    }
    return true;
}

bool parseAdditive() {
    if (!parseMultiply()) return false;
    
    while (CurTok.type == PLUS || CurTok.type == MINUS) {
        getNextToken();
        if (!parseMultiply()) return false;
    }
    return true;
}

bool parseMultiply() {
    if (!parseUnary()) return false;
    
    while (CurTok.type == ASTERIX || CurTok.type == DIV || CurTok.type == MOD) {
        getNextToken();
        if (!parseUnary()) return false;
    }
    return true;
}

bool parseUnary() {
    if (CurTok.type == MINUS || CurTok.type == NOT) {
        getNextToken();
        return parseUnary();
    }
    return parsePrimary();
}

bool parsePrimary() {
    switch (CurTok.type) {
        case LPAR:
            getNextToken();
            if (!parseExpr()) return false;
            return match(RPAR);
            
        case IDENT: {
            getNextToken();
            if (CurTok.type == LPAR) {
                // Function call
                getNextToken();
                if (!parseArgs()) return false;
                return match(RPAR);
            }
            return true;
        }
            
        case INT_LIT:
        case FLOAT_LIT:
        case BOOL_LIT:
            getNextToken();
            return true;
            
        default:
            std::cerr << "Expected primary expression\n";
            return false;
    }
}

bool parseArgs() {
    if (!FIRST_expr.count(CurTok.type)) {
        return true; // empty argument list
    }
    
    if (!parseExpr()) return false;
    
    while (CurTok.type == COMMA) {
        getNextToken();
        if (!parseExpr()) return false;
    }
    
    return true;
}

void parser() {
    if (parseProgram()) {
        std::cout << "Parsing succeeded\n";
    } else {
        std::cerr << "Parsing failed\n";
    }
}