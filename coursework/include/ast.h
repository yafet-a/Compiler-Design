#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>
#include "tokens.h"
#include "llvm/IR/Value.h"

using namespace llvm;

//Source location information for error messages
struct SourceLocation {
    std::string filename;
    int line;
    int column;
    std::string lineContent;  // The actual line of code

    SourceLocation(const std::string& fname = "", int l = 0, int c = 0, 
                  const std::string& content = "")
        : filename(fname), line(l), column(c), lineContent(content) {}
};

// Base AST node class
class ASTnode {
protected:
    SourceLocation loc;
public:
    virtual ~ASTnode() {}
    virtual Value* codegen() = 0;
    virtual std::string to_string(int indent = 0, bool isLast = true) const {
        return std::string(indent, ' ') + "ASTnode\n";
    }

protected:
    std::string getPrefix(int indent, bool isLast) const {
        if (indent == 0) return "";
        std::string result;
        // Add vertical lines for all parent levels
        for (int i = 3; i < indent - 3; i += 3) {
            result += "│  ";
        }
        // Use different symbols for last vs non-last items
        result += isLast ? "└─ " : "├─ ";
        return result;
    }

    std::string getChildIndent(int indent, bool isLast) const {
        if (indent == 0) return "";
        std::string result;
        // Add vertical lines for all levels except the last
        for (int i = 3; i < indent; i += 3) {
            result += (i < indent - 3) ? "│  " : (isLast ? "   " : "│  ");
        }
        return result;
    }
};


// Type node
class TypeNode : public ASTnode {
    std::string typeName;
public:
    TypeNode(std::string typeName, const SourceLocation& location = SourceLocation())
        : typeName(typeName) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// Program node - represents the entire program
class ProgramNode : public ASTnode {
    std::vector<std::unique_ptr<ASTnode>> externs;
    std::vector<std::unique_ptr<ASTnode>> declarations;
public:
    ProgramNode(std::vector<std::unique_ptr<ASTnode>> exts, 
                std::vector<std::unique_ptr<ASTnode>> decls,
                const SourceLocation& location = SourceLocation())
        : externs(std::move(exts)), declarations(std::move(decls)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// External declaration node
class ExternNode : public ASTnode {
    std::string type;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
public:
    ExternNode(std::string type, std::string name, 
               std::vector<std::pair<std::string, std::string>> params,
               const SourceLocation& location = SourceLocation())
        : type(type), name(name), params(params) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// Variable declaration node
class VarDeclNode : public ASTnode {
    std::string type;
    std::string name;
public:
    VarDeclNode(std::string type, std::string name,
                const SourceLocation& location = SourceLocation())
        : type(type), name(name) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// Function declaration node
class FunctionNode : public ASTnode {
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::unique_ptr<ASTnode> body;
public:
    FunctionNode(std::string returnType, std::string name,
                 std::vector<std::pair<std::string, std::string>> params,
                 std::unique_ptr<ASTnode> body,
                 const SourceLocation& location = SourceLocation())
        : returnType(returnType), name(name), params(params), body(std::move(body)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// Block node
class BlockNode : public ASTnode {
    std::vector<std::unique_ptr<ASTnode>> declarations;
    std::vector<std::unique_ptr<ASTnode>> statements;
public:
    BlockNode(std::vector<std::unique_ptr<ASTnode>> decls,
              std::vector<std::unique_ptr<ASTnode>> stmts,
              const SourceLocation& location = SourceLocation())
        : declarations(std::move(decls)), statements(std::move(stmts)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// Statement nodes
class IfNode : public ASTnode {
    std::unique_ptr<ASTnode> condition;
    std::unique_ptr<ASTnode> thenBlock;
    std::unique_ptr<ASTnode> elseBlock;
public:
    IfNode(std::unique_ptr<ASTnode> cond,
           std::unique_ptr<ASTnode> thenB,
           std::unique_ptr<ASTnode> elseB = nullptr,
           const SourceLocation& location = SourceLocation())
        : condition(std::move(cond)), thenBlock(std::move(thenB)), 
          elseBlock(std::move(elseB)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};
class WhileNode : public ASTnode {
    std::unique_ptr<ASTnode> condition;
    std::unique_ptr<ASTnode> body;
public:
    WhileNode(std::unique_ptr<ASTnode> cond, 
              std::unique_ptr<ASTnode> body,
              const SourceLocation& location = SourceLocation())
        : condition(std::move(cond)), body(std::move(body)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// ExternList node
class ExternListNode : public ASTnode {
public:
    std::vector<std::unique_ptr<ASTnode>> externs;
    
    ExternListNode(std::vector<std::unique_ptr<ASTnode>> exts,
                   const SourceLocation& location = SourceLocation())
        : externs(std::move(exts)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

// DeclList node
class DeclListNode : public ASTnode {
public:
    std::vector<std::unique_ptr<ASTnode>> declarations;
    
    DeclListNode(std::vector<std::unique_ptr<ASTnode>> decls,
                 const SourceLocation& location = SourceLocation())
        : declarations(std::move(decls)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class ReturnNode : public ASTnode {
    std::unique_ptr<ASTnode> value;
public:
    ReturnNode(std::unique_ptr<ASTnode> val = nullptr,
               const SourceLocation& location = SourceLocation())
        : value(std::move(val)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class ExprStmtNode : public ASTnode {
    std::unique_ptr<ASTnode> expr;
public:
    ExprStmtNode(std::unique_ptr<ASTnode> expr,
                 const SourceLocation& location = SourceLocation())
        : expr(std::move(expr)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class BinaryOpNode : public ASTnode {
    std::string op;
    std::unique_ptr<ASTnode> left;
    std::unique_ptr<ASTnode> right;
public:
    BinaryOpNode(std::string op,
                 std::unique_ptr<ASTnode> left,
                 std::unique_ptr<ASTnode> right,
                 const SourceLocation& location = SourceLocation())
        : op(op), left(std::move(left)), right(std::move(right)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class UnaryOpNode : public ASTnode {
    std::string op;
    std::unique_ptr<ASTnode> operand;
public:
    UnaryOpNode(std::string op, 
                std::unique_ptr<ASTnode> operand,
                const SourceLocation& location = SourceLocation())
        : op(op), operand(std::move(operand)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class AssignNode : public ASTnode {
    std::string name;
    std::unique_ptr<ASTnode> value;
public:
    AssignNode(std::string name, 
               std::unique_ptr<ASTnode> value,
               const SourceLocation& location = SourceLocation())
        : name(name), value(std::move(value)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class VariableNode : public ASTnode {
    std::string name;
public:
    VariableNode(std::string name,
                 const SourceLocation& location = SourceLocation())
        : name(name) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};
class FunctionCallNode : public ASTnode {
    std::string name;
    std::vector<std::unique_ptr<ASTnode>> arguments;
public:
    FunctionCallNode(std::string name, 
                    std::vector<std::unique_ptr<ASTnode>> args,
                    const SourceLocation& location = SourceLocation())
        : name(name), arguments(std::move(args)) {
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

class LiteralNode : public ASTnode {
    enum class LiteralType { Int, Float, Bool } type;
    union {
        int intValue;
        float floatValue;
        bool boolValue;
    } value;
public:
    LiteralNode(int val, const SourceLocation& location = SourceLocation()) 
        : type(LiteralType::Int) { 
        value.intValue = val; 
        loc = location;
    }
    LiteralNode(float val, const SourceLocation& location = SourceLocation()) 
        : type(LiteralType::Float) { 
        value.floatValue = val; 
        loc = location;
    }
    LiteralNode(bool val, const SourceLocation& location = SourceLocation()) 
        : type(LiteralType::Bool) { 
        value.boolValue = val; 
        loc = location;
    }
    Value* codegen() override;
    std::string to_string(int indent = 0, bool isLast = true) const override;
};

#endif // AST_H