
#ifndef PARSE_H
#define PARSE_H

/*
    ================================= parse.h =================================
    Module for parsing a token list into an abstract syntax tree. Also
    responsible for forwarding 'pragma' directives to the pragma module.
    ---------------------------------------------------------------------------
*/

#include "ast.h"
#include "lex.h"
#include "ground.h"
#include "object.h"

// Internal parse states that determine what to do
#define PARSE_STATE_NONE         0x00000000
#define PARSE_STATE_IDLE         0x00000001
#define PARSE_STATE_FUNC         0x00000002
#define PARSE_STATE_FOREIGN      0x00000003
#define PARSE_STATE_STRUCT       0x00000004
#define PARSE_STATE_GLOBAL       0x00000005
#define PARSE_STATE_ALIAS        0x00000006

// ------------------ parse_ctx_t ------------------
// A general container struct that holds general
// information about the current parsing context
typedef struct {
    compiler_t *compiler;
    object_t *object;
    tokenlist_t *tokenlist;
    ast_t *ast;
    length_t *i;
    ast_func_t *func;
} parse_ctx_t;

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

// ------------------ parse_type ------------------
// Parses a type into an abstract syntax tree form.
int parse_type(parse_ctx_t *ctx, ast_type_t *out_type);

// ------------------ parse_type_func ------------------
// Parses a function type into an abstract syntax tree
// type-element form. Primary called from 'parse_type'.
int parse_type_func(parse_ctx_t *ctx, ast_elem_func_t *out_func_elem);


// ------------------ parse_expr ------------------
// Parses an expression into an AST.
int parse_expr(parse_ctx_t *ctx, ast_expr_t **out_expr);

// ------------------ parse_primary_expr ------------------
// Parses the concrete part of an expression into an AST.
// Primary called from 'parse_expr'.
int parse_primary_expr(parse_ctx_t *ctx, ast_expr_t **out_expr);


// ------------------ parse_op_expr ------------------
// Parses the non-concrete parts of an expression that
// follows the concrete part. This function handles
// expression precedence and most operators.
// Primary called from 'parse_expr'.
int parse_op_expr(parse_ctx_t *ctx, int precedence, ast_expr_t** inout_left, bool keep_mutable);


// ------------------ parse_rhs_expr ------------------
// Parses the right hand side of an operator that takes
// two operands. This is a sub-part of 'parse_op_expr'.
// Primary called from 'parse_op_expr'.
int parse_rhs_expr(parse_ctx_t *ctx, ast_expr_t **left, ast_expr_t **out_right, int op_prec);

// ------------------ parse_stmts ------------------
// Parses one or more statements into two lists.
// 'expr_list' for standard statements
// 'defer_list' for defered statements
int parse_stmts(parse_ctx_t *ctx, ast_expr_list_t *expr_list, ast_expr_list_t *defer_list, trait_t mode);

// Possible modes for 'parse_stmts'
#define PARSE_STMTS_STANDARD TRAIT_NONE // Standard mode (will parse multiple statements)
#define PARSE_STMTS_SINGLE   TRAIT_1    // Single statement mode (will parse a single statement)

// ------------------ parse_stmt_call ------------------
// Parses a function call statement
int parse_stmt_call(parse_ctx_t *ctx, ast_expr_list_t *expr_list);

// ------------------ parse_stmt_declare ------------------
// Parses a variable declaration statement
int parse_stmt_declare(parse_ctx_t *ctx, ast_expr_list_t *expr_list);

// ------------------ parse_unravel_defer_stmts ------------------
// Unravels a list of defered statements into another list of
// statements in reverse order until 'unravel_point' is reached.
void parse_unravel_defer_stmts(ast_expr_list_t *stmts, ast_expr_list_t *defer_list, length_t unravel_point);

// ------------------ parse_ignore_newlines ------------------
// Passes over newlines until something else is encountered
int parse_ignore_newlines(parse_ctx_t *ctx, const char *error_message);

// ------------------ parse_get_precedence ------------------
// Returns the precedence of the expression that will
// be created by a given token id.
int parse_get_precedence(unsigned int id);

// ------------------ parse_panic_token ------------------
// Will print an error message with general information
// using the message 'message' and replacing '%s' within
// it with the name of the token pointed to by 'source'
void parse_panic_token(parse_ctx_t *ctx, source_t source, unsigned int token_id, const char *message);

#endif // PARSE_H