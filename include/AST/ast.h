
#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    ================================== ast.h ==================================
    Module for creating and manipulating abstract syntax trees
    ---------------------------------------------------------------------------
*/

#include "UTIL/trait.h"
#include "UTIL/ground.h"
#include "AST/ast_type.h"
#include "AST/ast_expr.h"
#include "AST/meta_directives.h"
#include "BRIDGE/type_table.h"

// ---------------- ast_func_t ----------------
// A function within the root AST
typedef struct {
    strong_cstr_t name;
    weak_cstr_t *arg_names;
    ast_type_t *arg_types;
    source_t *arg_sources;
    char *arg_flows; // in | out | inout
    trait_t *arg_type_traits;
    length_t arity;
    ast_type_t return_type;
    trait_t traits;
    ast_expr_t **statements;
    length_t statements_length;
    length_t statements_capacity;
    source_t source;
} ast_func_t;

#define AST_FUNC_ARG_TYPE_TRAIT_POD TRAIT_1

// Possible AST function traits
#define AST_FUNC_FOREIGN TRAIT_1
#define AST_FUNC_VARARG  TRAIT_2
#define AST_FUNC_MAIN    TRAIT_3
#define AST_FUNC_STDCALL TRAIT_4

// ---------------- ast_struct_t ----------------
// A structure within the root AST
typedef struct {
    strong_cstr_t name;
    strong_cstr_t *field_names;
    ast_type_t *field_types;
    length_t field_count;
    trait_t traits;
    source_t source;
} ast_struct_t;

// Possible AST structure traits
#define AST_STRUCT_PACKED TRAIT_1

// ---------------- ast_alias_t ----------------
// A type alias within the root AST
typedef struct {
    weak_cstr_t name;
    ast_type_t type;
    trait_t traits;
    source_t source;
} ast_alias_t;

// ---------------- ast_constant_t ----------------
// A global constant expression within the root AST
typedef struct {
    weak_cstr_t name;
    ast_expr_t *expression;
    trait_t traits;
    source_t source;
} ast_constant_t;

// ---------------- ast_global_t ----------------
// A global variable within the root AST

typedef struct {
    const char *name;
    ast_type_t type;
    ast_expr_t *initial;
    trait_t traits;
    source_t source;
} ast_global_t;

// Possible ast_global_t traits
#define AST_GLOBAL_EXTERNAL              TRAIT_1
#define AST_GLOBAL_SPECIAL               TRAIT_2
#define AST_GLOBAL___TYPES__             TRAIT_3 // Sub-trait of AST_GLOBAL_SPECIAL
#define AST_GLOBAL___TYPES_LENGTH__      TRAIT_4 // Sub-trait of AST_GLOBAL_SPECIAL
#define AST_GLOBAL___TYPE_KINDS__        TRAIT_5 // Sub-trait of AST_GLOBAL_SPECIAL
#define AST_GLOBAL___TYPE_KINDS_LENGTH__ TRAIT_6 // Sub-trait of AST_GLOBAL_SPECIAL

// ---------------- ast_enum_t ----------------
// An enum AST node
typedef struct {
    weak_cstr_t name;
    weak_cstr_t *kinds;
    length_t length;
    source_t source;
} ast_enum_t;

typedef struct {
    ast_type_t *ast_usize_type;
} ast_shared_common_t;

// ---------------- ast_t ----------------
// The root AST
typedef struct {
    ast_func_t *funcs;
    length_t funcs_length;
    length_t funcs_capacity;
    ast_struct_t *structs;
    length_t structs_length;
    length_t structs_capacity;
    ast_alias_t *aliases;
    length_t aliases_length;
    length_t aliases_capacity;
    ast_constant_t* constants;
    length_t constants_length;
    length_t constants_capacity;
    ast_global_t *globals;
    length_t globals_length;
    length_t globals_capacity;
    ast_enum_t *enums;
    length_t enums_length;
    length_t enums_capacity;
    strong_cstr_t *libraries;
    bool *libraries_are_framework;
    length_t libraries_length;
    length_t libraries_capacity;
    ast_shared_common_t common;

    // Data members for bridging
    type_table_t *type_table;

    // Data members for meta definitions
    meta_definition_t *meta_definitions;
    length_t meta_definitions_length;
    length_t meta_definitions_capacity;
} ast_t;

// ---------------- ast_init ----------------
// Initializes an AST
void ast_init(ast_t *ast);

// ---------------- ast_free ----------------
// Frees data within an AST
void ast_free(ast_t *ast);

// ---------------- ast_free_* ----------------
// Frees a specific part of the data within an AST
void ast_free_functions(ast_func_t *functions, length_t functions_length);
void ast_free_statements(ast_expr_t **statements, length_t length);
void ast_free_statements_fully(ast_expr_t **statements, length_t length);
void ast_free_structs(ast_struct_t *structs, length_t structs_length);
void ast_free_constants(ast_constant_t *constants, length_t constants_length);
void ast_free_globals(ast_global_t *globals, length_t globals_length);
void ast_free_enums(ast_enum_t *enums, length_t enums_length);

// ---------------- ast_dump ----------------
// Converts an AST to a string representation
// and then dumps it to a file
void ast_dump(ast_t *ast, const char *filename);

// ---------------- ast_dump_* ----------------
// Writes a specific part of an AST to a file
void ast_dump_functions(FILE *file, ast_func_t *functions, length_t functions_length);
void ast_dump_statements(FILE *file, ast_expr_t **statements, length_t length, length_t indentation);
void ast_dump_structs(FILE *file, ast_struct_t *structs, length_t structs_length);
void ast_dump_globals(FILE *file, ast_global_t *globals, length_t globals_length);
void ast_dump_enums(FILE *file, ast_enum_t *enums, length_t enums_length);

// ---------------- ast_func_create_template ----------------
// Fills out a blank template for a new function
void ast_func_create_template(ast_func_t *func, strong_cstr_t name, bool is_stdcall, bool is_foreign, source_t source);

// ---------------- ast_struct_init ----------------
// Initializes an AST struct
void ast_struct_init(ast_struct_t *structure, strong_cstr_t name, strong_cstr_t *names, ast_type_t *types,
        length_t length, trait_t traits, source_t source);

// ---------------- ast_alias_init ----------------
// Initializes an AST alias
void ast_alias_init(ast_alias_t *alias, weak_cstr_t name, ast_type_t type, trait_t traits, source_t source);

// ---------------- ast_enum_init ----------------
// Initializes an AST enum
void ast_enum_init(ast_enum_t *inum, weak_cstr_t name, weak_cstr_t *kinds, length_t length, source_t source);

// ---------------- ast_struct_find ----------------
// Finds a structure by name
ast_struct_t *ast_struct_find(ast_t *ast, const char *name);

// ---------------- ast_struct_find_field ----------------
// Finds a field by name within a structure
successful_t ast_struct_find_field(ast_struct_t *ast_struct, const char *name, length_t *out_index);

// ---------------- ast_enum_find_kind ----------------
// Finds a kind by name within an enum
successful_t ast_enum_find_kind(ast_enum_t *ast_enum, const char *name, length_t *out_index);

// ---------------- find_alias ----------------
// Finds an alias by name
maybe_index_t find_alias(ast_alias_t *aliases, length_t aliases_length, const char *alias);

// ---------------- find_constant ----------------
// Finds a global constant expression by name
maybe_index_t find_constant(ast_constant_t *constants, length_t constants_length, const char *constant);

// ---------------- find_enum ----------------
// Finds a enum expression by name
maybe_index_t find_enum(ast_enum_t *enums, length_t enums_length, const char *inum);

// ---------------- ast_add_enum ----------------
// Adds an enum to the global scope of an AST
void ast_add_enum(ast_t *ast, weak_cstr_t name, weak_cstr_t *kinds, length_t length, source_t source);

// ---------------- ast_add_struct ----------------
// Adds a struct to the global scope of an AST
void ast_add_struct(ast_t *ast, strong_cstr_t name, strong_cstr_t *names, ast_type_t *types,
        length_t length, trait_t traits, source_t source);

// ---------------- ast_add_global ----------------
// Adds a global variable to the global scope of an AST
void ast_add_global(ast_t *ast, weak_cstr_t name, ast_type_t type, ast_expr_t *initial_value, trait_t traits, source_t source);

// ---------------- ast_add_foreign_library ----------------
// Adds a library to the list of foreign libraries
// NOTE: Does not have ownership of library string
void ast_add_foreign_library(ast_t *ast, strong_cstr_t library, bool is_framework);

// ---------------- ast_get_usize ----------------
// Gets constant AST type for type 'usize'
ast_type_t* ast_get_usize(ast_t *ast);

// ---------------- ast_aliases_cmp ----------------
// Compares two 'ast_alias_t' structures.
// Used for qsort()
int ast_aliases_cmp(const void *a, const void *b);

// ---------------- ast_constants_cmp ----------------
// Compares two 'ast_constant_t' structures.
// Used for qsort()
int ast_constants_cmp(const void *a, const void *b);

// ---------------- ast_enums_cmp ----------------
// Compares two 'ast_enum_t' structures.
// Used for qsort()
int ast_enums_cmp(const void *a, const void *b);

#ifdef __cplusplus
}
#endif

#endif // AST_H
