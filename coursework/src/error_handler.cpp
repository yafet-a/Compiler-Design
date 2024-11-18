#include "error_handler.h"
#include "llvm/Support/raw_ostream.h"
#include "lexer.h"
#include <iostream>

/**
* @brief Main / Core error reporting function used for both semantic and syntax errors in the parser and codegen
*
* @param message The main error message to display
* @param token Token containing source location and lexeme information for the error
* @param withHighlighting Whether to use ANSI colors in the output
* @param note Optional additional context about the error
* @param mainCaret Optional custom caret position for the main error
* @param noteCaret Optional custom caret position for the note
* 
* @details Produces error output in clang-style format:
* filename:line:col: error: message
*     line | source code
*          |    ^~~~
* 
* If a note is provided, adds additional context in the same format but with
* "note:" instead of "error:". Custom caret positions can override the default
* token position for more precise error indication.
*
* Color coding (when enabled):
* - Location: Bold
* - Error: Red
* - Note: Cyan 
* - Carets: Green for main error, Cyan for notes
*
* @note Function is marked [[noreturn]] as it calls exit(1) meaning I don't need ot call nullptr and segfault to quit the program
*/
[[noreturn]] void reportError(const std::string& message, 
                             const TOKEN& token,
                             bool withHighlighting,
                             const Note* note,
                             const CaretPosition* mainCaret,
                             const CaretPosition* noteCaret) {
    
    llvm::errs().enable_colors(withHighlighting);

    // Main error message
    if (withHighlighting) {
        llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true)
                    << "\033[1m" << token.filename << ":" << token.lineNo 
                    << ":" << token.columnNo << "\033[0m: ";
        
        llvm::errs().changeColor(llvm::raw_ostream::RED, true) << "error: ";
        llvm::errs().resetColor() << "\033[1m" << message << "\033[0m\n";
    } else {
        llvm::errs() << token.filename << ":" << token.lineNo << ":" 
                     << token.columnNo << ": error: " << message << "\n";
    }

    if (!token.lineContent.empty()) {
        llvm::errs() << "    " << token.lineNo << " | " << token.lineContent << "\n";
        llvm::errs() << "      | ";
        
        if (withHighlighting) {
            llvm::errs().changeColor(llvm::raw_ostream::GREEN, true);
        }
        
        int caretCol = mainCaret ? mainCaret->column : token.columnNo;
        
        llvm::errs() << std::string(caretCol - 1, ' ') << "^";
        
        if (!mainCaret || mainCaret->useDefaultHighlight) {
            llvm::errs() << std::string(token.lexeme.length() > 0 ? token.lexeme.length() - 1 : 3, '~');
        }
        
        if (withHighlighting) {
            llvm::errs().resetColor();
        }
        llvm::errs() << "\n";
    }

    // Additional note to be used to show function and variable definitions similar to clang
    if (note) {
        if (withHighlighting) {
            llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true)
                        << note->location.filename << ":" << note->location.lineNo 
                        << ":" << note->location.columnNo << ": ";
            llvm::errs().changeColor(llvm::raw_ostream::CYAN, true) << "note: ";
            llvm::errs().resetColor() << note->message << "\n";
        } else {
            llvm::errs() << note->location.filename << ":" << note->location.lineNo 
                        << ":" << note->location.columnNo << ": note: " 
                        << note->message << "\n";
        }
        if (!note->location.lineContent.empty()) {
            llvm::errs() << "    " << note->location.lineNo << " | " 
                        << note->location.lineContent << "\n";
            llvm::errs() << "      | ";
            
            if (withHighlighting) {
                llvm::errs().changeColor(llvm::raw_ostream::CYAN, true);
            }
            
            int caretCol = noteCaret ? noteCaret->column : note->location.columnNo;
            
            llvm::errs() << std::string(caretCol - 1, ' ') << "^";
            
            if (!noteCaret || noteCaret->useDefaultHighlight) {
                llvm::errs() << std::string(note->location.lexeme.length() > 0 ? 
                                          note->location.lexeme.length() - 1 : 3, '~');
            }
            
            if (withHighlighting) {
                llvm::errs().resetColor();
            }
            llvm::errs() << "\n";
        }
    }

    llvm::errs() << "1 error generated.\n";
    exit(1);
}