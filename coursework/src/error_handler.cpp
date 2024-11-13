#include "error_handler.h"
#include "llvm/Support/raw_ostream.h"
#include "lexer.h"

[[noreturn]] void reportError(const std::string& message, 
                             int lineNo, 
                             int columnNo) {
    llvm::errs().enable_colors(true);

    // Filename:Location
    llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true) << "\033[1m" << currentFilename << ":" << lineNo << ":" << columnNo << "\033[0m: ";
    llvm::errs().changeColor(llvm::raw_ostream::RED, true) << "error: ";
    llvm::errs().resetColor() << message << "\n";

    // Show the code line
    if (!currentLineContent.empty()) {
        llvm::errs() << lineNo << " | " << currentLineContent << "\n";
        llvm::errs() << "  | ";
        // Make the caret and squiggly lines green and print them with the proper spacing
        llvm::errs().changeColor(llvm::raw_ostream::GREEN, true);
        llvm::errs() << std::string(columnNo - 1, ' ') << "^" << "~~~~";
        llvm::errs().resetColor() << "\n";
    }

    llvm::errs() << "1 error generated.\n";
    exit(1);
}

[[noreturn]] void reportError(const std::string& message, 
                             const SourceLocation& loc) {
    llvm::errs().enable_colors(true);

    // Filename:Location
    llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true) << "\033[1m" << loc.filename << ":" << loc.line << ":" << loc.column << "\033[0m: ";
    llvm::errs().changeColor(llvm::raw_ostream::RED, true) << "error: ";
    llvm::errs().resetColor() << message << "\n";

    if (!loc.lineContent.empty()) {
        // Line number and content
        llvm::errs() << loc.line << " | " << loc.lineContent << "\n";
        // Spacing for alignment and green caret
        llvm::errs() << "  | ";
        llvm::errs().changeColor(llvm::raw_ostream::GREEN, true);
        llvm::errs() << std::string(loc.column - 1, ' ') << "^" << "~~~~";
        llvm::errs().resetColor() << "\n";
    }

    llvm::errs() << "1 error generated.\n";
    exit(1);
}