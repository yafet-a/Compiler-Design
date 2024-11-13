#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include "tokens.h"
#include "ast.h"

// Base error reporting functions
[[noreturn]] void reportError(const std::string& message, 
                             int lineNo,
                             int columnNo);

// TOKEN-based errors (for parser)
[[noreturn]] void reportError(const std::string& message, 
                             const TOKEN& token);

// SourceLocation-based errors (for semantic analysis)
[[noreturn]] void reportError(const std::string& message, 
                             const SourceLocation& loc);

#endif // ERROR_HANDLER_H