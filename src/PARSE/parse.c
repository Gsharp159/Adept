
#include "UTIL/util.h"
#include "UTIL/color.h"
#include "UTIL/ground.h"
#include "UTIL/filename.h"
#include "PARSE/parse.h"
#include "PARSE/parse_enum.h"
#include "PARSE/parse_expr.h"
#include "PARSE/parse_func.h"
#include "PARSE/parse_meta.h"
#include "PARSE/parse_type.h"
#include "PARSE/parse_util.h"
#include "PARSE/parse_alias.h"
#include "PARSE/parse_global.h"
#include "PARSE/parse_pragma.h"
#include "PARSE/parse_struct.h"
#include "PARSE/parse_dependency.h"
#include "BRIDGE/any.h"

errorcode_t parse(compiler_t *compiler, object_t *object){
    parse_ctx_t ctx;

    object_init_ast(object);
    parse_ctx_init(&ctx, compiler, object);
    if(!(compiler->traits & COMPILER_INFLATE_PACKAGE)) any_inject_ast(ctx.ast);

    if(parse_tokens(&ctx)) return FAILURE;

    return SUCCESS;
}

errorcode_t parse_tokens(parse_ctx_t *ctx){
    // Expects from 'ctx': compiler, object, tokenlist, ast

    length_t i = 0;
    token_t *tokens = ctx->tokenlist->tokens;
    length_t tokens_length = ctx->tokenlist->length;

    for(ctx->i = &i; i != tokens_length; i++){
        switch(tokens[i].id){
        case TOKEN_NEWLINE:
            break;
        case TOKEN_FUNC: case TOKEN_STDCALL:
            if(parse_func(ctx)) return FAILURE;
            break;
        case TOKEN_FOREIGN:
            if(tokens[i + 1].id == TOKEN_STRING || tokens[i + 1].id == TOKEN_CSTRING){
                parse_foreign_library(ctx);
                break;
            }
            if(parse_func(ctx)) return FAILURE;
            break;
        case TOKEN_STRUCT: case TOKEN_PACKED:
            if(parse_struct(ctx)) return FAILURE;
            break;
        case TOKEN_WORD:
        case TOKEN_EXTERNAL:
            if(parse_global(ctx)) return FAILURE;
            break;
        case TOKEN_ALIAS:
            if(parse_alias(ctx)) return FAILURE;
            break;
        case TOKEN_IMPORT:
            if(parse_import(ctx)) return FAILURE;
            break;
        case TOKEN_PRAGMA:
            if(parse_pragma(ctx)) return FAILURE;
            break;
        case TOKEN_ENUM:
            if(parse_enum(ctx)) return FAILURE;
            break;
        case TOKEN_META:
            if(parse_meta(ctx)) return FAILURE;
            break;
        default:
            parse_panic_token(ctx, ctx->tokenlist->sources[i], tokens[i].id, "Encountered unexpected token '%s' in global scope");
            return FAILURE;
        }
    }

    return SUCCESS;
}
