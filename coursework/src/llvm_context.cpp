#include "llvm_context.h"
#include "error_handler.h"


std::map<std::string, FunctionInfo> FunctionDeclarations;
std::vector<std::map<std::string, VariableInfo>> NamedValuesStack = {{}};

// Helper function implementations
llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                        const std::string &VarName,
                                        llvm::Type *VarType) {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                          TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(VarType, nullptr, VarName);
}

llvm::Value* convertToType(llvm::Value* val, llvm::Type* targetType, 
                          bool inConditionalContext, const TOKEN& loc) {
    if (!val || !targetType) return nullptr;

    llvm::Type* sourceType = val->getType();
    if (sourceType == targetType) return val;

    // Widening conversions (always allowed):
    
    // 1. Bool to Int conversion
    if (sourceType->isIntegerTy(1) && targetType->isIntegerTy(32)) {
        // When widening bool to int, true becomes 1 and false becomes 0
        return Builder.CreateZExt(val, targetType, "bool_to_int");
    }
    
    // 2. Bool to Float conversion
    if (sourceType->isIntegerTy(1) && targetType->isFloatTy()) {
        // First convert bool to int, then int to float
        auto intVal = Builder.CreateZExt(val, llvm::Type::getInt32Ty(TheContext), "bool_to_int");
        return Builder.CreateSIToFP(intVal, targetType, "int_to_float");
    }
    
    // 3. Int to Float conversion
    if (sourceType->isIntegerTy(32) && targetType->isFloatTy()) {
        return Builder.CreateSIToFP(val, targetType, "int_to_float");
    }

    // Special case: Converting to Bool in conditional contexts
    if (targetType->isIntegerTy(1)) {
        if (inConditionalContext) {
            if (sourceType->isIntegerTy()) {
                // For ints, non-zero is true
                return Builder.CreateICmpNE(val, 
                    llvm::ConstantInt::get(sourceType, 0), "int_to_bool");
            }
            if (sourceType->isFloatTy()) {
                // For floats, non-zero is true
                return Builder.CreateFCmpONE(val,
                    llvm::ConstantFP::get(sourceType, 0.0), "float_to_bool");
            }
        } else {
            // In non-conditional contexts, only allow bool operations to result in bool
            // For arithmetic operations involving bools, they should be widened to int first
            reportError("Cannot convert to bool outside conditional context", loc);
            return nullptr;
        }
    }

    // Narrowing conversions (generate errors):
    

    // Unsupported type conversions
    std::string sourceTypeName = sourceType->isIntegerTy() ? "int" :
                                 sourceType->isFloatTy() ? "float" :
                                 sourceType->isIntegerTy(1) ? "bool" :
                                 "unknown";
    std::string targetTypeName = targetType->isIntegerTy() ? "int" :
                                 targetType->isFloatTy() ? "float" :
                                 targetType->isIntegerTy(1) ? "bool" :
                                 "unknown";

    // Float to Int
    if (sourceType->isFloatTy() && targetType->isIntegerTy()) {
        reportError("implicit conversion from '" + sourceTypeName + "' to '" + 
                   targetTypeName + "' may lose precision", loc);
        return nullptr;
    }
    std::string errorMessage = "Unsupported type conversion from " + sourceTypeName + 
                                " to " + targetTypeName;
    reportError(errorMessage, loc);
    
    return nullptr;
}

void pushScope() {
    NamedValuesStack.emplace_back();
}

void popScope() {
    if (!NamedValuesStack.empty()) {
        NamedValuesStack.pop_back();
    } else {
        // Handle error: popping from empty scope stack
    }
}

VariableInfo* findVariable(const std::string& name) {
    // Search from innermost scope to outermost
    for (auto scopeIt = NamedValuesStack.rbegin(); scopeIt != NamedValuesStack.rend(); ++scopeIt) {
        auto varIt = scopeIt->find(name);
        if (varIt != scopeIt->end()) {
            return &varIt->second;
        }
    }
    // Check global variables
    auto globalVarIt = GlobalNamedValues.find(name);
    if (globalVarIt != GlobalNamedValues.end()) {
        return &globalVarIt->second;
    }
    return nullptr;
}