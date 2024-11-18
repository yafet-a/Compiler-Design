#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include "tokens.h"

struct Note {
    std::string message;
    TOKEN location;
};

struct CaretPosition {
    int column;
    bool useDefaultHighlight;
};

[[noreturn]] void reportError(const std::string& message, 
                             const TOKEN& token,
                             bool withHighlighting = true,
                             const Note* note = nullptr,
                             const CaretPosition* mainCaret = nullptr,
                             const CaretPosition* noteCaret = nullptr);
                             
#endif