
#ifndef PARSE_FUNC_H
#define PARSE_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "UTIL/ground.h"
#include "PARSE/parse_ctx.h"

// ------------------ parse_func ------------------
// Parses a function
errorcode_t parse_func(parse_ctx_t *ctx);

// ------------------ parse_func_head ------------------
// Parses the head of a function definition
errorcode_t parse_func_head(parse_ctx_t *ctx, strong_cstr_t *out_name, bool *out_is_stdcall, bool *out_is_foreign);

// ------------------ parse_func_body ------------------
// Parses the body of a function
errorcode_t parse_func_body(parse_ctx_t *ctx, ast_func_t *func);

// ------------------ parse_func_arguments ------------------
// Parses the arguments that a function takes
errorcode_t parse_func_arguments(parse_ctx_t *ctx, ast_func_t *func);

// ------------------ parse_func_argument ------------------
// Parses a single argument that a function takes
errorcode_t parse_func_argument(parse_ctx_t *ctx, ast_func_t *func, length_t *backfill, bool *out_is_solid);

// ------------------ parse_func_backfill_arguments ------------------
// Backfills arguments that weren't initially given a type
void parse_func_backfill_arguments(ast_func_t *func, length_t *backfill);

// ------------------ parse_func_grow_arguments ------------------
// Grows function argument list so that another argument can be appended
void parse_func_grow_arguments(ast_func_t *func, length_t backfill, length_t *capacity);

// ------------------ parse_func_is_stdcall ------------------
// Handles 'stdcall' descriptive keyword in function head
bool parse_func_is_stdcall(parse_ctx_t *ctx);

// ------------------ parse_free_unbackfilled_arguments ------------------
// Frees function arguments that never got backfilled
void parse_free_unbackfilled_arguments(ast_func_t *func, length_t backfill);

#ifdef __cplusplus
}
#endif

#endif // PARSE_FUNC_H
