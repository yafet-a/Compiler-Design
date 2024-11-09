#include "llvm_context.h"

// Helper function implementations
llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                        const std::string &VarName,
                                        llvm::Type *VarType) {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                          TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(VarType, nullptr, VarName);
}

llvm::Value* convertToType(llvm::Value* val, llvm::Type* targetType) {
    if (!val || !targetType) return nullptr;

    llvm::Type* sourceType = val->getType();
    if (sourceType == targetType) return val;

    // Integer to Float conversion
    if (sourceType->isIntegerTy() && targetType->isFloatTy()) {
        return Builder.CreateSIToFP(val, targetType, "float_conv");
    }
    // Float to Integer conversion (Be careful with this, as it can lose precision)
    else if (sourceType->isFloatTy() && targetType->isIntegerTy()) {
        return Builder.CreateFPToSI(val, targetType, "int_conv");
    }
    // Integer to Integer conversion (different bit widths)
    else if (sourceType->isIntegerTy() && targetType->isIntegerTy()) {
        return Builder.CreateIntCast(val, targetType, true, "int_cast");
    }
    // Integer to Boolean conversion
    else if (sourceType->isIntegerTy() && targetType->isIntegerTy(1)) {
        return Builder.CreateICmpNE(val, 
            llvm::ConstantInt::get(sourceType, 0), "bool_conv");
    }
    // Boolean to Integer conversion
    else if (sourceType->isIntegerTy(1) && targetType->isIntegerTy()) {
        return Builder.CreateZExt(val, targetType, "bool_to_int");
    }

    return nullptr;
}