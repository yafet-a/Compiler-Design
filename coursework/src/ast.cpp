#include "ast.h"
#include "llvm_context.h"
#include "error_handler.h"
#include <iostream>
#include <sstream>

const char* BRIGHT_MAGENTA = "\033[95m";

const char* VERTICAL = "│   ";
const char* BRANCH = "├── ";
const char* LAST_BRANCH = "└── ";
const char* INDENT = "    ";

std::string formatLoc(const TOKEN& loc) {
    std::stringstream ss;
    ss << BRIGHT_MAGENTA << " <line:" << loc.lineNo << ", col:" << loc.columnNo << ">" << "\033[0m";
    return ss.str();
}

/**

 * @brief Core formatting function for AST node printing
 * 
 * @param indent Current indentation level (chosen to be 4 spaces for better readability)
 * @param isLast Whether this node is the last child at its level
 * @return std::string Tree connection characters showing hierarchy
 * 
 * @details Maintains a stack of boolean flags to track which levels 
 * have pending siblings. This allows proper drawing of vertical 
 * connection lines in multi-level trees.
 */
std::string ASTnode::getPrefix(int indent, bool isLast) {
    std::string result;

    static std::vector<bool> isLastLevel;
    if (indent / 4 > isLastLevel.size()) {
        isLastLevel.push_back(false);
    }
    if (indent / 4 < isLastLevel.size()) {
        isLastLevel.resize(indent / 4);
    }

    if (indent > 0) {
        for (size_t i = 0; i < isLastLevel.size() - 1; ++i) {
            result += isLastLevel[i] ? INDENT : VERTICAL;
        }
        result += isLast ? LAST_BRANCH : BRANCH;
    }

    if (indent / 4 >= 1) {
        isLastLevel[indent / 4 - 1] = isLast;
    }
    return result;
}


std::string BinaryOpNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + 
                        "BinaryOperator" + 
                        formatLoc(loc) + " " +
                        "'" + op + "'" + "\n";
    if (left) {
        result += left->to_string(indent + 4, !right);
    }
    if (right) {
        result += right->to_string(indent + 4, true);
    }
    return result;
}

std::string BlockNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + 
                        "Block" + 
                        formatLoc(loc) + "\n";
    
    std::vector<ASTnode*> allChildren;
    for (const auto& decl : declarations) {
        allChildren.push_back(decl.get());
    }
    for (const auto& stmt : statements) {
        allChildren.push_back(stmt.get());
    }
    
    for (size_t i = 0; i < allChildren.size(); i++) {
        bool isLastChild = (i == allChildren.size() - 1);
        result += allChildren[i]->to_string(indent + 4, isLastChild);
    }
    return result;
}

std::string IfNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "IfStmt" + formatLoc(loc) + "\n";
    
    if (condition) {
        result += condition->to_string(indent + 4, !thenBlock && !elseBlock);
    }
    if (thenBlock) {
        result += thenBlock->to_string(indent + 4, !elseBlock);
    }
    if (elseBlock) {
        result += elseBlock->to_string(indent + 4, true);
    }
    return result;
}

std::string TypeNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "TypeNode" + formatLoc(loc) + 
           " '" + typeName + "'\n";
}

std::string ProgramNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "Program" + formatLoc(loc) + "\n";
    
    for (size_t i = 0; i < externs.size(); i++) {
        bool isLastExtern = (i == externs.size() - 1) && declarations.empty();
        result += externs[i]->to_string(indent + 4, isLastExtern);
    }
    
    for (size_t i = 0; i < declarations.size(); i++) {
        result += declarations[i]->to_string(indent + 4, i == declarations.size() - 1);
    }
    return result;
}

std::string ExternNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExternDecl" + formatLoc(loc) + 
                        " '" + name + "' type='" + type + "'\n";
    
    for (size_t i = 0; i < params.size(); i++) {
        result += getPrefix(indent + 4, i == params.size() - 1) +
                 "ParmVarDecl '" + params[i].second + "' type='" + params[i].first + "'\n";
    }
    return result;
}

std::string FunctionNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + 
                        "FunctionDecl" + 
                        formatLoc(loc) + " " +
                        "'" + name + "' type='" + returnType + "'" + "\n";
    
    for (size_t i = 0; i < params.size(); i++) {
        bool isLastParam = (i == params.size() - 1) && !body;
        result += getPrefix(indent + 4, isLastParam) +
                 "ParmVarDecl" + " " +
                 "'" + params[i].second + "' type='" + params[i].first + "'" + "\n";
    }
    
    if (body) {
        result += body->to_string(indent + 4, true);
    }
    return result;
}


std::string VariableNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "VariableNode" + formatLoc(loc) + 
           " '" + name + "'\n";
}

std::string LiteralNode::to_string(int indent, bool isLast) const {
    std::string typeStr;
    std::string valueStr;
    
    switch (type) {
        case LiteralType::Int:
            typeStr = "IntegerLiteral";
            valueStr = std::to_string(value.intValue);
            break;
        case LiteralType::Float:
            typeStr = "FloatingLiteral";
            valueStr = std::to_string(value.floatValue);
            break;
        case LiteralType::Bool:
            typeStr = "BooleanLiteral";
            valueStr = value.boolValue ? "true" : "false";
            break;
    }
    
    return getPrefix(indent, isLast) + typeStr + formatLoc(loc) + 
           " '" + valueStr + "'\n";
}

std::string VarDeclNode::to_string(int indent, bool isLast) const {
    return getPrefix(indent, isLast) + "VarDecl" + formatLoc(loc) + 
           " '" + name + "' type='" + type + "'\n";
}

std::string WhileNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "WhileStmt" + formatLoc(loc) + "\n";
    
    if (condition) {
        result += condition->to_string(indent + 4, !body);
    }
    if (body) {
        result += body->to_string(indent + 4, true);
    }
    return result;
}


std::string ReturnNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ReturnStmt" + formatLoc(loc) + "\n";
    if (value) {
        result += value->to_string(indent + 4, true);
    }
    return result;
}

std::string ExprStmtNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExprStmt" + formatLoc(loc) + "\n";
    if (expr) {
        result += expr->to_string(indent + 4, true);
    }
    return result;
}

std::string UnaryOpNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "UnaryOperator" + formatLoc(loc) + 
                        " '" + op + "'\n";
    if (operand) {
        result += operand->to_string(indent + 4, true);
    }
    return result;
}

std::string AssignNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "BinaryOperator" + formatLoc(loc) + 
                        " '='\n";
    
    // variable reference
    result += getPrefix(indent + 4, !value) + "DeclRefExpr" + formatLoc(loc) + 
              " '" + name + "'\n";
    
    if (value) {
        result += value->to_string(indent + 4, true);
    }
    return result;
}

std::string FunctionCallNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "FunctionCall" + formatLoc(loc) + 
                        " '" + name + "'\n";
    
    result += getPrefix(indent + 4, arguments.empty()) + 
              "DeclRefExpr" + formatLoc(loc) + " '" + name + "'\n";
    
    for (size_t i = 0; i < arguments.size(); i++) {
        bool isLastArg = (i == arguments.size() - 1);
        result += arguments[i]->to_string(indent + 4, isLastArg);
    }
    return result;
}

std::string DeclListNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "DeclList" + formatLoc(loc) + "\n";
    
    for (size_t i = 0; i < declarations.size(); i++) {
        result += declarations[i]->to_string(indent + 4, i == declarations.size() - 1);
    }
    return result;
}

std::string ExternListNode::to_string(int indent, bool isLast) const {
    std::string result = getPrefix(indent, isLast) + "ExternList" + formatLoc(loc) + "\n";
    
    for (size_t i = 0; i < externs.size(); i++) {
        result += externs[i]->to_string(indent + 4, i == externs.size() - 1);
    }
    return result;
}

Value* ProgramNode::codegen() {
    for (const auto& ext : externs) {
        if (!ext->codegen()) {
            std::cerr << "Error: Failed to generate code for extern\n";
            return nullptr;
        }
    }
    
    for (const auto& decl : declarations) {
        if (!decl->codegen()) {
            std::cerr << "Error: Failed to generate code for declarationp\n";
            return nullptr;
        }
    }
    
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
}


Value* ExternNode::codegen() {

    std::vector<Type*> ArgTypes;
    for (const auto& param : params) {
        Type* paramType = getTypeFromStr(param.first);
        if (!paramType) {
            reportError("Unknown type in function parameter: " + param.first, loc);
        }
        ArgTypes.push_back(paramType);
    }
    
    Type* RetType = getTypeFromStr(type);
    if (!RetType) {
        std::cerr << "Error: Unknown return type: " << type << "\n";
        return nullptr;
    }
    
    FunctionType *FT = FunctionType::get(RetType, ArgTypes, false);
Function *F = Function::Create(
        FT, 
        Function::ExternalLinkage,
        name,
        TheModule.get()
    );
    F->setCallingConv(llvm::CallingConv::C);    
    // Setting up the parameter names
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(params[Idx++].second);
    }
    
    return F;
}


Value* VarDeclNode::codegen() {
    llvm::Type* varType = getTypeFromStr(type);
    if (!varType) {
        reportError("Unknown type in variable declaration: " + type, loc);
    }

    llvm::Function* TheFunction = Builder.GetInsertBlock() ? Builder.GetInsertBlock()->getParent() : nullptr;

    if (TheFunction) {
        auto& CurrentScope = NamedValuesStack.back();

        if (CurrentScope.find(name) != CurrentScope.end()) {
            Note note{
                "previous declaration of '" + name + "' was here",
                CurrentScope[name].declLocation
            };
            reportError("Redefinition of local variable '" + name + "'", loc, true, &note);
        }

        llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, name, varType);


        CurrentScope[name] = { Alloca, varType, false, loc };
        return Alloca;
    } else {

        if (TheModule->getGlobalVariable(name)) {
            Note note{
                "previous declaration of '" + name + "' was here",
                GlobalNamedValues[name].declLocation
            };
            reportError("Redefinition of global variable '" + name + "'", loc, true, &note);
        }

        llvm::GlobalVariable* GlobalVar = new llvm::GlobalVariable(
            *TheModule,
            varType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvm::Constant::getNullValue(varType),
            name
        );

        GlobalNamedValues[name] = { GlobalVar, varType, true, loc };
        return GlobalVar;
    }
}

Value* FunctionNode::codegen() {
    Function* ExistingFunc = TheModule->getFunction(name);

    std::vector<Type*> ArgTypes;
    for (const auto& param : params) {
        Type* paramType = getTypeFromStr(param.first);
        if (!paramType) {
            reportError("Unknown type in function parameter: " + param.first, loc);
        }
        ArgTypes.push_back(paramType);
    }
    
    Type* RetType = getTypeFromStr(returnType);
    if (!RetType) {
        reportError("Unknown return type: " + returnType, loc);
    }
    
    FunctionType *FT = FunctionType::get(RetType, ArgTypes, false);
    
    // add error checking for redefinition and type conflicts before creating the function (important for mutual recursion)
    if (Function* existingFunc = TheModule->getFunction(name)) {
        // check for type mismatch with existing declaration
        if (existingFunc->getFunctionType() != FT) {
            Note note{
                "previous declaration is here",
                FunctionDeclarations[name].declLocation
            };
            reportError("conflicting types for '" + name + "'", loc, true, &note);
        }
        if (!existingFunc->empty()) {
            Note note{
                "previous definition is here",
                FunctionDeclarations[name].declLocation
            };
            reportError("redefinition of '" + name + "'", loc, true, &note);
        }
        
    }

    Function *F;
    if (ExistingFunc) {
        if (ExistingFunc->getFunctionType() != FT) {
            reportError("Function redefinition with different type", loc);
        }
        F = ExistingFunc;
    } else {
        F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
        F->setCallingConv(llvm::CallingConv::C);
    }
    
    // don't create a new body if one already exists
    if (!F->empty()) {
        reportError("Redefinition of function '" + name + "'", loc);
    }
    
    // using functiondeclarations for the reportError notes
    FunctionDeclarations[name] = { F, loc };
    
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    
    pushScope();
    auto& CurrentScope = NamedValuesStack.back();
    
    // set up parameters
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(params[Idx].second);
        llvm::Type* paramType = ArgTypes[Idx];
        AllocaInst *Alloca = CreateEntryBlockAlloca(F, params[Idx].second, paramType);
        Builder.CreateStore(&Arg, Alloca);

        // check for duplicate parameter names within the same scope
        if (CurrentScope.find(params[Idx].second) != CurrentScope.end()) {
            Note note{
                "previous declaration of parameter '" + params[Idx].second + "' was here",
                CurrentScope[params[Idx].second].declLocation
            };
            reportError("Duplicate parameter name '" + params[Idx].second + "'", loc, true, &note);
            popScope();
        }

        // store VariableInfo in CurrentScope
        CurrentScope[params[Idx].second] = { Alloca, paramType, false, loc };
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
            
            verifyFunction(*F);
            popScope();
            return F;
        } else {
            std::cerr << "Error: Failed to generate code for function body\n";
            popScope();
        }
    } else {
        std::cerr << "Error: Function body is missing\n";
    }
    
    F->eraseFromParent();
    popScope();
    return nullptr;
}


Value* BlockNode::codegen() {
    Value* Last = nullptr;

    //begin by pushing a new scope
    pushScope();

    // decls
    for (const auto& decl : declarations) {
        Last = decl->codegen();
        if (!Last) {
            reportError("Failed to generate code for declaration", decl->loc);
            popScope();
        }
    }

    // stmts
    for (const auto& stmt : statements) {
        Last = stmt->codegen();
        if (!Last) {
            reportError("Failed to generate code for statement", stmt->loc);
            popScope();
        }

        // If block is terminated like hitting a return, break
        if (Builder.GetInsertBlock()->getTerminator())
            break;
    }
    
    popScope();

    return Last;
}


Value* IfNode::codegen() {
    Value *CondV = condition->codegen();
    if (!CondV) {
        reportError("Failed to generate code for if condition", loc);
    }

    // Convert condition to bool using convertToType with conditional context
    if (!CondV->getType()->isIntegerTy(1)) {
        CondV = convertToType(CondV, llvm::Type::getInt1Ty(TheContext), true, loc);
        if (!CondV) {
            reportError("Failed to convert condition to bool", loc);
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
        reportError("Failed to generate code for then block", loc);
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
            reportError("Failed to generate code for else block", loc);
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

Value* WhileNode::codegen() {
    
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
        reportError("Failed to generate code for while condition", loc);
    }

    // Convert condition to bool using convertToType with conditional context
    if (!CondV->getType()->isIntegerTy(1)) {
        CondV = convertToType(CondV, llvm::Type::getInt1Ty(TheContext), true, loc);
        if (!CondV) {
            reportError("Failed to convert condition to bool", loc);
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
        reportError("Failed to generate code for while body", loc);
    }

    // Create branch back to header block if the block isn't already terminated
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(HeaderBB);
    }

    // Move to exit block
    Builder.SetInsertPoint(ExitBB);

    return Constant::getNullValue(Type::getInt32Ty(TheContext));
}

Value* ReturnNode::codegen() {
    // Get function and its return type
    Function* TheFunction = Builder.GetInsertBlock()->getParent();
    Type* RetType = TheFunction->getReturnType();
    std::string FuncName = TheFunction->getName().str();
    
    // If function returns void but we have a return value
    if (RetType->isVoidTy() && value) {
        reportError("void function '" + FuncName + "' cannot return a value", loc);
    }
    
    // If function doesn't return void but we don't have a return value
    if (!RetType->isVoidTy() && !value) {
        reportError("non-void function '" + FuncName + "' should return a value", loc);
    }

    Value* RetVal = nullptr;
    if (value) {
        RetVal = value->codegen();
        if (!RetVal){
            reportError("Failed to generate code for return value", loc);
        }

        // If the return value type does not match the function's return type, try to convert it
        if (RetVal && RetVal->getType() != RetType) {
            RetVal = convertToType(RetVal, RetType, false, loc);
            if (!RetVal) {
                reportError("Failed to convert return value to function return type", loc);
            }
        }
    }
    
    return Builder.CreateRet(RetVal);
}

Value* ExprStmtNode::codegen() {
    Value* Val = expr->codegen();
    if (!Val) {
        reportError("Failed to generate code for expression statement", loc);
    }
    // The value is not used further, as it's an expression statement.
    return Val;
}

Value* BinaryOpNode::codegen() {

    // Lazy evaluation for logical operators
    if (op == "&&" || op == "||") {
        Function* TheFunction = Builder.GetInsertBlock()->getParent();
        
        // Generate code for left operand
        Value* L = left->codegen();
        if (!L){
            reportError("Failed to generate code for left operand", loc);
        };

        // Convert to bool if needed
        if (!L->getType()->isIntegerTy(1)) {
            L = convertToType(L, Type::getInt1Ty(TheContext), true, loc);
            if (!L){
                reportError("Failed to convert left operand to bool", loc);
            }
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

        if (!R->getType()->isIntegerTy(1)) {
            R = convertToType(R, Type::getInt1Ty(TheContext), true, loc);
        }

        // Store the RHS end block for PHI
        BasicBlock* RHSEndBlock = Builder.GetInsertBlock();
        Builder.CreateBr(MergeBlock);

        // Emit merge block
        Builder.SetInsertPoint(MergeBlock);
        PHINode* PN = Builder.CreatePHI(Type::getInt1Ty(TheContext), 2, "logical.result");

        // Adding incoming values for the PHI node
        if (op == "&&") {
            PN->addIncoming(ConstantInt::getFalse(TheContext), EntryBlock);
            PN->addIncoming(R, RHSEndBlock);
        } else {
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
    }

    // For all binary operations, handle type conversions first
    if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
        if (!L->getType()->isFloatTy()) {
            L = convertToType(L, Type::getFloatTy(TheContext), false, loc);
        }
        if (!R->getType()->isFloatTy()) {
            R = convertToType(R, Type::getFloatTy(TheContext), false, loc);
        }
        
        // Floating point operations
        if (op == "+") return Builder.CreateFAdd(L, R, "addtmp");
        if (op == "-") return Builder.CreateFSub(L, R, "subtmp");
        if (op == "*") return Builder.CreateFMul(L, R, "multmp");
        if (op == "/") return Builder.CreateFDiv(L, R, "divtmp");
        if (op == "%") {
            reportError("Modulo not supported for floating point", loc);
            return nullptr;
        }
        // Float comparisons
        if (op == "<")  return Builder.CreateFCmpOLT(L, R, "cmptmp");
        if (op == "<=") return Builder.CreateFCmpOLE(L, R, "cmptmp");
        if (op == ">")  return Builder.CreateFCmpOGT(L, R, "cmptmp");
        if (op == ">=") return Builder.CreateFCmpOGE(L, R, "cmptmp");
        if (op == "==") return Builder.CreateFCmpOEQ(L, R, "cmptmp");
        if (op == "!=") return Builder.CreateFCmpONE(L, R, "cmptmp");
    } 
    else {
        // For non-float operations, convert to int32 (except when both are bool and doing comparison)
        bool bothBool = L->getType()->isIntegerTy(1) && R->getType()->isIntegerTy(1);
        bool isComparison = (op == "==" || op == "!=" || op == "<" || op == "<=" || 
                           op == ">" || op == ">=");
        
        if (!bothBool || !isComparison) {
            if (!L->getType()->isIntegerTy(32)) {
                L = convertToType(L, Type::getInt32Ty(TheContext), false, loc);
            }
            if (!R->getType()->isIntegerTy(32)) {
                R = convertToType(R, Type::getInt32Ty(TheContext), false, loc);
            }
        }

        // Integer operations
        if (op == "+") return Builder.CreateAdd(L, R, "addtmp");
        if (op == "-") return Builder.CreateSub(L, R, "subtmp");
        if (op == "*") return Builder.CreateMul(L, R, "multmp");
        if (op == "/") return Builder.CreateSDiv(L, R, "divtmp");
        if (op == "%") return Builder.CreateSRem(L, R, "modtmp");
        // Integer comparisons
        if (op == "<")  return Builder.CreateICmpSLT(L, R, "cmptmp");
        if (op == "<=") return Builder.CreateICmpSLE(L, R, "cmptmp");
        if (op == ">")  return Builder.CreateICmpSGT(L, R, "cmptmp");
        if (op == ">=") return Builder.CreateICmpSGE(L, R, "cmptmp");
        if (op == "==") return Builder.CreateICmpEQ(L, R, "cmptmp");
        if (op == "!=") return Builder.CreateICmpNE(L, R, "cmptmp");
    }

    reportError("Unknown binary operator: " + op, loc);
}

Value* UnaryOpNode::codegen() {
    Value* Val = operand->codegen();
    if (!Val) {
        reportError("Failed to generate code for operand", loc);
    }

    if (op == "!") {
        // Convert operand to bool in conditional context since we're doing logical operation
        Value* BoolVal = convertToType(Val, Type::getInt1Ty(TheContext), true, loc);
        if (!BoolVal) {
            reportError("Failed to convert operand to bool", loc);
        }
        
        // Perform the logical NOT
        return Builder.CreateNot(BoolVal, "not");
    } 
    else if (op == "-") {
        // For negation, determine target type based on input
        if (Val->getType()->isFloatTy()) {
            return Builder.CreateFNeg(Val, "neg");
        } 
        else {
            // For any integer type (including bool), convert to int32 first
            Value* IntVal = convertToType(Val, Type::getInt32Ty(TheContext), false, loc);
            if (!IntVal) {
                reportError("Failed to convert operand to int", loc);
            }
            return Builder.CreateNeg(IntVal, "neg");
        }
    }

    reportError("Unknown unary operator: " + op, loc);
}

Value* AssignNode::codegen() {
    Value* Val = value->codegen();
    if (!Val) {
        reportError("Failed to generate code for assignment value", loc);
    }

    //find the variable in the current scope
    VariableInfo* varInfo = findVariable(name);
    if (!varInfo) {
        reportError("Use of undeclared identifier '" + name + "'", loc);
    }

    // Handle all type conversions through convertToType
    if (Val->getType() != varInfo->type) {
        // Assignments should be allowed to convert to bool if needed since it isn't a return or function call like the spec specifies
        bool isAssigningToBool = varInfo->type->isIntegerTy(1);
        Val = convertToType(Val, varInfo->type, isAssigningToBool, loc);
        if (!Val) {
            reportError("Failed to convert value to variable type", loc);
        }
    }

    Builder.CreateStore(Val, varInfo->value);
    return Val;
}

// VariableNode
Value* VariableNode::codegen() {

    //find the variable in the current scope
    VariableInfo* varInfo = findVariable(name);
    if (!varInfo) {
        reportError("Use of undeclared identifier '" + name + "'", loc);
    }

    return Builder.CreateLoad(varInfo->type, varInfo->value, name.c_str());
}
// FunctionCallNode
Value* FunctionCallNode::codegen() {
    
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(name);
    if (!CalleeF) {
        reportError("Call to undeclared function '" + name + "'", loc);
    }

    // Check argument count
    size_t expectedArgs = CalleeF->arg_size();
    size_t providedArgs = arguments.size();
    
    if (expectedArgs != providedArgs) {
        TOKEN errorLoc = loc;
        errorLoc.columnNo = loc.columnNo;  // Where "foo" starts
        errorLoc.lexeme = name;  // Set lexeme to function name for highlighting

        // Calculate caret position for the problematic argument
        int caretCol;
        if (providedArgs > expectedArgs) {
            // For too many args, put caret at first extra argument
            caretCol = loc.columnNo + name.length() + 1;  // After "foo("
            for (size_t i = 0; i < expectedArgs; i++) {
                caretCol += 2;  // Skip past each valid argument and comma
            }
        } else {
            // For too few args, put caret at end of last provided argument
            caretCol = loc.columnNo + name.length() + 1;
            for (size_t i = 0; i < providedArgs; i++) {
                caretCol += 2;  // Skip past each provided argument and comma
            }
        }

        // Create caret position - just the caret, no highlighting
        CaretPosition caret{caretCol, false};
        
        std::string msg;
        if (providedArgs > expectedArgs) {
            msg = "too many arguments to function call, expected " +
                std::to_string(expectedArgs) + ", have " +
                std::to_string(providedArgs);
        } else {
            msg = "too few arguments to function call, expected " +
                std::to_string(expectedArgs) + ", have " +
                std::to_string(providedArgs);
        }

        Note note{
            "function '" + name + "' declared here",
            FunctionDeclarations[name].declLocation
        };
        
        reportError(msg, errorLoc, true, &note, &caret);
    }
    std::vector<Value *> ArgsV;
    auto funcArgsIt = CalleeF->arg_begin();  // Get iterator for function parameters

    for (unsigned i = 0; i < arguments.size(); i++, ++funcArgsIt) {
        Value* ArgVal = arguments[i]->codegen();
        if (!ArgVal) {
            reportError("Failed to generate code for function argument", arguments[i]->loc);
        }

        // Get the expected parameter type from the function declaration
        Type* paramType = funcArgsIt->getType();
        
        // Convert argument to the parameter type if needed
        if (ArgVal->getType() != paramType) {
            ArgVal = convertToType(ArgVal, paramType, false, arguments[i]->loc);
            if (!ArgVal) {
                reportError("Failed to convert argument to parameter type", arguments[i]->loc);
            }
        }
        
        ArgsV.push_back(ArgVal);
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// LiteralNode
Value* LiteralNode::codegen() {
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
            reportError("Unknown literal type", loc);
    }
}

Value* ExternListNode::codegen() {
    for (const auto& ext : externs) {
        if (!ext->codegen())
            reportError("Failed to generate code for extern", ext->loc);
    }
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
}

Value* DeclListNode::codegen() {
    for (const auto& decl : declarations) {
        if (!decl->codegen())
            reportError("Failed to generate code for declaration", decl->loc);
    }
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
}