
#include "AST/ast.h"
#include "UTIL/util.h"
#include "UTIL/search.h"
#include "PARSE/parse_type.h"
#include "PARSE/parse_alias.h"

errorcode_t parse_alias(parse_ctx_t *ctx){
    // NOTE: Assumes 'alias' keyword

    ast_t *ast = ctx->ast;

    ast_type_t type;
    source_t source = ctx->tokenlist->sources[(*ctx->i)++];

    maybe_null_weak_cstr_t name = parse_eat_word(ctx, "Expected alias name after 'alias' keyword");
    if (name == NULL) return FAILURE;

    const char *invalid_names[] = {
        "Any", "AnyFixedArrayType", "AnyFuncPtrType", "AnyPtrType", "AnyStructType",
        "AnyType", "AnyTypeKind", "String", "StringOwnership", "bool", "byte", "double", "float", "int", "long", "ptr",
        "short", "successful", "ubyte", "uint", "ulong", "ushort", "usize", "void"
    };
    length_t invalid_names_length = sizeof(invalid_names) / sizeof(const char*);

    if(binary_string_search(invalid_names, invalid_names_length, name) != -1){
        compiler_panicf(ctx->compiler, source, "Reserved type name '%s' can't be used to create an alias", name);
        return FAILURE;
    }

    if(parse_eat(ctx, TOKEN_ASSIGN, "Expected '=' after alias name")) return FAILURE;
    if(parse_type(ctx, &type)) return FAILURE;

    expand((void**) &ast->aliases, sizeof(ast_alias_t), ast->aliases_length, &ast->aliases_capacity, 1, 8);

    ast_alias_t *alias = &ast->aliases[ast->aliases_length++];
    ast_alias_init(alias, name, type, TRAIT_NONE, source);
    return SUCCESS;
}
