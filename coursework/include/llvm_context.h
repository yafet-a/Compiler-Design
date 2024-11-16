#ifndef LLVM_CONTEXT_H
#define LLVM_CONTEXT_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include <map>
#include <string>
#include <iostream>
#include "tokens.h"

extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern std::unique_ptr<llvm::Module> TheModule;

// Symbol tables
struct VariableInfo {
    llvm::Value* value;  // Can be either AllocaInst* or GlobalVariable*
    llvm::Type* type;
    bool isGlobal;
};

extern std::map<std::string, VariableInfo> NamedValues;    // Local variables
extern std::map<std::string, VariableInfo> GlobalNamedValues;   // Global variables

// Helper function declarations
inline llvm::Type* getTypeFromStr(const std::string& type) {
    if (type == "int") return llvm::Type::getInt32Ty(TheContext);
    if (type == "float") return llvm::Type::getFloatTy(TheContext);
    if (type == "bool") return llvm::Type::getInt1Ty(TheContext);
    if (type == "void") return llvm::Type::getVoidTy(TheContext);
    return nullptr;
}

// Function declarations
llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction, 
                                        const std::string &VarName,
                                        llvm::Type *VarType);

llvm::Value* convertToType(llvm::Value* val, llvm::Type* targetType, 
                          bool inConditionalContext = false,
                          const TOKEN& loc = TOKEN());
#endif