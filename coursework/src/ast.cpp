#include "ast.h"
#include "llvm_context.h"
#include "error_handler.h"
#include <iostream>

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

// ProgramNode
Value* ProgramNode::codegen() {
    std::cerr << "Generating code for ProgramNode\n";
    // Generate code for externs
    for (const auto& ext : externs) {
        if (!ext->codegen()) {
            std::cerr << "Error: Failed to generate code for extern\n";
            return nullptr;
        }
    }
    
    // Generate code for declarations
    for (const auto& decl : declarations) {
        if (!decl->codegen()) {
            std::cerr << "Error: Failed to generate code for declarationp\n";
            return nullptr;
        }
    }
    
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)); // Dummy value
}

// ExternNode
Value* ExternNode::codegen() {
    std::cerr << "Generating code for ExternNode: " << name << "\n";
    std::vector<Type*> ArgTypes;
    for (const auto& param : params) {
        Type* paramType = getTypeFromStr(param.first);
        if (!paramType) {
            std::cerr << "Error: Unknown type for parameter: " << param.first << "\n";
            return nullptr;
        }
        ArgTypes.push_back(paramType);
    }
    
    Type* RetType = getTypeFromStr(type);
    if (!RetType) {
        std::cerr << "Error: Unknown return type: " << type << "\n";
        return nullptr;
    }
    
    FunctionType *FT = FunctionType::get(RetType, ArgTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
    
    // Set parameter names
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(params[Idx++].second);
    }
    
    return F;
}


// VarDeclNode
Value* VarDeclNode::codegen() {
    std::cerr << "Generating code for VarDeclNode: " << name << "\n";
    llvm::Type* varType = getTypeFromStr(type);
    if (!varType) {
        std::cerr << "Error: Unknown type for variable: " << type << "\n";
        return nullptr;
    }

    // Check if we're in a function context
    llvm::Function* TheFunction = Builder.GetInsertBlock() ? Builder.GetInsertBlock()->getParent() : nullptr;

    if (TheFunction) {
        // Local variable
        // Check if variable already exists in current scope
        if (NamedValues.find(name) != NamedValues.end()) {
            // Allow shadowing of outer scope variables
            llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, name, varType);
            NamedValues[name] = { Alloca, varType, false };
            return Alloca;
        } else {
            llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, name, varType);
            NamedValues[name] = { Alloca, varType, false };
            return Alloca;
        }
    } else {
        // Global variable
        // Check if global variable already exists
        if (TheModule->getGlobalVariable(name)) {
            reportError("Redefinition of global variable '" + name + "'", loc);
        }

        // Create global variable
        llvm::GlobalVariable* GlobalVar = new llvm::GlobalVariable(
            *TheModule,
            varType,
            false, // isConstant
            llvm::GlobalValue::ExternalLinkage,
            llvm::Constant::getNullValue(varType), // initializer
            name
        );

        GlobalNamedValues[name] = { GlobalVar, varType, true };
        return GlobalVar;
    }
}


// FunctionNode 
Value* FunctionNode::codegen() {
    std::cerr << "Generating code for FunctionNode: " << name << "\n";
    
    // Create function type
    std::vector<Type*> ArgTypes;
    for (const auto& param : params) {
        Type* paramType = getTypeFromStr(param.first);
        if (!paramType) {
            std::cerr << "Error: Unknown type for parameter: " << param.first << "\n";
            return nullptr;
        }
        ArgTypes.push_back(paramType);
    }
    
    Type* RetType = getTypeFromStr(returnType);
    if (!RetType) {
        std::cerr << "Error: Unknown return type: " << returnType << "\n";
        return nullptr;
    }
    
    FunctionType *FT = FunctionType::get(RetType, ArgTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
    
    // Create basic block
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    
    // Save the current scope
    std::map<std::string, VariableInfo> OldNamedValues = NamedValues;
    NamedValues.clear();
    
    // Set up parameters
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(params[Idx].second);
        llvm::Type* paramType = ArgTypes[Idx];
        AllocaInst *Alloca = CreateEntryBlockAlloca(F, params[Idx].second, paramType);
        Builder.CreateStore(&Arg, Alloca);
        // Store VariableInfo in NamedValues
        NamedValues[params[Idx].second] = { Alloca, paramType };
        Idx++;
    }
    
    if (body) {
        if (Value *RetVal = body->codegen()) {
            // If body doesn't end with a terminator, add one
            if (!Builder.GetInsertBlock()->getTerminator()) {
                if (RetType->isVoidTy()) {
                    Builder.CreateRetVoid();
                } else {
                    Builder.CreateRet(Constant::getNullValue(RetType));
                }
            }
            
            // Verify the function
            verifyFunction(*F);
            // Restore the previous scope
            NamedValues = OldNamedValues;
            return F;
        } else {
            std::cerr << "Error: Failed to generate code for function body\n";
        }
    } else {
        std::cerr << "Error: Function body is missing\n";
    }
    
    // Remove the function if codegen failed
    F->eraseFromParent();
    // Restore the previous scope
    NamedValues = OldNamedValues;
    return nullptr;
}

// BlockNode
Value* BlockNode::codegen() {
    std::cerr << "Generating code for BlockNode\n";
    Value* Last = nullptr;
    
    // Save the current scope
    std::map<std::string, VariableInfo> OldNamedValues = NamedValues;
    
    // Generate code for declarations in new scope
    for (const auto& decl : declarations) {
        Last = decl->codegen();
        if (!Last) {
            std::cerr << "Error: Failed to generate code for declaration\n";
            NamedValues = OldNamedValues;  // Restore scope before error return
            return nullptr;
        }
    }
    
    // Generate code for statements
    for (const auto& stmt : statements) {
        Last = stmt->codegen();
        if (!Last) {
            std::cerr << "Error: Failed to generate code for statement\n";
            NamedValues = OldNamedValues;  // Restore scope before error return
            return nullptr;
        }
        
        // If block is terminated (e.g., by a return), stop generating more code
        if (Builder.GetInsertBlock()->getTerminator())
            break;
    }
    
    // Restore the previous scope
    NamedValues = OldNamedValues;
    
    return Last;
}

// IfNode modification
Value* IfNode::codegen() {
    std::cerr << "Generating code for IfNode\n";
    Value *CondV = condition->codegen();
    if (!CondV) {
        std::cerr << "Error: Failed to generate code for condition\n";
        return nullptr;
    }

    // Convert condition to bool using convertToType with conditional context
    if (!CondV->getType()->isIntegerTy(1)) {
        CondV = convertToType(CondV, llvm::Type::getInt1Ty(TheContext), true);
        if (!CondV) {
            std::cerr << "Error: Failed to convert condition to bool\n";
            return nullptr;
        }
    }

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    
    // Create blocks
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
    
    // Create else block only if we have an else statement
    BasicBlock *ElseBB = nullptr;
    if (elseBlock) {
        ElseBB = BasicBlock::Create(TheContext, "else");
    }

    // Insert MergeBB and ElseBB (if it exists) before creating branches
    if (elseBlock) {
        ElseBB->insertInto(TheFunction);
    }
    MergeBB->insertInto(TheFunction);
    
    // Create conditional branch
    Builder.CreateCondBr(CondV, ThenBB, ElseBB ? ElseBB : MergeBB);
    
    // Emit then block
    Builder.SetInsertPoint(ThenBB);
    Value *ThenV = thenBlock->codegen();
    if (!ThenV) {
        std::cerr << "Error: Failed to generate code for then block\n";
        return nullptr;
    }
    // Add branch to merge block if it doesn't already have a terminator
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(MergeBB);
    }
    
    // Emit else block if it exists
    if (elseBlock) {
        Builder.SetInsertPoint(ElseBB);
        Value *ElseV = elseBlock->codegen();
        if (!ElseV) {
            std::cerr << "Error: Failed to generate code for else block\n";
            return nullptr;
        }
        // Add branch to merge block if it doesn't already have a terminator
        if (!Builder.GetInsertBlock()->getTerminator()) {
            Builder.CreateBr(MergeBB);
        }
    }
    
    // Set insert point to merge block
    Builder.SetInsertPoint(MergeBB);
    
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
}

// WhileNode modification
Value* WhileNode::codegen() {
    std::cerr << "Generating code for WhileNode\n";
    
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create basic blocks for the loop
    BasicBlock *HeaderBB = BasicBlock::Create(TheContext, "while.header", TheFunction);
    BasicBlock *BodyBB = BasicBlock::Create(TheContext, "while.body");
    BasicBlock *ExitBB = BasicBlock::Create(TheContext, "while.exit");

    // Branch from the current block to the header
    Builder.CreateBr(HeaderBB);

    // Emit the header block
    Builder.SetInsertPoint(HeaderBB);
    Value *CondV = condition->codegen();
    if (!CondV) {
        std::cerr << "Error: Failed to generate code for while condition\n";
        return nullptr;
    }

    // Convert condition to bool if necessary (for C-style conditions)
    if (!CondV->getType()->isIntegerTy(1)) {
        if (CondV->getType()->isIntegerTy()) {
            // Compare with 0 to convert to bool
            CondV = Builder.CreateICmpNE(
                CondV, 
                ConstantInt::get(CondV->getType(), 0),
                "while.cond"
            );
        } else {
            std::cerr << "Error: While condition must be convertible to bool\n";
            return nullptr;
        }
    }

    // Add both BodyBB and ExitBB to the function before creating the conditional branch
    BodyBB->insertInto(TheFunction);
    ExitBB->insertInto(TheFunction);

    // Create conditional branch
    Builder.CreateCondBr(CondV, BodyBB, ExitBB);

    // Emit the body block
    Builder.SetInsertPoint(BodyBB);
    if (!body->codegen()) {
        std::cerr << "Error: Failed to generate code for while body\n";
        return nullptr;
    }

    // Create branch back to header block if the block isn't already terminated
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(HeaderBB);
    }

    // Move to exit block
    Builder.SetInsertPoint(ExitBB);

    return Constant::getNullValue(Type::getInt32Ty(TheContext));
}

// ReturnNode
Value* ReturnNode::codegen() {
    Value* RetVal = nullptr;
    if (value) {
        RetVal = value->codegen();
        if (!RetVal) return nullptr;

        // Get function's return type
        Function* TheFunction = Builder.GetInsertBlock()->getParent();
        Type* RetType = TheFunction->getReturnType();

        // If the return value type does not match the function's return type, convert it
        if (RetVal && RetVal->getType() != RetType) {
            RetVal = convertToType(RetVal, RetType);
            if (!RetVal) {
                std::cerr << "Error: Invalid type conversion in return\n";
                return nullptr;
            }
        }
    }
    
    return Builder.CreateRet(RetVal);
}

// ExprStmtNode
Value* ExprStmtNode::codegen() {
    std::cerr << "Generating code for ExprStmtNode\n";
    Value* Val = expr->codegen();
    if (!Val) {
        std::cerr << "Error: Failed to generate code for expression in ExprStmtNode\n";
        return nullptr;
    }
    // The value is not used further, as it's an expression statement.
    return Val;
}

// BinaryOpNode
// BinaryOpNode
Value* BinaryOpNode::codegen() {
    std::cerr << "Generating code for BinaryOpNode: " << op << "\n";

    // Special handling for logical operators
    if (op == "&&" || op == "||") {
        Function* TheFunction = Builder.GetInsertBlock()->getParent();
        
        // Generate code for left operand
        Value* L = left->codegen();
        if (!L) return nullptr;

        // Convert to bool if needed
        if (!L->getType()->isIntegerTy(1)) {
            L = convertToType(L, Type::getInt1Ty(TheContext), true, loc.line, loc.column);
            if (!L) return nullptr;
        }

        // Create blocks but don't insert yet
        BasicBlock* RHSBlock = BasicBlock::Create(TheContext, "rhs");
        BasicBlock* MergeBlock = BasicBlock::Create(TheContext, "merge");

        // Store the entry block for PHI
        BasicBlock* EntryBlock = Builder.GetInsertBlock();

        // Add the blocks to the function
        TheFunction->insert(TheFunction->end(), RHSBlock);
        TheFunction->insert(TheFunction->end(), MergeBlock);

        // Create conditional branch based on operator
        if (op == "&&") {
            Builder.CreateCondBr(L, RHSBlock, MergeBlock);
        } else { // op == "||"
            Builder.CreateCondBr(L, MergeBlock, RHSBlock);
        }

        // Emit RHS block
        Builder.SetInsertPoint(RHSBlock);
        Value* R = right->codegen();
        if (!R) return nullptr;

        if (!R->getType()->isIntegerTy(1)) {
            R = convertToType(R, Type::getInt1Ty(TheContext), true, loc.line, loc.column);
            if (!R) return nullptr;
        }

        // Store the RHS end block for PHI
        BasicBlock* RHSEndBlock = Builder.GetInsertBlock();
        Builder.CreateBr(MergeBlock);

        // Emit merge block
        Builder.SetInsertPoint(MergeBlock);
        PHINode* PN = Builder.CreatePHI(Type::getInt1Ty(TheContext), 2, "logical.result");

        // Set up PHI node values
        if (op == "&&") {
            PN->addIncoming(ConstantInt::getFalse(TheContext), EntryBlock);
            PN->addIncoming(R, RHSEndBlock);
        } else { // op == "||"
            PN->addIncoming(ConstantInt::getTrue(TheContext), EntryBlock);
            PN->addIncoming(R, RHSEndBlock);
        }

        return PN;
    }

    // Generate code for both operands for non-logical operators
    Value* L = left->codegen();
    Value* R = right->codegen();
    if (!L || !R) {
        reportError("Invalid operands to binary expression", loc);
        return nullptr;
    }

    // For arithmetic operations
    // If either operand is float, convert both to float
    if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
        if (!L->getType()->isFloatTy()) {
            L = convertToType(L, Type::getFloatTy(TheContext));
        }
        if (!R->getType()->isFloatTy()) {
            R = convertToType(R, Type::getFloatTy(TheContext));
        }
        if (!L || !R) return nullptr;
    }
    // Otherwise if either is int32, convert both to int32
    else if (L->getType()->isIntegerTy(32) || R->getType()->isIntegerTy(32)) {
        if (!L->getType()->isIntegerTy(32)) {
            L = convertToType(L, Type::getInt32Ty(TheContext));
        }
        if (!R->getType()->isIntegerTy(32)) {
            R = convertToType(R, Type::getInt32Ty(TheContext));
        }
        if (!L || !R) return nullptr;
    }

    // Create the result based on operator
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (L->getType()->isFloatTy()) {
            if (op == "%") {
                std::cerr << "Error: Modulo not supported for floating point\n";
                return nullptr;
            }
            if (op == "+") return Builder.CreateFAdd(L, R, "addtmp");
            if (op == "-") return Builder.CreateFSub(L, R, "subtmp");
            if (op == "*") return Builder.CreateFMul(L, R, "multmp");
            if (op == "/") return Builder.CreateFDiv(L, R, "divtmp");
        } else {
            if (op == "+") return Builder.CreateAdd(L, R, "addtmp");
            if (op == "-") return Builder.CreateSub(L, R, "subtmp");
            if (op == "*") return Builder.CreateMul(L, R, "multmp");
            if (op == "/") return Builder.CreateSDiv(L, R, "divtmp");
            if (op == "%") return Builder.CreateSRem(L, R, "modtmp");
        }
    }

    // Handle comparisons (==, !=, <, >, <=, >=)
    if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        // For comparisons between booleans, leave them as bools
        if (L->getType()->isIntegerTy(1) && R->getType()->isIntegerTy(1)) {
            if (op == "==") return Builder.CreateICmpEQ(L, R, "eqtmp");
            if (op == "!=") return Builder.CreateICmpNE(L, R, "neqtmp");
        }

        // For other comparisons, do the usual type conversion
        bool isFloating = L->getType()->isFloatTy() || R->getType()->isFloatTy();

        if (isFloating) {
            if (op == "<")  return Builder.CreateFCmpOLT(L, R, "cmptmp");
            if (op == "<=") return Builder.CreateFCmpOLE(L, R, "cmptmp");
            if (op == ">")  return Builder.CreateFCmpOGT(L, R, "cmptmp");
            if (op == ">=") return Builder.CreateFCmpOGE(L, R, "cmptmp");
            if (op == "==") return Builder.CreateFCmpOEQ(L, R, "cmptmp");
            if (op == "!=") return Builder.CreateFCmpONE(L, R, "cmptmp");
        } else {
            if (op == "<")  return Builder.CreateICmpSLT(L, R, "cmptmp");
            if (op == "<=") return Builder.CreateICmpSLE(L, R, "cmptmp");
            if (op == ">")  return Builder.CreateICmpSGT(L, R, "cmptmp");
            if (op == ">=") return Builder.CreateICmpSGE(L, R, "cmptmp");
            if (op == "==") return Builder.CreateICmpEQ(L, R, "cmptmp");
            if (op == "!=") return Builder.CreateICmpNE(L, R, "cmptmp");
        }
    }

    std::cerr << "Error: Unknown binary operator: " << op << "\n";
    return nullptr;
}

// UnaryOpNode
Value* UnaryOpNode::codegen() {
    std::cerr << "Generating code for UnaryOpNode: " << op << "\n";
    Value* Val = operand->codegen();
    if (!Val) {
        std::cerr << "Error: Failed to generate code for operand\n";
        return nullptr;
    }

    if (op == "!") {
        // First convert operand to bool if it isn't already
        Value* BoolVal;
        if (Val->getType()->isIntegerTy(1)) {
            BoolVal = Val;
        } else if (Val->getType()->isIntegerTy()) {
            // Convert integer to bool (0 = false, non-0 = true)
            BoolVal = Builder.CreateICmpNE(
                Val, 
                ConstantInt::get(Val->getType(), 0), 
                "to_bool"
            );
        } else if (Val->getType()->isFloatTy()) {
            // Convert float to bool (0.0 = false, non-0.0 = true)
            BoolVal = Builder.CreateFCmpONE(
                Val,
                ConstantFP::get(Val->getType(), 0.0),
                "to_bool"
            );
        } else {
            std::cerr << "Error: Invalid operand type for ! operator\n";
            return nullptr;
        }
        
        // Now perform the logical NOT
        return Builder.CreateNot(BoolVal, "not");
    } else if (op == "-") {
        // For negation, first ensure we're working with a numeric type
        if (Val->getType()->isFloatTy()) {
            return Builder.CreateFNeg(Val, "neg");
        } else if (Val->getType()->isIntegerTy(32)) {
            return Builder.CreateNeg(Val, "neg");
        } else if (Val->getType()->isIntegerTy(1)) {
            // For bool, first convert to int32, then negate
            Value* IntVal = Builder.CreateZExt(Val, Type::getInt32Ty(TheContext), "bool_to_int");
            return Builder.CreateNeg(IntVal, "neg");
        }
    }

    std::cerr << "Error: Invalid unary operator or operand type\n";
    return nullptr;
}

// AssignNode
Value* AssignNode::codegen() {
    std::cerr << "Generating code for AssignNode: " << name << "\n";
    Value* Val = value->codegen();
    if (!Val) {
        std::cerr << "Error: Failed to generate code for value\n";
        return nullptr;
    }

    VariableInfo* varInfo = nullptr;
    if (NamedValues.find(name) != NamedValues.end()) {
        varInfo = &NamedValues[name];
        std::cerr << "Found " << name << " in local scope\n";
    } else if (GlobalNamedValues.find(name) != GlobalNamedValues.end()) {
        varInfo = &GlobalNamedValues[name];
        std::cerr << "Found " << name << " in global scope\n";
    }

    if (!varInfo) {
        reportError("Assignment to undeclared variable '" + name + "'", loc);
    }

    // Handle all type conversions through convertToType
    if (Val->getType() != varInfo->type) {
        // Pass true for inConditionalContext if assigning to a bool
        bool isAssigningToBool = varInfo->type->isIntegerTy(1);
        Val = convertToType(Val, varInfo->type, isAssigningToBool, loc.line, loc.column);
        if (!Val) {
            std::cerr << "Error: Invalid type conversion\n";
            return nullptr;
        }
    }

    Builder.CreateStore(Val, varInfo->value);
    return Val;
}

// VariableNode
Value* VariableNode::codegen() {
    std::cerr << "Generating code for VariableNode: " << name << "\n";
    
    // First check local variables
    if (NamedValues.find(name) != NamedValues.end()) {
        VariableInfo& varInfo = NamedValues[name];
        return Builder.CreateLoad(varInfo.type, varInfo.value, name.c_str());
    }
    
    // Then check global variables
    if (GlobalNamedValues.find(name) != GlobalNamedValues.end()) {
        VariableInfo& varInfo = GlobalNamedValues[name];
        return Builder.CreateLoad(varInfo.type, varInfo.value, name.c_str());
    }

    // VariableNode::codegen()
    if (NamedValues.find(name) == NamedValues.end() && 
        GlobalNamedValues.find(name) == GlobalNamedValues.end()) {
        reportError("Use of undeclared variable '" + name + "'", loc);
    }
    return nullptr;
}

// FunctionCallNode
Value* FunctionCallNode::codegen() {
    std::cerr << "Generating code for FunctionCallNode: " << name << "\n";
    
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(name);
    if (!CalleeF) {
        reportError("Call to undeclared function '" + name + "'", loc);
    }

    // Check argument count
    size_t expectedArgs = CalleeF->arg_size();
    size_t providedArgs = arguments.size();
    
    if (expectedArgs != providedArgs) {
        // Create modified location pointing to the problematic argument
        SourceLocation errorLoc = loc;
        
        // Calculate new column position:
        // Start with function name position
        int newCol = loc.column;
        
        if (providedArgs > expectedArgs) {
            // For too many args, point to the first extra argument
            // Skip past function name and opening paren
            newCol += name.length() + 1;  // +1 for '('
            
            // Skip past the valid arguments and their commas
            for (size_t i = 0; i < expectedArgs; i++) {
                // Add argument length and ", " separator
                newCol += 2;  // For ", "
            }
        } else {
            // For too few args, point to where the next arg should be
            // Skip to the end of the last provided argument
            newCol += name.length() + 1;  // +1 for '('
            for (size_t i = 0; i < providedArgs; i++) {
                newCol += 2;  // For ", "
            }
        }
        
        errorLoc.column = newCol;

        std::string msg;
        if (providedArgs > expectedArgs) {
            msg = "too many arguments to function call, expected " + 
                  std::to_string(expectedArgs) + " argument" +
                  (expectedArgs != 1 ? "s" : "") + ", have " +
                  std::to_string(providedArgs) + " arguments";
        } else {
            msg = "too few arguments to function call, expected " +
                  std::to_string(expectedArgs) + " argument" +
                  (expectedArgs != 1 ? "s" : "") + ", have " +
                  std::to_string(providedArgs) + " arguments";
        }
        reportError(msg, errorLoc);
    }

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
        ArgsV.push_back(arguments[i]->codegen());
        if (!ArgsV.back()) {
            return nullptr;
        }
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// LiteralNode
Value* LiteralNode::codegen() {
    std::cerr << "Generating code for LiteralNode: ";
    switch (type) {
        case LiteralType::Int:
            std::cerr << value.intValue << "\n";
            return llvm::ConstantInt::get(TheContext, llvm::APInt(32, value.intValue, true)); // 'true' for signed
        case LiteralType::Float:
            std::cerr << value.floatValue << "\n";
            return llvm::ConstantFP::get(TheContext, llvm::APFloat(value.floatValue));
        case LiteralType::Bool:
            std::cerr << (value.boolValue ? "true" : "false") << "\n";
            return llvm::ConstantInt::get(TheContext, llvm::APInt(1, value.boolValue));
        default:
            std::cerr << "Error: Unknown literal type\n";
            return nullptr;
    }
}

Value* ExternListNode::codegen() {
    for (const auto& ext : externs) {
        if (!ext->codegen())
            return nullptr;
    }
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)); // Dummy value
}

Value* DeclListNode::codegen() {
    for (const auto& decl : declarations) {
        if (!decl->codegen())
            return nullptr;
    }
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)); // Dummy value
}