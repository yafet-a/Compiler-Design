#include "llvm_context.h"
#include "error_handler.h"


std::map<std::string, FunctionInfo> FunctionDeclarations;
std::vector<std::map<std::string, VariableInfo>> NamedValuesStack = {{}};


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
            std::string sourceTypeName = sourceType->isIntegerTy() ? "int" :
                                   sourceType->isFloatTy() ? "float" :
                                   sourceType->isIntegerTy(1) ? "bool" :
                                   "unknown";
            Note note{
            "narrowing conversions are not allowed in return statements and function calls",
            loc 
        };
        
        reportError("Cannot convert from '" + sourceTypeName + "' to 'bool' outside conditional context", 
                   loc, true, &note);
        }
    }

    // For all other cases, report an error
    std::string sourceTypeName = sourceType->isIntegerTy() ? "int" :
                                 sourceType->isFloatTy() ? "float" :
                                 sourceType->isIntegerTy(1) ? "bool" :
                                 "unknown";
    std::string targetTypeName = targetType->isIntegerTy() ? "int" :
                                 targetType->isFloatTy() ? "float" :
                                 targetType->isIntegerTy(1) ? "bool" :
                                 "unknown";

    // All examples of narrowing conversions as listed above for erroring
    if (sourceType->isFloatTy() && targetType->isIntegerTy()) {
        reportError("implicit conversion from '" + sourceTypeName + "' to '" + 
                   targetTypeName + "' may lose precision", loc);
    }
    std::string errorMessage = "Unsupported type conversion from " + sourceTypeName + 
                                " to " + targetTypeName;
    reportError(errorMessage, loc);
}

void pushScope() {
    NamedValuesStack.emplace_back();
}

void popScope() {
    if (!NamedValuesStack.empty()) {
        NamedValuesStack.pop_back();
    } else {
        reportError("Attempted to pop an empty scope", {});
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