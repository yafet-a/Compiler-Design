#include "ast.h"
#include "llvm/IR/IRBuilder.h"

// Type node
std::string TypeNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "Type: " + typeName + "\n";
}

// Program node
std::string ProgramNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "Program\n";
    for (size_t i = 0; i < externs.size(); i++) {
        result += externs[i]->to_string(indent + 3, 
            i == externs.size() - 1 && declarations.empty());
    }
    for (size_t i = 0; i < declarations.size(); i++) {
        result += declarations[i]->to_string(indent + 3, 
            i == declarations.size() - 1);
    }
    return result;
}

// External declaration node
std::string ExternNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExternDef: (" + type + ") " + name + "\n";
    for (size_t i = 0; i < params.size(); i++) {
        result += getPrefix(indent + 3, i == params.size() - 1) + 
                 "Param: (" + params[i].first + ") " + params[i].second + "\n";
    }
    return result;
}

// Variable declaration node
std::string VarDeclNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "VariableDeclaration: (" + type + ") " + name + "\n";
}

// Function declaration node
std::string FunctionNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "FunctionDef: (" + returnType + ") " + name + "\n";
    for (size_t i = 0; i < params.size(); i++) {
        bool paramIsLast = (i == params.size() - 1) && !body;
        result += getPrefix(indent + 3, paramIsLast) + 
                 "Param: (" + params[i].first + ") " + params[i].second + "\n";
    }
    if (body) {
        result += body->to_string(indent + 3, true);
    }
    return result;
}

// Block node
std::string BlockNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "BlockStmt:\n";
    std::string childIndent = getChildIndent(indent, isLast);
    
    if (!declarations.empty() || !statements.empty()) {
        result += childIndent + "│\n";  // Add initial connecting line
    }
    
    for (size_t i = 0; i < declarations.size(); i++) {
        bool isDeclLast = (i == declarations.size() - 1 && statements.empty());
        result += declarations[i]->to_string(indent + 3, isDeclLast);
        if (!isDeclLast) {
            result += childIndent + "│\n";  // Add connecting line between items
        }
    }
    
    for (size_t i = 0; i < statements.size(); i++) {
        result += statements[i]->to_string(indent + 3, i == statements.size() - 1);
        if (i < statements.size() - 1) {
            result += childIndent + "│\n";  // Add connecting line between items
        }
    }
    return result;
}


// If node
std::string IfNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "IfStmt:\n";
    std::string childIndent = getChildIndent(indent, isLast);
    
    result += childIndent + "│\n";  // Add initial connecting line
    result += getPrefix(indent + 3, false) + "Condition:\n";
    result += condition->to_string(indent + 6, true);
    
    result += childIndent + "│\n";  // Add connecting line between condition and then
    result += getPrefix(indent + 3, !elseBlock) + "ThenBlock:\n";
    result += thenBlock->to_string(indent + 6, !elseBlock);
    
    if (elseBlock) {
        result += childIndent + "│\n";  // Add connecting line before else
        result += getPrefix(indent + 3, true) + "ElseBlock:\n";
        result += elseBlock->to_string(indent + 6, true);
    }
    return result;
}

// While node
std::string WhileNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "WhileStmt:\n";
    result += getPrefix(indent + 3, false) + "Condition:\n";
    result += condition->to_string(indent + 6, true);
    result += getPrefix(indent + 3, true) + "Body:\n";
    result += body->to_string(indent + 6, true);
    return result;
}

// Return node
std::string ReturnNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ReturnStmt:\n";
    if (value) {
        result += value->to_string(indent + 3, true);
    }
    return result;
}

// Expression statement node
std::string ExprStmtNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExprStmt:\n";
    if (expr) {
        result += expr->to_string(indent + 3, true);
    }
    return result;
}

// Binary operation node
std::string BinaryOpNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "BinaryOperation: " + op + "\n";
    std::string childIndent = getChildIndent(indent, isLast);
    // Add vertical line prefix for nested structure
    result += childIndent + "│\n";  // Add connecting line
    result += left->to_string(indent + 3, false);
    result += right->to_string(indent + 3, true);
    return result;
}


// Unary operation node
std::string UnaryOpNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "UnaryOperation: " + op + "\n";
    result += operand->to_string(indent + 3, true);
    return result;
}

// Assignment node
std::string AssignNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "VariableAssignment: " + name + "\n";
    std::string childIndent = getChildIndent(indent, isLast);
    result += childIndent + "│\n";  // Add connecting line
    result += value->to_string(indent + 3, true);
    return result;
}

// Variable node
std::string VariableNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "VariableCall: " + name + "\n";
}

// Function call node
std::string FunctionCallNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "FunctionCall: " + name + "\n";
    for (size_t i = 0; i < arguments.size(); i++) {
        result += arguments[i]->to_string(indent + 3, i == arguments.size() - 1);
    }
    return result;
}

// Literal node
std::string LiteralNode::to_string(int indent, bool isLast) const {
    std::string typeStr;
    std::string valueStr;
    
    switch (type) {
        case LiteralType::Int:
            typeStr = "IntLit";
            valueStr = std::to_string(value.intValue);
            break;
        case LiteralType::Float:
            typeStr = "FloatLit";
            valueStr = std::to_string(value.floatValue);
            break;
        case LiteralType::Bool:
            typeStr = "BoolLit";
            valueStr = value.boolValue ? "true" : "false";
            break;
    }
    
    return getPrefix(indent, isLast) + typeStr + ": " + valueStr + "\n";
}

// ExternListNode
std::string ExternListNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExternList:\n";
    for (size_t i = 0; i < externs.size(); i++) {
        result += externs[i]->to_string(indent + 3, i == externs.size() - 1);
    }
    return result;
}

// DeclListNode
std::string DeclListNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "DeclList:\n";
    for (size_t i = 0; i < declarations.size(); i++) {
        result += declarations[i]->to_string(indent + 3, i == declarations.size() - 1);
    }
    return result;
}

Value* ExternListNode::codegen() {
    return nullptr;
}

Value* DeclListNode::codegen() {
    return nullptr;
}


// ProgramNode
Value* ProgramNode::codegen() {
    return nullptr;
}

// ExternNode
Value* ExternNode::codegen() {
    return nullptr;
}

// VarDeclNode
Value* VarDeclNode::codegen() {
    return nullptr;
}

// FunctionNode
Value* FunctionNode::codegen() {
    return nullptr;
}

// BlockNode
Value* BlockNode::codegen() {
    return nullptr;
}

// IfNode
Value* IfNode::codegen() {
    return nullptr;
}

// WhileNode
Value* WhileNode::codegen() {
    return nullptr;
}

// ReturnNode
Value* ReturnNode::codegen() {
    return nullptr;
}

// ExprStmtNode
Value* ExprStmtNode::codegen() {
    return nullptr;
}

// BinaryOpNode
Value* BinaryOpNode::codegen() {
    return nullptr;
}

// UnaryOpNode
Value* UnaryOpNode::codegen() {
    return nullptr;
}

// AssignNode
Value* AssignNode::codegen() {
    return nullptr;
}

// VariableNode
Value* VariableNode::codegen() {
    return nullptr;
}

// FunctionCallNode
Value* FunctionCallNode::codegen() {
    return nullptr;
}

// LiteralNode
Value* LiteralNode::codegen() {
    return nullptr;
}