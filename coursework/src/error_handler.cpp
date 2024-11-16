// error_handler.cpp
#include "error_handler.h"
#include "llvm/Support/raw_ostream.h"
#include "lexer.h"
#include <iostream>

// Need to match exactly what's in the header - standalone function, not in a class
[[noreturn]] void reportError(const std::string& message, 
                             const TOKEN& token,
                             bool withHighlighting) {

    std::cerr << "Current line content: '" << token.lineContent << "'\n";

    llvm::errs().enable_colors(withHighlighting);
    llvm::errs().enable_colors(withHighlighting);

    if (withHighlighting) {
        llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true)
                    << "\033[1m" << token.filename << ":" << token.lineNo 
                    << ":" << token.columnNo << "\033[0m: ";
        
        llvm::errs().changeColor(llvm::raw_ostream::RED, true) << "error: ";
        llvm::errs().resetColor() << "\033[1m" << message << "\033[0m\n";
    } else {
        llvm::errs() << currentFilename << ":" << token.lineNo << ":" 
                     << token.columnNo << ": error: " << message << "\n";
    }

    if (!token.lineContent.empty()) {
        llvm::errs() << token.lineNo << " | " << token.lineContent << "\n";
        llvm::errs() << "  | ";
        
        if (withHighlighting) {
            llvm::errs().changeColor(llvm::raw_ostream::GREEN, true);
        }
        
        llvm::errs() << std::string(token.columnNo - 1, ' ') << "^";
        llvm::errs() << std::string(token.lexeme.length() > 0 ? token.lexeme.length() - 1 : 3, '~');
        
        if (withHighlighting) {
            llvm::errs().resetColor();
        }
        llvm::errs() << "\n";
    }

    llvm::errs() << "1 error generated.\n";
    exit(1);
}