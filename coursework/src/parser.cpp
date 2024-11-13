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

//Error Handling
void emitError(const std::string& message, const TOKEN& token) {
    // Save current file position
    long currentPos = ftell(pFile);
    
    // Go to start of the line containing the error
    fseek(pFile, 0, SEEK_SET);
    int currentLine = 1;
    std::string line;
    char buffer[1024];
    
    while (currentLine < token.lineNo && fgets(buffer, sizeof(buffer), pFile)) {
        currentLine++;
    }
    
    // Read the error line
    if (fgets(buffer, sizeof(buffer), pFile)) {
        line = buffer;
        // Remove trailing newline if present
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
    }
    
    // Restore file position
    fseek(pFile, currentPos, SEEK_SET);
    
    // Print error message with location and context
    fprintf(stderr, "error:%d:%d: %s\n", token.lineNo, token.columnNo, message.c_str());
    if (!line.empty()) {
        fprintf(stderr, "%s\n", line.c_str());
        // Print caret
        std::string caretLine(token.columnNo - 1, ' ');
        caretLine += "^";
        fprintf(stderr, "%s\n", caretLine.c_str());
    }
}

bool expectToken(int expectedType, const std::string& context) {
    if (CurTok.type != expectedType) {
        std::string msg = "Expected " + tokenTypeToString(expectedType) +
                         " but got " + tokenTypeToString(CurTok.type) +
                         " in " + context;
        emitError(msg, CurTok);
        return false;
    }
    return true;
}

SourceLocation captureLocation() {
    return SourceLocation{currentFilename, CurTok.lineNo, CurTok.columnNo, currentLineContent};
}

// Keep the token management functions the same
TOKEN getNextToken() {
    if (tok_buffer.empty())
        tok_buffer.push_back(gettok());

    TOKEN temp = tok_buffer.front();
    tok_buffer.pop_front();

    fprintf(stderr, "getNextToken: \"%s\" with type %s (%d) at line %d\n", temp.lexeme.c_str(),
            tokenTypeToString(temp.type).c_str(), temp.type, temp.lineNo);
    CurTok = temp;
    return CurTok;
}

void putBackToken(TOKEN tok) {
    fprintf(stderr, "putBackToken: \"%s\" with type %s (%d) at line %d\n", tok.lexeme.c_str(),
            tokenTypeToString(tok.type).c_str(), tok.type, tok.lineNo);
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
    std::cerr << "Entering " << functionName << " at line " << CurTok.lineNo << "\n";
}

//program ::= extern_list decl_list 
//          | decl_list
std::unique_ptr<ProgramNode> parseProgram() {
    SourceLocation loc = captureLocation();
    if (!FIRST_program.count(CurTok.type)) {
        emitError("Expected program to start with extern, int, float, bool, or void", CurTok);
        return nullptr;
    }

    std::vector<std::unique_ptr<ASTnode>> externs;
    std::vector<std::unique_ptr<ASTnode>> declarations;

    if (CurTok.type == EXTERN) {
        auto externList = parseExternList();
        if (!externList) return nullptr;
        externs = std::move(externList->externs);
    }

    auto declList = parseDeclList();
    if (!declList) return nullptr;
    declarations = std::move(declList->declarations);

    return std::make_unique<ProgramNode>(std::move(externs), std::move(declarations), loc);
}

// extern_list ::= extern extern_list'
std::unique_ptr<ExternListNode> parseExternList() {
    SourceLocation loc = captureLocation();
    if (!expectToken(EXTERN, "start of extern list")) return nullptr;
    
    std::vector<std::unique_ptr<ASTnode>> externs;
    auto firstExtern = parseExtern();
    if (!firstExtern) {
        emitError("Invalid extern declaration", CurTok);
        return nullptr;
    }
    externs.push_back(std::move(firstExtern));
    
    return parseExternListPrime(std::move(externs), loc);
}

// extern_list' ::= extern extern_list'
//                | epsilon
std::unique_ptr<ExternListNode> parseExternListPrime(std::vector<std::unique_ptr<ASTnode>>&& externs, SourceLocation loc) {
    debugPrint("parseExternListPrime");
    
    if (CurTok.type == EXTERN) {
        auto nextExtern = parseExtern();
        if (!nextExtern) return nullptr;
        externs.push_back(std::move(nextExtern));
        return parseExternListPrime(std::move(externs), loc);
    }
    
    // epsilon production
    return std::make_unique<ExternListNode>(std::move(externs), loc);
}

// decl_list ::= decl decl_list'
std::unique_ptr<DeclListNode> parseDeclList() {
    SourceLocation loc = captureLocation();  
    
    debugPrint("parseDeclList");
    std::vector<std::unique_ptr<ASTnode>> declarations;

    auto firstDecl = parseDecl();
    if (!firstDecl) {
        std::cerr << "Expected declaration at start of declaration list\n";
        return nullptr;
    }
    declarations.push_back(std::move(firstDecl));
    
    return parseDeclListPrime(std::move(declarations), loc);
}

// decl_list' ::= decl decl_list'
//              | epsilon
std::unique_ptr<DeclListNode> parseDeclListPrime(std::vector<std::unique_ptr<ASTnode>> declarations, SourceLocation loc) {
    debugPrint("parseDeclListPrime");
    
    if (FIRST_decl.count(CurTok.type)) {
        auto nextDecl = parseDecl();
        if (!nextDecl) return nullptr;
        declarations.push_back(std::move(nextDecl));
        return parseDeclListPrime(std::move(declarations), loc);
    }
    
    // epsilon production
    return std::make_unique<DeclListNode>(std::move(declarations), loc);
}

//extern ::= "extern" type_spec IDENT "(" params ")" ";"
std::unique_ptr<ExternNode> parseExtern() {
    SourceLocation loc{currentFilename, CurTok.lineNo, CurTok.columnNo, ""};
    
    if (!expectToken(EXTERN, "extern declaration")) return nullptr;
    getNextToken();
    
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) return nullptr;
    
    if (!expectToken(IDENT, "extern function name")) return nullptr;
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (!expectToken(LPAR, "extern parameter list")) return nullptr;
    getNextToken();
    
    auto params = parseParams();
    if (!params) return nullptr;
    
    if (!expectToken(RPAR, "closing extern parameter list")) return nullptr;
    getNextToken();
    
    if (!expectToken(SC, "extern declaration")) return nullptr;
    getNextToken();
    
    return std::make_unique<ExternNode>(returnType, name, std::move(*params), loc);
}

std::unique_ptr<ASTnode> parseDecl() {
    debugPrint("parseDecl");
    
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) {
        std::cerr << "Expected type specifier in declaration\n";
        return nullptr;
    }

    // Capture location at the identifier
    if (CurTok.type != IDENT) {
        std::cerr << "Expected identifier after type '" << returnType << "'\n";
        return nullptr;
    }
    
    // Capture location here where we know we have an identifier
    SourceLocation loc = captureLocation();
    std::string name = CurTok.lexeme;
    getNextToken();

    if (CurTok.type == LPAR) {
        // Function declaration/definition
        getNextToken();
        auto params = parseParams();
        if (!params) return nullptr;

        if (CurTok.type != RPAR) return nullptr;
        getNextToken();

        if (CurTok.type == LBRA) {
            auto body = parseBlock();
            if (!body) return nullptr;
            return std::make_unique<FunctionNode>(returnType, name, 
                                                std::move(*params), 
                                                std::move(body), 
                                                loc);  // Pass location
        } else if (CurTok.type == SC) {
            getNextToken();
            return std::make_unique<FunctionNode>(returnType, name, 
                                                std::move(*params), 
                                                nullptr, 
                                                loc);  // Pass location
        }
        return nullptr;
    } else {
        // Variable declaration
        if (CurTok.type != SC) return nullptr;
        getNextToken();
        return std::make_unique<VarDeclNode>(returnType, name, loc);  // Pass location
    }
}

// block ::= "{" local_decls stmt_list "}"
std::unique_ptr<BlockNode> parseBlock() {
    debugPrint("parseBlock");
    SourceLocation loc = captureLocation();
    if (CurTok.type != LBRA) {
        std::cerr << "Expected '{' at start of block\n";
        return nullptr;
    }
    getNextToken();
    
    // Parse local declarations
    auto declarations = parseLocalDecls();
    if (!declarations) {
        std::cerr << "Error parsing local declarations\n";
        return nullptr;
    }
    
    // Parse statements
    auto statements = parseStmtList();
    
    if (CurTok.type != RBRA) {
        std::cerr << "Expected '}' at end of block\n";
        return nullptr;
    }
    getNextToken();
    
    return std::make_unique<BlockNode>(std::move(declarations->declarations), std::move(statements), loc);
}

// local_decls ::= local_decls local_decl
//               | epsilon
std::unique_ptr<DeclListNode> parseLocalDecls() {
    debugPrint("parseLocalDecls");
    
    // local_decls ::= local_decls local_decl | epsilon
    if (!FIRST_type_spec.count(CurTok.type)) {
        // epsilon production
        return std::make_unique<DeclListNode>(std::vector<std::unique_ptr<ASTnode>>());
    }
    
    std::vector<std::unique_ptr<ASTnode>> decls;
    auto firstDecl = parseLocalDecl();
    if (!firstDecl) return nullptr;
    decls.push_back(std::move(firstDecl));
    
    return parseLocalDeclsPrime(std::move(decls));
}

// local_decls' ::= local_decl local_decls' | epsilon
std::unique_ptr<DeclListNode> parseLocalDeclsPrime(std::vector<std::unique_ptr<ASTnode>>&& decls) {
    debugPrint("parseLocalDeclsPrime");
    
    if (FIRST_type_spec.count(CurTok.type)) {
        auto nextDecl = parseLocalDecl();
        if (!nextDecl) return nullptr;
        decls.push_back(std::move(nextDecl));
        return parseLocalDeclsPrime(std::move(decls));
    }
    
    // epsilon production
    return std::make_unique<DeclListNode>(std::move(decls));
}

// stmt ::= expr_stmt
//        | block
//        | if_stmt
//        | while_stmt
//        | return_stmt
std::unique_ptr<ASTnode> parseStmt() {
    debugPrint("parseStmt");
    switch (CurTok.type) {
        case IF:
            return parseIfStmt();
        case WHILE:
            return parseWhile();
        case RETURN:
            return parseReturnStmt();
        case LBRA:
            return parseBlock();  // matches "stmt ::= block"
        case SC:
            getNextToken(); // consume the semicolon
            return std::make_unique<ExprStmtNode>(nullptr); // empty expr_stmt
        default:
            if (FIRST_expr.count(CurTok.type))
                return parseExprStmt();
            std::cerr << "Unexpected token in statement\n";
            return nullptr;
    }
}

// stmt_list ::= stmt_list stmt
//             | epsilon
std::vector<std::unique_ptr<ASTnode>> parseStmtList() {
    debugPrint("parseStmtList");
    
    // stmt_list ::= stmt_list stmt | epsilon
    if (CurTok.type == RBRA) {
        // epsilon production
        return std::vector<std::unique_ptr<ASTnode>>();
    }
    
    std::vector<std::unique_ptr<ASTnode>> stmts;
    auto firstStmt = parseStmt();
    if (!firstStmt) return std::vector<std::unique_ptr<ASTnode>>();
    stmts.push_back(std::move(firstStmt));
    
    return parseStmtListPrime(std::move(stmts));
}

//stmt_list' ::= stmt stmt_list' | epsilon
std::vector<std::unique_ptr<ASTnode>> parseStmtListPrime(std::vector<std::unique_ptr<ASTnode>>&& stmts) {
    debugPrint("parseStmtListPrime");
    
    if (CurTok.type != RBRA) {  // If we haven't reached the end of the block
        auto nextStmt = parseStmt();
        if (!nextStmt) return std::vector<std::unique_ptr<ASTnode>>();
        stmts.push_back(std::move(nextStmt));
        return parseStmtListPrime(std::move(stmts));
    }
    
    // epsilon production
    return stmts;
}

std::unique_ptr<IfNode> parseIfStmt() {
    debugPrint("parseIfStmt");
    SourceLocation loc = captureLocation();
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
    
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBlock), std::move(elseBlock), loc);
}

std::unique_ptr<ASTnode> parseExprStmt() {
    debugPrint("parseExprStmt");
    SourceLocation loc = captureLocation();
    auto expr = parseExpr();
    if (!expr) return nullptr;
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<ExprStmtNode>(std::move(expr), loc);
}

std::unique_ptr<ReturnNode> parseReturnStmt() {
    debugPrint("parseReturnStmt");
    
    SourceLocation loc = captureLocation();

    getNextToken(); // consume RETURN
    
    if (CurTok.type == SC) {
        getNextToken(); // consume ';'
        return std::make_unique<ReturnNode>(nullptr, loc); // void return
    }
    
    auto expr = parseExpr();
    if (!expr) return nullptr;
    
    if (CurTok.type != SC) return nullptr;
    getNextToken();
    
    return std::make_unique<ReturnNode>(std::move(expr), loc);
}

std::unique_ptr<ASTnode> parseRelational() {
    debugPrint("parseRelational");
    auto left = parseAdditive();
    if (!left) return nullptr;
    return parseRelationalPrime(std::move(left));
}

std::unique_ptr<ASTnode> parseRelationalPrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseRelationalPrime");
    if (CurTok.type == LT || CurTok.type == GT || 
        CurTok.type == LE || CurTok.type == GE) {
        std::string op;
        SourceLocation loc = captureLocation();
        switch (CurTok.type) {
            case LT: op = "<"; break;
            case GT: op = ">"; break;
            case LE: op = "<="; break;
            case GE: op = ">="; break;
        }
        getNextToken();
        auto right = parseAdditive();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseRelationalPrime(std::move(newLeft));
    }
    return left;
}

std::unique_ptr<ASTnode> parseEquality() {
    debugPrint("parseEquality");
    auto left = parseRelational();
    if (!left) return nullptr;
    return parseEqualityPrime(std::move(left));
}

std::unique_ptr<ASTnode> parseEqualityPrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseEqualityPrime");
    if (CurTok.type == EQ || CurTok.type == NE) {
        SourceLocation loc = captureLocation();
        std::string op = CurTok.type == EQ ? "==" : "!=";
        getNextToken();
        auto right = parseRelational();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseEqualityPrime(std::move(newLeft));
    }
    return left;
}

std::unique_ptr<ASTnode> parseLogicAnd() {
    debugPrint("parseLogicAnd");
    auto left = parseEquality();
    if (!left) return nullptr;
    return parseLogicAndPrime(std::move(left));
}

std::unique_ptr<ASTnode> parseLogicAndPrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseLogicAndPrime");
    if (CurTok.type == AND) {
        SourceLocation loc = captureLocation();
        getNextToken();
        auto right = parseEquality();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>("&&", std::move(left), std::move(right), loc);
        return parseLogicAndPrime(std::move(newLeft));
    }
    return left;
}

// logic_or ::= logic_and logic_or'
std::unique_ptr<ASTnode> parseLogicOr() {
    debugPrint("parseLogicOr");
    auto left = parseLogicAnd();
    if (!left) return nullptr;
    return parseLogicOrPrime(std::move(left));
}

// logic_or' ::= "||" logic_and logic_or'
//             | epsilon
std::unique_ptr<ASTnode> parseLogicOrPrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseLogicOrPrime");
    if (CurTok.type == OR) {
        SourceLocation loc = captureLocation();
        getNextToken();
        auto right = parseLogicAnd();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>("||", std::move(left), std::move(right), loc);
        return parseLogicOrPrime(std::move(newLeft));
    }
    return left;
}

std::unique_ptr<ASTnode> parseExpr() {
    return parseAssignExpr();
}

std::unique_ptr<ASTnode> parseAssignExpr() {
    if (CurTok.type == IDENT) {
        SourceLocation loc = captureLocation();
        TOKEN savedTok = CurTok;
        getNextToken();
        if (CurTok.type == ASSIGN) {
            getNextToken();
            auto rhs = parseAssignExpr();
            if (!rhs) return nullptr;
            return std::make_unique<AssignNode>(savedTok.lexeme, std::move(rhs), loc);
        }
        putBackToken(CurTok);
        CurTok = savedTok;
    }
    return parseLogicOr();
}

std::unique_ptr<ASTnode> parseAdditive() {
    debugPrint("parseAdditive");
    auto left = parseMultiply();
    if (!left) return nullptr;
    return parseAdditivePrime(std::move(left));
}

std::unique_ptr<ASTnode> parseAdditivePrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseAdditivePrime");
    if (CurTok.type == PLUS || CurTok.type == MINUS) {
        std::string op = CurTok.type == PLUS ? "+" : "-";
        SourceLocation loc = captureLocation();
        getNextToken();
        auto right = parseMultiply();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseAdditivePrime(std::move(newLeft));
    }
    return left;
}

std::unique_ptr<ASTnode> parseMultiply() {
    debugPrint("parseMultiply");
    auto left = parseUnary();
    if (!left) return nullptr;
    return parseMultiplyPrime(std::move(left));
}

std::unique_ptr<ASTnode> parseMultiplyPrime(std::unique_ptr<ASTnode> left) {
    debugPrint("parseMultiplyPrime");
    if (CurTok.type == ASTERIX || CurTok.type == DIV || CurTok.type == MOD) {
        std::string op;
        switch (CurTok.type) {
            case ASTERIX: op = "*"; break;
            case DIV: op = "/"; break;
            case MOD: op = "%"; break;
        }
        SourceLocation loc = captureLocation();
        getNextToken();
        auto right = parseUnary();
        if (!right) return nullptr;
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseMultiplyPrime(std::move(newLeft));
    }
    return left;
}

std::unique_ptr<ASTnode> parsePrimary() {
    debugPrint("parsePrimary");
    switch (CurTok.type) {
        case IDENT: {
            std::string name = CurTok.lexeme;
            SourceLocation loc = captureLocation();

            getNextToken();
            if (CurTok.type == LPAR) {
                getNextToken();
                auto args = parseArgList();
                if (!args) return nullptr;
                if (CurTok.type != RPAR) {
                    std::cerr << "Expected ')' after function arguments\n";
                    return nullptr;
                }
                getNextToken();
                return std::make_unique<FunctionCallNode>(name, std::move(*args), loc);
            }
            return std::make_unique<VariableNode>(name, loc);
        }
        case INT_LIT: {
            int val = std::stoi(CurTok.lexeme);
            SourceLocation loc = captureLocation();
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
        }
        case FLOAT_LIT: {
            float val = std::stof(CurTok.lexeme);
            SourceLocation loc = captureLocation();
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
        }
        case BOOL_LIT: {
            bool val = (CurTok.lexeme == "true");
            SourceLocation loc = captureLocation();
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
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
        SourceLocation loc = captureLocation();
        getNextToken(); // Consume the unary operator
        
        auto operand = parseUnary(); // Recursive call to handle nested unary expressions
        if (!operand) return nullptr;
        
        return std::make_unique<UnaryOpNode>(op, std::move(operand), loc);
    }
    
    return parsePrimary(); // If no unary operator, parse as primary expression
}

std::unique_ptr<ASTnode> parseLocalDecl() {
    debugPrint("parseLocalDecl");
    std::string type = parseTypeSpec();
    if (type.empty()) return nullptr;
    
    if (CurTok.type != IDENT) return nullptr;
    SourceLocation loc = captureLocation();
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type != SC) return nullptr; //Expected ';'
    getNextToken();
    
    return std::make_unique<VarDeclNode>(type, name, loc);
}

// Helper functions
std::string parseTypeSpec() {
    debugPrint("parseTypeSpec");
    std::string type = CurTok.lexeme;  // Get the type name BEFORE checking
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        std::cerr << "Token type not in FIRST_type_spec: " << tokenTypeToString(CurTok.type) << "\n";
        return "";
    }
    
    getNextToken();  // Only consume the token AFTER we've verified and saved it
    return type;
}

std::unique_ptr<WhileNode> parseWhile() {
    debugPrint("parseWhile");
    SourceLocation loc = captureLocation();
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
    
    // This matches the grammar production "while_stmt ::= while ( expr ) stmt"
    auto body = parseStmt();  
    if (!body) return nullptr;
    
    return std::make_unique<WhileNode>(std::move(condition), std::move(body), loc);
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseParams() {
    debugPrint("parseParams");
    
    // Handle empty parameter list
    if (CurTok.type == RPAR) {
        return std::vector<std::pair<std::string, std::string>>();
    }
    
    // Handle void parameter
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return std::vector<std::pair<std::string, std::string>>();
    }
    
    return parseParamList();
}

std::optional<std::pair<std::string, std::string>> parseParam() {
    debugPrint("parseParam");
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        std::cerr << "Expected type specifier in parameter\n";
        return std::nullopt;
    }
    
    std::string type = parseTypeSpec();
    if (type.empty()) return std::nullopt;
    
    if (CurTok.type != IDENT) {
        std::cerr << "Expected identifier in parameter\n";
        return std::nullopt;
    }
    std::string name = CurTok.lexeme;
    getNextToken();
    
    return std::make_pair(type, name);
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseParamList() {
    debugPrint("parseParamList");
    std::vector<std::pair<std::string, std::string>> params;
    
    // param_list ::= param param_list'
    auto firstParam = parseParam();
    if (!firstParam) {
        std::cerr << "Error parsing first parameter\n";
        return std::nullopt;
    }
    
    params.push_back(std::move(*firstParam));
    return parseParamListPrime(std::move(params));
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseParamListPrime(
    std::vector<std::pair<std::string, std::string>>&& params) {
    debugPrint("parseParamListPrime");
    
    // param_list' ::= "," param param_list'
    //               | epsilon
    if (CurTok.type == COMMA) {
        getNextToken();
        auto nextParam = parseParam();
        if (!nextParam) {
            std::cerr << "Error parsing parameter after comma\n";
            return std::nullopt;
        }
        params.push_back(std::move(*nextParam));
        return parseParamListPrime(std::move(params));
    }
    
    // epsilon production
    return params;
}
std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgList() {
    debugPrint("parseArgList");

    // Handle epsilon production for args ::= epsilon
    if (CurTok.type == RPAR) {
        return std::vector<std::unique_ptr<ASTnode>>();
    }
    
    // Otherwise parse arg_list
    std::vector<std::unique_ptr<ASTnode>> args;
    
    auto firstArg = parseExpr();
    if (!firstArg) return std::nullopt;
    args.push_back(std::move(firstArg));
    
    return parseArgListPrime(std::move(args));
}

std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgListPrime(
    std::vector<std::unique_ptr<ASTnode>>&& args) { 
    if (CurTok.type == COMMA) {
        getNextToken();
        auto nextArg = parseExpr();
        if (!nextArg) return std::nullopt;
        args.push_back(std::move(nextArg));
        return parseArgListPrime(std::move(args));
    }
    return args;  // epsilon production
}

std::unique_ptr<ASTnode> parser() {
    debugPrint("parser");
    auto program = parseProgram();
    if (program) {
        std::cout << "Parsing Finished\n";
        std::cout << program->to_string();
        return program; // Return the AST
    } else {
        std::cerr << "Parsing failed\n";
        return nullptr;
    }
}