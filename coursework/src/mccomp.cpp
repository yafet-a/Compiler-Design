#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include "llvm_context.h"
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include "ast.h"

using namespace llvm;
using namespace llvm::sys;

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
std::unique_ptr<llvm::Module> TheModule;

std::map<std::string, VariableInfo> NamedValues;
std::map<std::string, VariableInfo> GlobalNamedValues;


//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ASTnode &ast) {
  os << ast.to_string();
  return os;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
    if (argc == 2) {
        pFile = fopen(argv[1], "r");
        if (pFile == NULL) {
            perror("Error opening file");
            return 1;
        }
    } else {
        std::cout << "Usage: ./code InputFile\n";
        return 1;
    }

    // Initialize line number and column numbers
    lineNo = 1;
    columnNo = 1;

    // Get the first token
    getNextToken();

    // Make the module, which holds all the code
    TheModule = std::make_unique<Module>("mini-c", TheContext);
    
    // Set up the module target triple
    TheModule->setTargetTriple(llvm::sys::getDefaultTargetTriple());

    // Run the parser and get the AST
    auto ast = parser();
    if (!ast) {
        llvm::errs() << "Failed to generate AST\n";
        return 1;
    }

    // Generate code from the AST
    if (!ast->codegen()) {
        llvm::errs() << "Failed to generate LLVM IR\n";
        return 1;
    }

    // Print out the IR
    auto Filename = "output.ll";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    // Print IR to output.ll
    TheModule->print(dest, nullptr);

    fclose(pFile);
    return 0;
}