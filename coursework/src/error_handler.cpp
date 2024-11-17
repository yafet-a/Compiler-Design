#include "error_handler.h"
#include "llvm/Support/raw_ostream.h"
#include "lexer.h"
#include <iostream>

[[noreturn]] void reportError(const std::string& message, 
                             const TOKEN& token,
                             bool withHighlighting,
                             const Note* note,
                             const CaretPosition* mainCaret,
                             const CaretPosition* noteCaret) {
    
    llvm::errs().enable_colors(withHighlighting);

    // Print main error
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

    // Print the line and caret for main error
    if (!token.lineContent.empty()) {
        llvm::errs() << "    " << token.lineNo << " | " << token.lineContent << "\n";
        llvm::errs() << "      | ";
        
        if (withHighlighting) {
            llvm::errs().changeColor(llvm::raw_ostream::GREEN, true);
        }
        
        // Get caret position
        int caretCol = mainCaret ? mainCaret->column : token.columnNo;
        
        llvm::errs() << std::string(caretCol - 1, ' ') << "^";
        
        // Only add highlighting if using default position
        if (!mainCaret || mainCaret->useDefaultHighlight) {
            llvm::errs() << std::string(token.lexeme.length() > 0 ? token.lexeme.length() - 1 : 3, '~');
        }
        
        if (withHighlighting) {
            llvm::errs().resetColor();
        }
        llvm::errs() << "\n";
    }

    // Print note if it exists
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

        // Print the line and caret for the note
        if (!note->location.lineContent.empty()) {
            llvm::errs() << "    " << note->location.lineNo << " | " 
                        << note->location.lineContent << "\n";
            llvm::errs() << "      | ";
            
            if (withHighlighting) {
                llvm::errs().changeColor(llvm::raw_ostream::CYAN, true);
            }
            
            // Get caret position
            int caretCol = noteCaret ? noteCaret->column : note->location.columnNo;
            
            llvm::errs() << std::string(caretCol - 1, ' ') << "^";
            
            // Only add highlighting if using default position
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