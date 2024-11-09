#include "ast.h"
#include "llvm_context.h"
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
        llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, name, varType);
        NamedValues[name] = { Alloca, varType, false };  // Store value, type, and global flag
        return Alloca;
    } else {
        // Global variable
        // Check if global variable already exists
        if (TheModule->getGlobalVariable(name)) {
            std::cerr << "Error: Global variable " << name << " already declared\n";
            return nullptr;
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
    
    for (const auto& decl : declarations) {
        Last = decl->codegen();
        if (!Last) {
            std::cerr << "Error: Failed to generate code for declaration\n";
            return nullptr;
        }
    }
    
    for (const auto& stmt : statements) {
        Last = stmt->codegen();
        if (!Last) {
            std::cerr << "Error: Failed to generate code for statement\n";
            return nullptr;
        }
    }
    
    return Last;
}

// IfNode
Value* IfNode::codegen() {
    std::cerr << "Generating code for IfNode\n";
    Value *CondV = condition->codegen();
    if (!CondV) {
        std::cerr << "Error: Failed to generate code for condition\n";
        return nullptr;
    }

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    
    // Create blocks
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
    
    Builder.CreateCondBr(CondV, ThenBB, ElseBB);
    
    // Emit then block
    Builder.SetInsertPoint(ThenBB);
    Value *ThenV = thenBlock->codegen();
    if (!ThenV) {
        std::cerr << "Error: Failed to generate code for then block\n";
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    
    // Emit else block
    // Use insert method instead of accessing list directly
    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder.SetInsertPoint(ElseBB);
    Value *ElseV = elseBlock->codegen();
    if (!ElseV) {
        std::cerr << "Error: Failed to generate code for else block\n";
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    
    // Emit merge block
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder.SetInsertPoint(MergeBB);
    
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
}

// WhileNode
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

    // Add the body block to the function
    TheFunction->insert(TheFunction->end(), BodyBB);

    // Create conditional branch
    Builder.CreateCondBr(CondV, BodyBB, ExitBB);

    // Emit the body block
    Builder.SetInsertPoint(BodyBB);
    if (!body->codegen()) {
        std::cerr << "Error: Failed to generate code for while body\n";
        return nullptr;
    }

    // Create branch back to header block
    Builder.CreateBr(HeaderBB);

    // Add the exit block to the function
    TheFunction->insert(TheFunction->end(), ExitBB);
    Builder.SetInsertPoint(ExitBB);

    return Constant::getNullValue(Type::getInt32Ty(TheContext));
}

// ReturnNode
Value* ReturnNode::codegen() {
    Value* RetVal = nullptr;
    if (value) {
        RetVal = value->codegen();
        if (!RetVal) return nullptr;
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
Value* BinaryOpNode::codegen() {
    llvm::errs() << "Generating code for BinaryOpNode: " << op << "\n";
    Value* L = left->codegen();
    Value* R = right->codegen();

    if (!L || !R) {
        llvm::errs() << "Error: Failed to generate code for operands\n";
        return nullptr;
    }

    llvm::errs() << "Left operand type: " << *L->getType() << "\n";
    llvm::errs() << "Right operand type: " << *R->getType() << "\n";

    // Determine if we're dealing with floating point
    bool isFloating = L->getType()->isFloatTy() || R->getType()->isFloatTy();

    // Convert types if needed
    if (isFloating) {
        if (L->getType()->isIntegerTy()) {
            L = Builder.CreateSIToFP(L, Type::getFloatTy(TheContext), "float_cast");
        }
        if (R->getType()->isIntegerTy()) {
            R = Builder.CreateSIToFP(R, Type::getFloatTy(TheContext), "float_cast");
        }
    } else {
        // For integers, ensure consistent bit width
        if (L->getType() != R->getType()) {
            if (L->getType()->getIntegerBitWidth() > R->getType()->getIntegerBitWidth()) {
                R = Builder.CreateIntCast(R, L->getType(), true, "cast");
            } else {
                L = Builder.CreateIntCast(L, R->getType(), true, "cast");
            }
        }
    }

    // Map operators to LLVM instructions
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        Instruction::BinaryOps binOp;
        
        if (isFloating) {
            if (op == "+") binOp = Instruction::FAdd;
            else if (op == "-") binOp = Instruction::FSub;
            else if (op == "*") binOp = Instruction::FMul;
            else if (op == "/") binOp = Instruction::FDiv;
            else return nullptr;
        } else {
            if (op == "+") binOp = Instruction::Add;
            else if (op == "-") binOp = Instruction::Sub;
            else if (op == "*") binOp = Instruction::Mul;
            else if (op == "/") binOp = Instruction::SDiv;
            else return nullptr;
        }
        
        return Builder.CreateBinOp(binOp, L, R, "binop");
    }
    
    // Handle comparisons
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        if (isFloating) {
            CmpInst::Predicate pred;
            if (op == "<") pred = CmpInst::FCMP_OLT;
            else if (op == ">") pred = CmpInst::FCMP_OGT;
            else if (op == "<=") pred = CmpInst::FCMP_OLE;
            else if (op == ">=") pred = CmpInst::FCMP_OGE;
            else if (op == "==") pred = CmpInst::FCMP_OEQ;
            else if (op == "!=") pred = CmpInst::FCMP_ONE;
            else return nullptr;
            
            return Builder.CreateFCmp(pred, L, R, "fcmp");
        } else {
            CmpInst::Predicate pred;
            if (op == "<") pred = CmpInst::ICMP_SLT;
            else if (op == ">") pred = CmpInst::ICMP_SGT;
            else if (op == "<=") pred = CmpInst::ICMP_SLE;
            else if (op == ">=") pred = CmpInst::ICMP_SGE;
            else if (op == "==") pred = CmpInst::ICMP_EQ;
            else if (op == "!=") pred = CmpInst::ICMP_NE;
            else return nullptr;
            
            return Builder.CreateICmp(pred, L, R, "icmp");
        }
    }

    // Handle logical operators
    if (op == "&&" || op == "||") {
        // Convert operands to bool if needed
        if (!L->getType()->isIntegerTy(1)) {
            L = Builder.CreateICmpNE(L, ConstantInt::get(L->getType(), 0), "tobool");
        }
        if (!R->getType()->isIntegerTy(1)) {
            R = Builder.CreateICmpNE(R, ConstantInt::get(R->getType(), 0), "tobool");
        }

        if (op == "&&") return Builder.CreateAnd(L, R, "and");
        if (op == "||") return Builder.CreateOr(L, R, "or");
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
        // Handle boolean negation
        if (Val->getType()->isIntegerTy(1)) {
            // Direct boolean negation
            return Builder.CreateNot(Val, "nottmp");
        } else if (Val->getType()->isIntegerTy()) {
            // Convert integer to bool first (0 = false, non-0 = true)
            Value* BoolVal = Builder.CreateICmpNE(
                Val, 
                ConstantInt::get(Val->getType(), 0), 
                "booltmp"
            );
            return Builder.CreateNot(BoolVal, "nottmp");
        }
    } else if (op == "-") {
        // Handle numeric negation
        if (Val->getType()->isFloatTy()) {
            return Builder.CreateFNeg(Val, "negtmp");
        } else if (Val->getType()->isIntegerTy()) {
            return Builder.CreateNeg(Val, "negtmp");
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
    } else if (GlobalNamedValues.find(name) != GlobalNamedValues.end()) {
        varInfo = &GlobalNamedValues[name];
    }

    if (!varInfo) {
        std::cerr << "Error: Unknown variable " << name << "\n";
        return nullptr;
    }

    // Type checking and conversion
    if (Val->getType() != varInfo->type) {
        if (varInfo->type->isFloatTy() && Val->getType()->isIntegerTy()) {
            Val = Builder.CreateSIToFP(Val, varInfo->type, "conv");
        } else {
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

    std::cerr << "Error: Unknown variable " << name << "\n";
    return nullptr;
}

// FunctionCallNode
Value* FunctionCallNode::codegen() {
    std::cerr << "Generating code for FunctionCallNode: " << name << "\n";
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(name);
    if (!CalleeF) {
        std::cerr << "Error: Function not found: " << name << "\n";
        return nullptr;
    }

    // Check argument count
    if (CalleeF->arg_size() != arguments.size()) {
        std::cerr << "Error: Incorrect number of arguments for function: " << name << "\n";
        return nullptr;
    }

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
        ArgsV.push_back(arguments[i]->codegen());
        if (!ArgsV.back()) {
            std::cerr << "Error: Failed to generate code for argument " << i << " of function: " << name << "\n";
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