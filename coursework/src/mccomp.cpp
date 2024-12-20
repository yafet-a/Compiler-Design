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
extern std::string currentFilename;


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
        currentFilename = argv[1];
        pFile = fopen(argv[1], "r");
        if (pFile == NULL) {
            perror("Error opening file");
            return 1;
        }
    } else {
        std::cout << "Usage: ./mccomp InputFile\n";
        return 1;
    }

    // initialize line number and column numbers
    lineNo = 1;
    columnNo = 1;
    std::string filename = argv[1];  // save the filename for the error msg

    // get the first token
    getNextToken();

    // Make the module, which holds all the code
    TheModule = std::make_unique<Module>("mini-c", TheContext);
    
    //Set the target triple
    TheModule->setTargetTriple(llvm::sys::getDefaultTargetTriple());

    // Run the parser and get the AST
    auto ast = parser();
    if (!ast) {
        llvm::errs() << "Failed to generate AST\n";
        return 1;
    }

    // Generate code from the AST
    Value* result = ast->codegen();
    if (!result) {
        // If codegen failed, don't proceed to output
        return 1;
    }

    //********************* Start printing final IR **************************
    // Print out all of the generated code into a file called output.ll
    auto outputFile = "output.ll";
    std::error_code EC;
    raw_fd_ostream dest(outputFile, EC, sys::fs::OF_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    TheModule->print(dest, nullptr);
    fclose(pFile);
    return 0;
}