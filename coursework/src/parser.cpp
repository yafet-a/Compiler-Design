#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include "error_handler.h"
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

    CurTok = temp;
    return CurTok;
}
std::unordered_set<int> FIRST_program = {EXTERN, INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_decl = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_type_spec = {INT_TOK, FLOAT_TOK, BOOL_TOK, VOID_TOK};
std::unordered_set<int> FIRST_expr = {IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT, LPAR, NOT, MINUS};
std::unordered_set<int> FOLLOW_stmt_list = {RBRA};
std::unordered_set<int> FOLLOW_param_list = {RPAR};
std::unordered_set<int> FOLLOW_arg_list = {RPAR};
std::unordered_set<int> FOLLOW_local_decls = {IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT, IF, WHILE, RETURN, LBRA, RBRA, SC, NOT, MINUS, LPAR};

/**
 * @brief Looks ahead to the next token without consuming it.
 *
 * This lookahead function is used only once, as justified in the accompanying report.
 * This kept usage and dependence on lookahead to a need to use basis, making the rest of
 * the parser LL(1)
 *
 * @return The next TOKEN in the input stream.
 */
TOKEN peekNextToken() {
    if (tok_buffer.empty()) {
        tok_buffer.push_back(gettok());
    }
    return tok_buffer.front();
}


//program ::= extern_list decl_list 
//          | decl_list
std::unique_ptr<ProgramNode> parseProgram() {
    TOKEN loc = CurTok;
    if (!FIRST_program.count(CurTok.type)) {
        reportError("undefined reference to 'main'", CurTok);
    }

    std::vector<std::unique_ptr<ASTnode>> externs;
    std::vector<std::unique_ptr<ASTnode>> declarations;

    if (CurTok.type == EXTERN) {
        auto externList = parseExternList();

        externs = std::move(externList->externs);
    }

    auto declList = parseDeclList();
    declarations = std::move(declList->declarations);

    if (CurTok.type != EOF_TOK) {
        reportError("Expected end of file, got '" + CurTok.lexeme + "'", CurTok);
    }   
    return std::make_unique<ProgramNode>(std::move(externs), std::move(declarations), loc);
}

// extern_list ::= extern extern_list'
std::unique_ptr<ExternListNode> parseExternList() {
    TOKEN loc = CurTok;
    
    if (CurTok.type != EXTERN) {
        reportError("Expected 'extern' keyword at start of extern list", CurTok);
    }
    
    std::vector<std::unique_ptr<ASTnode>> externs;
    auto firstExtern = parseExtern();
    
    externs.push_back(std::move(firstExtern));
    
    return parseExternListPrime(std::move(externs), loc);
}

// extern_list' ::= extern extern_list'
//                | epsilon
std::unique_ptr<ExternListNode> parseExternListPrime(std::vector<std::unique_ptr<ASTnode>>&& externs, TOKEN loc) {
    
    if (CurTok.type == EXTERN) {
        auto nextExtern = parseExtern();
        externs.push_back(std::move(nextExtern));
        return parseExternListPrime(std::move(externs), loc);
    }
    
    // epsilon production
    return std::make_unique<ExternListNode>(std::move(externs), loc);
}

// decl_list ::= decl decl_list'
std::unique_ptr<DeclListNode> parseDeclList() {
    TOKEN loc = CurTok;  
    
    std::vector<std::unique_ptr<ASTnode>> declarations;

    auto firstDecl = parseDecl();
    declarations.push_back(std::move(firstDecl));
    
    return parseDeclListPrime(std::move(declarations), loc);
}

// decl_list' ::= decl decl_list'
//              | epsilon
std::unique_ptr<DeclListNode> parseDeclListPrime(std::vector<std::unique_ptr<ASTnode>> declarations, TOKEN loc) {
    
    if (FIRST_decl.count(CurTok.type)) {
        auto nextDecl = parseDecl();
        declarations.push_back(std::move(nextDecl));
        return parseDeclListPrime(std::move(declarations), loc);
    }
    
    // epsilon production
    return std::make_unique<DeclListNode>(std::move(declarations), loc);
}

//extern ::= "extern" type_spec IDENT "(" params ")" ";"
std::unique_ptr<ExternNode> parseExtern() {
    TOKEN loc = CurTok;
    
    if (CurTok.type != EXTERN) {
        reportError("Expected 'extern' keyword, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) return nullptr;
    
    if (CurTok.type != IDENT) {
        reportError("Expected identifier after return type in extern declaration, got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type != LPAR) {
        reportError("Expected '(' after function name in extern declaration, got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    auto params = parseParams();
    
    if (CurTok.type != RPAR) {
        reportError("Expected ')' after parameters in extern declaration, got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    if (CurTok.type != SC) {
        reportError("Expected ';' after extern declaration, got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    return std::make_unique<ExternNode>(returnType, name, std::move(*params), loc);
}


// var_decl ::= var_type IDENT ";"
std::unique_ptr<ASTnode> parseVarDecl(const std::string& type, const std::string& name, const TOKEN& loc) {
    if (CurTok.type != SC) {
        reportError("Expected ';' after variable declaration", CurTok);
    }
    getNextToken();
    return std::make_unique<VarDeclNode>(type, name, loc);
}

// fun_decl ::= type_spec IDENT "(" params ")" block
std::unique_ptr<ASTnode> parseFunDecl(const std::string& returnType, const std::string& name, const TOKEN& loc) {
    // Already past the '('
    getNextToken();
    auto params = parseParams();
    if (CurTok.type != RPAR) {
        reportError("Expected ')' after function parameters, got '" + 
                CurTok.lexeme + "'", CurTok);
    }
    getNextToken();

    if (CurTok.type == LBRA) {
        auto body = parseBlock();
        if (CurTok.type == LBRA) {
            reportError("Unexpected extra block after function definition", CurTok);
        }
        return std::make_unique<FunctionNode>(returnType, name, 
                                            std::move(*params), 
                                            std::move(body), 
                                            loc);
    } else if (CurTok.type == SC) {
        getNextToken();
        return std::make_unique<FunctionNode>(returnType, name, 
                                            std::move(*params), 
                                            nullptr, 
                                            loc);
    }
    reportError("Expected '{' or ';' after function declaration, got '" + 
               CurTok.lexeme + "'", CurTok);
}

// decl ::= var_decl
//        | fun_decl
std::unique_ptr<ASTnode> parseDecl() {
    std::string returnType = parseTypeSpec();
    if (returnType.empty()) {
        reportError("Expected type specifier at start of declaration", CurTok);
    }

    if (CurTok.type != IDENT) {
        reportError("Expected identifier after return type in declaration", CurTok);
    }
    
    TOKEN loc = CurTok;
    std::string name = CurTok.lexeme;
    getNextToken();

    if (CurTok.type == LPAR) {
        return parseFunDecl(returnType, name, loc);
    } else {
        return parseVarDecl(returnType, name, loc);
    }
}

// block ::= "{" local_decls stmt_list "}"
std::unique_ptr<BlockNode> parseBlock() {
    TOKEN loc = CurTok;
    if (CurTok.type != LBRA) {
        reportError("Expected '{' at start of block", CurTok), " instead got '" + CurTok.lexeme + "'";
    }
    getNextToken();
    
    // Parse local declarations
    auto declarations = parseLocalDecls();
    
    // Parse statements
    auto statements = parseStmtList();
    
    if (CurTok.type != RBRA) {
        reportError("Expected '}' at end of block", CurTok), " instead got '" + CurTok.lexeme + "'";
    }
    getNextToken();
    
    return std::make_unique<BlockNode>(std::move(declarations->declarations), std::move(statements), loc);
}

// local_decls ::= local_decls local_decl
//               | epsilon
std::unique_ptr<DeclListNode> parseLocalDecls() {
    
    if (FOLLOW_local_decls.count(CurTok.type)) {
        return std::make_unique<DeclListNode>(std::vector<std::unique_ptr<ASTnode>>());
    }
    
    std::vector<std::unique_ptr<ASTnode>> decls;
    auto firstDecl = parseLocalDecl();
    decls.push_back(std::move(firstDecl));
    
    return parseLocalDeclsPrime(std::move(decls));
}

// local_decls' ::= local_decl local_decls' | epsilon
std::unique_ptr<DeclListNode> parseLocalDeclsPrime(std::vector<std::unique_ptr<ASTnode>>&& decls) {
    
    //Check for epsilon production
    if (FOLLOW_local_decls.count(CurTok.type)) {
        return std::make_unique<DeclListNode>(std::move(decls));
    }
    
    auto nextDecl = parseLocalDecl();
    decls.push_back(std::move(nextDecl));
    return parseLocalDeclsPrime(std::move(decls));
}

// stmt ::= expr_stmt
//        | block
//        | if_stmt
//        | while_stmt
//        | return_stmt
std::unique_ptr<ASTnode> parseStmt() {
    switch (CurTok.type) {
        case IF:
            return parseIfStmt();
        case WHILE:
            return parseWhile();
        case RETURN:
            return parseReturnStmt();
        case LBRA:
            return parseBlock();
        case SC:
            getNextToken();
            return std::make_unique<ExprStmtNode>(nullptr);// made to handle empty statements
        default:
            if (FIRST_expr.count(CurTok.type))
                return parseExprStmt();
                reportError("Unexpected token " + CurTok.lexeme + " in statement", CurTok);
    }
}

// stmt_list ::= stmt_list stmt
//             | epsilon
std::vector<std::unique_ptr<ASTnode>> parseStmtList() {
    
    //Check for epsilon production
    if (FOLLOW_stmt_list.count(CurTok.type)) {
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
    
    if (CurTok.type != RBRA) {
        auto nextStmt = parseStmt();
        if (!nextStmt) return std::vector<std::unique_ptr<ASTnode>>();
        stmts.push_back(std::move(nextStmt));
        return parseStmtListPrime(std::move(stmts));
    }

    return stmts;
}

// if_stmt ::= "if" "(" expr ")" block else_stmt
std::unique_ptr<IfNode> parseIfStmt() {
    TOKEN loc = CurTok;
    getNextToken();
    
    if (CurTok.type != LPAR){
        reportError("Expected '(' after 'if' keyword", CurTok);
    }
    getNextToken();
    
    auto condition = parseExpr();
    
    if (CurTok.type != RPAR) {
        reportError("Expected ')' after if condition, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    auto thenBlock = parseBlock();
    
    std::unique_ptr<ASTnode> elseBlock = nullptr;
    if (CurTok.type == ELSE) {
        getNextToken();
        elseBlock = parseBlock();
    }
    
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBlock), std::move(elseBlock), loc);
}

// expr_stmt ::= expr ";"
//             | ";"
std::unique_ptr<ASTnode> parseExprStmt() {
    TOKEN loc = CurTok;
    auto expr = parseExpr();
    
    if (CurTok.type != SC) {
        reportError("Expected ';' after expression statement, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    return std::make_unique<ExprStmtNode>(std::move(expr), loc);
}

// return_stmt ::= "return" ";"
//               | "return" expr ";"
std::unique_ptr<ReturnNode> parseReturnStmt() {
    
    TOKEN loc = CurTok;

    getNextToken();
    
    if (CurTok.type == SC) {
        getNextToken();
        return std::make_unique<ReturnNode>(nullptr, loc); // In the case of a void return
    }
    
    auto expr = parseExpr();
    
    if (CurTok.type != SC) {
        reportError("Expected semicolon after return statement, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    return std::make_unique<ReturnNode>(std::move(expr), loc);
}

// relation ::= additive relation'
std::unique_ptr<ASTnode> parseRelational() {
    auto left = parseAdditive();
    return parseRelationalPrime(std::move(left));
}

// relation' ::= ("<=" | "<" | ">=" | ">") additive relation'
//             | epsilon
std::unique_ptr<ASTnode> parseRelationalPrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == LT || CurTok.type == GT || 
        CurTok.type == LE || CurTok.type == GE) {
        std::string op;
        TOKEN loc = CurTok;;
        switch (CurTok.type) {
            case LT: op = "<"; break;
            case GT: op = ">"; break;
            case LE: op = "<="; break;
            case GE: op = ">="; break;
        }
        getNextToken();
        auto right = parseAdditive();
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseRelationalPrime(std::move(newLeft));
    }
    return left;
}

// equality ::= relation equality'
std::unique_ptr<ASTnode> parseEquality() {
    auto left = parseRelational();
    return parseEqualityPrime(std::move(left));
}

// equality' ::= ("==" | "!=") relation equality'
//             | epsilon
std::unique_ptr<ASTnode> parseEqualityPrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == EQ || CurTok.type == NE) {
        TOKEN loc = CurTok;;
        std::string op = CurTok.type == EQ ? "==" : "!=";
        getNextToken();
        auto right = parseRelational();
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseEqualityPrime(std::move(newLeft));
    }
    return left;
}

// logic_and ::= equality logic_and'
std::unique_ptr<ASTnode> parseLogicAnd() {
    auto left = parseEquality();
    return parseLogicAndPrime(std::move(left));
}

// logic_and' ::= "&&" equality logic_and'
//              | epsilon
std::unique_ptr<ASTnode> parseLogicAndPrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == AND) {
        TOKEN loc = CurTok;;
        getNextToken();
        auto right = parseEquality();
        auto newLeft = std::make_unique<BinaryOpNode>("&&", std::move(left), std::move(right), loc);
        return parseLogicAndPrime(std::move(newLeft));
    }
    return left;
}

// logic_or ::= logic_and logic_or'
std::unique_ptr<ASTnode> parseLogicOr() {
    auto left = parseLogicAnd();
    return parseLogicOrPrime(std::move(left));
}

// logic_or' ::= "||" logic_and logic_or'
//             | epsilon
std::unique_ptr<ASTnode> parseLogicOrPrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == OR) {
        TOKEN loc = CurTok;;
        getNextToken();
        auto right = parseLogicAnd();
        auto newLeft = std::make_unique<BinaryOpNode>("||", std::move(left), std::move(right), loc);
        return parseLogicOrPrime(std::move(newLeft));
    }
    return left;
}

// expr ::= assign_expr
std::unique_ptr<ASTnode> parseExpr() {
    return parseAssignExpr();
}

// assign_expr ::= IDENT "=" assign_expr  
//               | logic_or
std::unique_ptr<ASTnode> parseAssignExpr() {
    if (CurTok.type == IDENT) {
        TOKEN loc = CurTok;
        TOKEN idTok = CurTok;
        TOKEN nextTok = peekNextToken();
        
        if (nextTok.type == ASSIGN) {
            getNextToken();
            getNextToken();
            auto rhs = parseAssignExpr();
            return std::make_unique<AssignNode>(idTok.lexeme, std::move(rhs), loc);
        }
    }
    return parseLogicOr();
}

// additive ::= multiply additive'
std::unique_ptr<ASTnode> parseAdditive() {
    auto left = parseMultiply();

    return parseAdditivePrime(std::move(left));
}

// additive' ::= ("+" | "-") multiply additive'
//             | epsilon
std::unique_ptr<ASTnode> parseAdditivePrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == PLUS || CurTok.type == MINUS) {
        std::string op = CurTok.type == PLUS ? "+" : "-";
        TOKEN loc = CurTok;;
        getNextToken();
        auto right = parseMultiply();
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseAdditivePrime(std::move(newLeft));
    }
    return left;
}

// multiply ::= unary multiply'
std::unique_ptr<ASTnode> parseMultiply() {
    auto left = parseUnary();
    return parseMultiplyPrime(std::move(left));
}

// multiply' ::= ("*" | "/" | "%") unary multiply'
//             | epsilon
std::unique_ptr<ASTnode> parseMultiplyPrime(std::unique_ptr<ASTnode> left) {
    if (CurTok.type == ASTERIX || CurTok.type == DIV || CurTok.type == MOD) {
        std::string op;
        switch (CurTok.type) {
            case ASTERIX: op = "*"; break;
            case DIV: op = "/"; break;
            case MOD: op = "%"; break;
        }
        TOKEN loc = CurTok;;
        getNextToken();
        auto right = parseUnary();
        auto newLeft = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
        return parseMultiplyPrime(std::move(newLeft));
    }
    return left;
}

// primary ::= "(" expr ")"
//           | IDENT
//           | IDENT "(" args ")"
//           | INT_LIT
//           | FLOAT_LIT
//           | BOOL_LIT
std::unique_ptr<ASTnode> parsePrimary() {
    switch (CurTok.type) {
        case IDENT: {
            std::string name = CurTok.lexeme;
            TOKEN loc = CurTok;;

            getNextToken();
            if (CurTok.type == LPAR) {
                getNextToken();
                auto args = parseArgList();
                if (CurTok.type != RPAR) {
                    reportError("Expected ')' after function arguments", CurTok);
                }
                getNextToken();
                return std::make_unique<FunctionCallNode>(name, std::move(*args), loc);
            }
            return std::make_unique<VariableNode>(name, loc);
        }
        case INT_LIT: {
            int val = std::stoi(CurTok.lexeme);
            TOKEN loc = CurTok;;
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
        }
        case FLOAT_LIT: {
            float val = std::stof(CurTok.lexeme);
            TOKEN loc = CurTok;;
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
        }
        case BOOL_LIT: {
            bool val = (CurTok.lexeme == "true");
            TOKEN loc = CurTok;;
            getNextToken();
            return std::make_unique<LiteralNode>(val, loc);
        }
        case LPAR: {
            getNextToken();
            auto expr = parseExpr();

            if (CurTok.type != RPAR) {
                reportError("Expected ')' after expression in primary expression", CurTok);
            }
            getNextToken();
            return expr;
        }
        default:
            reportError("Unexpected token " + CurTok.lexeme + " in primary expression", CurTok);
    }
}

// unary ::= ("-" | "!") unary
//         | primary
std::unique_ptr<ASTnode> parseUnary() {
    if (CurTok.type == MINUS || CurTok.type == NOT) {
        std::string op = (CurTok.type == MINUS) ? "-" : "!";
        TOKEN loc = CurTok;;
        getNextToken();
        
        auto operand = parseUnary();     
        return std::make_unique<UnaryOpNode>(op, std::move(operand), loc);
    }
    
    return parsePrimary();
}

// local_decl ::= var_type IDENT ";"
std::unique_ptr<ASTnode> parseLocalDecl() {
    std::string type = parseTypeSpec();
    
    if (CurTok.type != IDENT) {
        reportError("Expected identifier after type '" + type + "', got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    TOKEN loc = CurTok;
    std::string name = CurTok.lexeme;
    getNextToken();
    
    if (CurTok.type != SC) {
        reportError("Expected semicolon after variable declaration, got '" + 
                   CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    return std::make_unique<VarDeclNode>(type, name, loc);
}

// type_spec ::= "void"
//             | var_type
std::string parseTypeSpec() {
    std::string type = CurTok.lexeme;
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        reportError("Expected type specifier (int, float, bool, void), got '" + 
                   CurTok.lexeme + "'", CurTok);
       
        return "";
    }
    
    getNextToken();
    return type;
}

// while_stmt ::= "while" "(" expr ")" stmt
std::unique_ptr<WhileNode> parseWhile() {
    TOKEN loc = CurTok;
    getNextToken();
    
    if (CurTok.type != LPAR) {
        reportError("Expected '(' after while, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    auto condition = parseExpr();
    
    if (CurTok.type != RPAR) {
        reportError("Expected ')' after while condition, got '" + CurTok.lexeme + "'", CurTok);
    }
    getNextToken();
    
    auto body = parseStmt();
    
    return std::make_unique<WhileNode>(std::move(condition), std::move(body), loc);
}

// params ::= param_list
//          | "void"
//          | epsilon
std::optional<std::vector<std::pair<std::string, std::string>>> parseParams() {
    
    if (FOLLOW_param_list.count(CurTok.type)) {
        return std::vector<std::pair<std::string, std::string>>();
    }
    
    if (CurTok.type == VOID_TOK) {
        getNextToken();
        return std::vector<std::pair<std::string, std::string>>();
    }
    
    return parseParamList();
}

// param ::= var_type IDENT
std::optional<std::pair<std::string, std::string>> parseParam() {
    
    if (!FIRST_type_spec.count(CurTok.type)) {
        reportError("Expected type specifier in parameter, got '" + CurTok.lexeme + "'", CurTok);
    }
    
    std::string type = parseTypeSpec();
    if (type.empty()) return std::nullopt;
    
    if (CurTok.type != IDENT) {
        reportError("Expected identifier in parameter, got '" + CurTok.lexeme + "'", CurTok);
    }
    std::string name = CurTok.lexeme;
    getNextToken();
    
    return std::make_pair(type, name);
}

// param_list ::= param param_list'
std::optional<std::vector<std::pair<std::string, std::string>>> parseParamList() {
    std::vector<std::pair<std::string, std::string>> params;
    
    auto firstParam = parseParam();

    params.push_back(std::move(*firstParam));
    return parseParamListPrime(std::move(params));
}

// param_list' ::= "," param param_list'
//               | epsilon
std::optional<std::vector<std::pair<std::string, std::string>>> parseParamListPrime(
    std::vector<std::pair<std::string, std::string>>&& params) {
    
    if (CurTok.type == COMMA) {
        getNextToken();
        auto nextParam = parseParam();
        params.push_back(std::move(*nextParam));
        return parseParamListPrime(std::move(params));
    }
    
    // epsilon production
    return params;
}
std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgList() {

    if (FOLLOW_arg_list.count(CurTok.type)) {
        return std::vector<std::unique_ptr<ASTnode>>();
    }
    
    std::vector<std::unique_ptr<ASTnode>> args;
    auto firstArg = parseExpr();
    args.push_back(std::move(firstArg));
    
    return parseArgListPrime(std::move(args));
}

std::optional<std::vector<std::unique_ptr<ASTnode>>> parseArgListPrime(
    std::vector<std::unique_ptr<ASTnode>>&& args) { 
    if (CurTok.type == COMMA) {
        getNextToken();
        auto nextArg = parseExpr();
        args.push_back(std::move(nextArg));
        return parseArgListPrime(std::move(args));
    }
    return args;
}

std::unique_ptr<ASTnode> parser() {
    auto program = parseProgram();
    if (program) {
        std::cout << program->to_string();
        return program;
    } else {
        reportError("Parsing failed", CurTok);
    }
}