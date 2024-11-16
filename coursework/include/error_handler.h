#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include "tokens.h"

// The main error reporting function
[[noreturn]] void reportError(const std::string& message, const TOKEN& token, bool withHighlighting = true);

#endif // ERROR_HANDLER_H