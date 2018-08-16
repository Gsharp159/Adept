
#ifndef PARSE_H
#define PARSE_H

/*
    ================================= parse.h =================================
    Module for parsing a token list into an abstract syntax tree. Also
    responsible for forwarding 'pragma' directives to the pragma module.
    ---------------------------------------------------------------------------
*/

#include "PARSE/parse_ctx.h"

// ------------------ parse ------------------
// Entry point of the parser
// Returns 0 on conventional success (parse completed)
// Returns 1 on error
// Returns 2 on unconventional success (parse incomplete)
int parse(compiler_t *compiler, object_t *object);

// ------------------ parse_tokens ------------------
// Indirect entry point of the parser. Takes in a
// parsing context that has already been created.
int parse_tokens(parse_ctx_t *ctx);

#endif // PARSE_H
