#include "llvm_context.h"
#include "error_handler.h"


// Helper function implementations
llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                        const std::string &VarName,
                                        llvm::Type *VarType) {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                          TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(VarType, nullptr, VarName);
}

llvm::Value* convertToType(llvm::Value* val, llvm::Type* targetType, 
                          bool inConditionalContext, int lineNo, int columnNo) {
    if (!val || !targetType) return nullptr;

    llvm::Type* sourceType = val->getType();
    if (sourceType == targetType) return val;

    // Widening conversions (always allowed):
    
    // 1. Bool to Int conversion
    if (sourceType->isIntegerTy(1) && targetType->isIntegerTy(32)) {
        return Builder.CreateZExt(val, targetType, "bool_to_int");
    }
    
    // 2. Bool to Float conversion
    if (sourceType->isIntegerTy(1) && targetType->isFloatTy()) {
        auto intVal = Builder.CreateZExt(val, llvm::Type::getInt32Ty(TheContext), "bool_to_int");
        return Builder.CreateSIToFP(intVal, targetType, "int_to_float");
    }
    
    // 3. Int to Float conversion
    if (sourceType->isIntegerTy(32) && targetType->isFloatTy()) {
        return Builder.CreateSIToFP(val, targetType, "int_to_float");
    }

    // Special case: Converting to Bool in conditional contexts
    if (targetType->isIntegerTy(1) && inConditionalContext) {
        if (sourceType->isIntegerTy()) {
            return Builder.CreateICmpNE(val, 
                llvm::ConstantInt::get(sourceType, 0), "to_bool");
        }
        if (sourceType->isFloatTy()) {
            return Builder.CreateFCmpONE(val,
                llvm::ConstantFP::get(sourceType, 0.0),  // Use sourceType since we know it's float
                "floattobool");
        }
    }

    // Narrowing conversions (should generate errors):
    
    // Float to Int
    if (sourceType->isFloatTy() && targetType->isIntegerTy()) {
        reportError("Cannot convert float to int (narrowing conversion)", lineNo, columnNo);
    }

    // Float to Bool (only in non-conditional contexts)
    if (sourceType->isFloatTy() && targetType->isIntegerTy(1) && !inConditionalContext) {
        reportError("Cannot convert float to bool outside conditional context", lineNo, columnNo);
    }

    // Int to Bool (only in non-conditional contexts)
    if (sourceType->isIntegerTy(32) && targetType->isIntegerTy(1) && !inConditionalContext) {
        reportError("Cannot convert int to bool outside conditional context", lineNo, columnNo);
    }


    std::cerr << "Error: Unsupported type conversion from ";
    val->getType()->print(llvm::errs());
    std::cerr << " to ";
    targetType->print(llvm::errs());
    std::cerr << "\n";
    
    return nullptr;
}