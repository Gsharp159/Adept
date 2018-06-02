
#ifndef ASSEMBLE_FIND_H
#define ASSEMBLE_FIND_H

/*
    ============================= assemble_find.h =============================
    Module for locating intermediate representation data structures
    that reside in organized lists
    ---------------------------------------------------------------------------
*/

#include "ir.h"
#include "ast.h"
#include "ground.h"
#include "compiler.h"
#include "irbuilder.h"

// ---------------- funcpair_t ----------------
// A container for function results
typedef struct {
    ast_func_t *ast_func;
    ir_func_t *ir_func;
    length_t func_id;
} funcpair_t;

// ---------------- assemble_find_func ----------------
// Finds a function that exactly matches the given
// name and arguments. Result info stored 'result'
int assemble_find_func(compiler_t *compiler, object_t *object, const char *name,
    ast_type_t *arg_types, length_t arg_types_length, funcpair_t *result);

// ---------------- assemble_find_func_named ----------------
// Finds a function that exactly matches the given name.
// Result info stored 'result'
int assemble_find_func_named(compiler_t *compiler, object_t *object,
    const char *name, funcpair_t *result);

// ---------------- assemble_find_func_conforming ----------------
// Finds a function that has the given name and conforms.
// to the arguments given. Result info stored 'result'
int assemble_find_func_conforming(ir_builder_t *builder, const char *name, ir_value_t **arg_values,
        ast_type_t *arg_types, length_t type_list_length, funcpair_t *result);

// ---------------- assemble_find_method_conforming ----------------
// Finds a method that has the given name and conforms.
// to the arguments given. Result info stored 'result'
int assemble_find_method_conforming(ir_builder_t *builder, const char *struct_name,
    const char *name, ir_value_t **arg_values, ast_type_t *arg_types,
    length_t type_list_length, funcpair_t *result);

// ---------------- find_beginning_of_func_group ----------------
// Searches for beginning of function group in a list of mappings
int find_beginning_of_func_group(ir_func_mapping_t *mappings, length_t length, const char *name);

// ---------------- find_beginning_of_method_group ----------------
// Searches for beginning of method group in a list of methods
int find_beginning_of_method_group(ir_method_t *methods, length_t length,
    const char *struct_name, const char *name);

// ---------------- func_args_match ----------------
// Returns whether a function's arguments match
// the arguments supplied.
bool func_args_match(ast_func_t *func, ast_type_t *type_list, length_t type_list_length);

// ---------------- func_args_conform ----------------
// Returns whether a function's arguments conform
// to the arguments supplied.
bool func_args_conform(ir_builder_t *builder, ast_func_t *func, ir_value_t **arg_value_list,
        ast_type_t *arg_type_list, length_t type_list_length);

#endif // ASSEMBLE_FIND_H
