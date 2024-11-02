#include "ast.h"
#include "llvm/IR/IRBuilder.h"

// Type node
std::string TypeNode::to_string(int indent) const {
    return indentStr(indent) + "Type: " + typeName + "\n";
}

Value* TypeNode::codegen() {
    return nullptr;
}

// Program node
std::string ProgramNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "Program\n";
    for (const auto& ext : externs) {
        result += ext->to_string(indent + 3);
    }
    for (const auto& decl : declarations) {
        result += decl->to_string(indent + 3);
    }
    return result;
}

// External declaration node
std::string ExternNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "ExternDef: (" + type + ") " + name + "\n";
    for (const auto& param : params) {
        result += indentStr(indent + 3) + "Param: (" + param.first + ") " + param.second + "\n";
    }
    return result;
}

// Variable declaration node
std::string VarDeclNode::to_string(int indent) const {
    return indentStr(indent) + "VariableDeclaration: (" + type + ") " + name + "\n";
}

// Function declaration node
std::string FunctionNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "FunctionDef: (" + returnType + ") " + name + "\n";
    for (const auto& param : params) {
        result += indentStr(indent + 3) + "Param: (" + param.first + ") " + param.second + "\n";
    }
    if (body) {
        result += body->to_string(indent + 3);
    }
    return result;
}

// Block node
std::string BlockNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "BlockStmt:\n";
    for (const auto& decl : declarations) {
        result += decl->to_string(indent + 3);
    }
    for (const auto& stmt : statements) {
        result += stmt->to_string(indent + 3);
    }
    return result;
}

// If node
std::string IfNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "IfStmt:\n";
    result += indentStr(indent + 3) + "Condition:\n";
    result += condition->to_string(indent + 6);
    result += indentStr(indent + 3) + "ThenBlock:\n";
    result += thenBlock->to_string(indent + 6);
    if (elseBlock) {
        result += indentStr(indent + 3) + "ElseBlock:\n";
        result += elseBlock->to_string(indent + 6);
    }
    return result;
}

// While node
std::string WhileNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "WhileStmt:\n";
    result += indentStr(indent + 3) + "Condition:\n";
    result += condition->to_string(indent + 6);
    result += indentStr(indent + 3) + "Body:\n";
    result += body->to_string(indent + 6);
    return result;
}

// Return node
std::string ReturnNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "ReturnStmt:\n";
    if (value) {
        result += value->to_string(indent + 3);
    }
    return result;
}

// Expression statement node
std::string ExprStmtNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "ExprStmt:\n";
    result += expr->to_string(indent + 3);
    return result;
}

// Binary operation node
std::string BinaryOpNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "BinaryOperation: " + op + "\n";
    result += left->to_string(indent + 3);
    result += right->to_string(indent + 3);
    return result;
}

// Unary operation node
std::string UnaryOpNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "UnaryOperation: " + op + "\n";
    result += operand->to_string(indent + 3);
    return result;
}

// Assignment node
std::string AssignNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "VariableAssignment: " + name + "\n";
    result += value->to_string(indent + 3);
    return result;
}

// Variable node
std::string VariableNode::to_string(int indent) const {
    return indentStr(indent) + "VariableCall: " + name + "\n";
}

// Function call node
std::string FunctionCallNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "FunctionCall: " + name + "\n";
    for (const auto& arg : arguments) {
        result += arg->to_string(indent + 3);
    }
    return result;
}

// Literal node
std::string LiteralNode::to_string(int indent) const {
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
    
    return indentStr(indent) + typeStr + ": " + valueStr + "\n";
}
// ExternListNode
std::string ExternListNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "ExternList:\n";
    for (const auto& ext : externs) {
        result += ext->to_string(indent + 2);
    }
    return result;
}

Value* ExternListNode::codegen() {
    return nullptr;
}

// DeclListNode
std::string DeclListNode::to_string(int indent) const {
    std::string result = indentStr(indent) + "DeclList:\n";
    for (const auto& decl : declarations) {
        result += decl->to_string(indent + 2);
    }
    return result;
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