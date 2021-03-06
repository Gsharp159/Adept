
#ifndef IR_BUILDER_H
#define IR_BUILDER_H

/*
    =============================== ir_builder.h ===============================
    Module for building intermediate representation
    ----------------------------------------------------------------------------
*/

#include "IR/ir.h"
#include "UTIL/ground.h"
#include "DRVR/object.h"
#include "DRVR/compiler.h"
#include "BRIDGE/bridge.h"

// ---------------- ir_builder_t ----------------
// Container for storing general information
// about the building state
typedef struct {
    ir_basicblock_t *basicblocks;
    length_t basicblocks_length;
    length_t basicblocks_capacity;
    ir_basicblock_t *current_block;
    length_t current_block_id;
    length_t break_block_id; // 0 == none
    length_t continue_block_id; // 0 == none
    bridge_var_scope_t *break_continue_scope;
    strong_cstr_t *block_stack_labels;
    length_t *block_stack_break_ids;
    length_t *block_stack_continue_ids;
    bridge_var_scope_t **block_stack_scopes;
    length_t block_stack_length;
    length_t block_stack_capacity;
    ir_pool_t *pool;
    ir_type_map_t *type_map;
    compiler_t *compiler;
    object_t *object;
    ast_func_t *ast_func;
    ir_func_t *module_func;
    bridge_var_scope_t *var_scope;
    length_t next_var_id;
    length_t *next_reference_id;
    troolean has_string_struct;
} ir_builder_t;

// ---------------- build_basicblock ----------------
// Builds a new basic block in the current function
// NOTE: All basic block pointers should be recalculated
//       after calling this function
length_t build_basicblock(ir_builder_t *builder);

// ---------------- build_using_basicblock ----------------
// Changes the current basic block that new instructions are added into
void build_using_basicblock(ir_builder_t *builder, length_t basicblock_id);

// ---------------- build_instruction ----------------
// Builds a new undetermined instruction
ir_instr_t *build_instruction(ir_builder_t *builder, length_t size);

// ---------------- build_value_from_prev_instruction ----------------
// Builds an IR value from the result of the previsous instruction
ir_value_t *build_value_from_prev_instruction(ir_builder_t *builder);

// ---------------- build_varptr ----------------
// Builds a varptr instruction
ir_value_t* build_varptr(ir_builder_t *builder, ir_type_t *ptr_type, length_t variable_id);

// ---------------- build_varptr ----------------
// Builds a globalvarptr instruction
ir_value_t* build_gvarptr(ir_builder_t *builder, ir_type_t *ptr_type, length_t variable_id);

// ---------------- build_load ----------------
// Builds a load instruction
ir_value_t* build_load(ir_builder_t *builder, ir_value_t *value);

// ---------------- build_store ----------------
// Builds a store instruction
void build_store(ir_builder_t *builder, ir_value_t *value, ir_value_t *destination);

// ---------------- build_break ----------------
// Builds a break instruction
void build_break(ir_builder_t *builder, length_t basicblock_id);

// ---------------- build_static_struct ----------------
// Builds a static struct
ir_value_t *build_static_struct(ir_module_t *module, ir_type_t *type, ir_value_t **values, length_t length, bool make_mutable);

// ---------------- build_struct_construction ----------------
// Builds a struct construction
ir_value_t *build_struct_construction(ir_pool_t *pool, ir_type_t *type, ir_value_t **values, length_t length);

// ---------------- build_static_array ----------------
// Builds a static array
ir_value_t *build_static_array(ir_pool_t *pool, ir_type_t *type, ir_value_t **values, length_t length);

// ---------------- build_anon_global ----------------
// Builds an anonymous global variable
ir_value_t *build_anon_global(ir_module_t *module, ir_type_t *type, bool is_constant);

// ---------------- build_anon_global_initializer ----------------
// Builds an anonymous global variable initializer
void build_anon_global_initializer(ir_module_t *module, ir_value_t *anon_global, ir_value_t *initializer);

// ---------------- ir_builder_funcptr ----------------
// Gets a shared IR function pointer type
ir_type_t* ir_builder_funcptr(ir_builder_t *builder);

// ---------------- ir_builder_usize ----------------
// Gets a shared IR usize type
ir_type_t* ir_builder_usize(ir_builder_t *builder);

// ---------------- ir_builder_usize_ptr ----------------
// Gets a shared IR usize pointer type
ir_type_t* ir_builder_usize_ptr(ir_builder_t *builder);

// ---------------- ir_builder_bool ----------------
// Gets a shared IR boolean type
ir_type_t* ir_builder_bool(ir_builder_t *builder);

// ---------------- build_literal_int ----------------
// Builds a literal int value
ir_value_t* build_literal_int(ir_pool_t *pool, long long value);

// ---------------- build_literal_usize ----------------
// Builds a literal usize value
ir_value_t* build_literal_usize(ir_pool_t *pool, length_t value);

// ---------------- build_literal_str ----------------
// Builds a literal string value
// NOTE: If no 'String' type is present, an error will be printed and NULL will be returned
ir_value_t* build_literal_str(ir_builder_t *builder, char *array, length_t length);

// ---------------- build_literal_cstr ----------------
// Builds a literal c-string value
ir_value_t* build_literal_cstr(ir_builder_t *builder, weak_cstr_t value);

// ---------------- build_literal_cstr ----------------
// Builds a literal c-string value
ir_value_t* build_literal_cstr_of_length(ir_builder_t *builder, weak_cstr_t value, length_t length);

// ---------------- build_null_pointer ----------------
// Builds a literal null pointer value
ir_value_t* build_null_pointer(ir_pool_t *pool);

// ---------------- build_null_pointer_of_type ----------------
// Builds a literal null pointer value
ir_value_t* build_null_pointer_of_type(ir_pool_t *pool, ir_type_t *type);

// ---------------- build_bitcast ----------------
// Builds a bitcast instruction
ir_value_t* build_bitcast(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);

// ---------------- build_const_bitcast ----------------
// Builds a const bitcast value
ir_value_t* build_const_bitcast(ir_pool_t *pool, ir_value_t *from, ir_type_t *to);

// ---------------- build_<cast> ----------------
// Builds a specialized value cast
ir_value_t* build_zext(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_trunc(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_fext(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_ftrunc(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_inttoptr(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_ptrtoint(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_fptoui(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_fptosi(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_uitofp(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);
ir_value_t* build_sitofp(ir_builder_t *builder, ir_value_t *from, ir_type_t *to);

// ---------------- build_math ----------------
// Bulids a basic math instruction
ir_value_t *build_math(ir_builder_t *builder, unsigned int instr_id, ir_value_t *a, ir_value_t *b, ir_type_t *result);

// ---------------- build_bool ----------------
// Builds a literal boolean value
ir_value_t *build_bool(ir_pool_t *pool, bool value);

// ---------------- prepare_for_new_label ----------------
// Ensures there's enough room for another label
void prepare_for_new_label(ir_builder_t *builder);

// ---------------- open_var_scope ----------------
// Opens a new variable scope
void open_var_scope(ir_builder_t *builder);

// ---------------- close_var_scope ----------------
// Closes the current variable scope
void close_var_scope(ir_builder_t *builder);

// ---------------- add_variable ----------------
// Adds a variable to the current bridge_var_scope_t
void add_variable(ir_builder_t *builder, weak_cstr_t name, ast_type_t *ast_type, ir_type_t *ir_type, trait_t traits);

// ---------------- handle_defer_management ----------------
// Handles '__defer__' management method calls for stack allocated variables
void handle_defer_management(ir_builder_t *builder, bridge_var_list_t *list);

// ---------------- handle_pass_management ----------------
// Handles '__pass__' management method calls for passing arguments
// NOTE: 'arg_type_traits' can be NULL
void handle_pass_management(ir_builder_t *builder, ir_value_t **values, ast_type_t *types, trait_t *arg_type_traits, length_t arity);

// ---------------- handle_assign_management ----------------
// Handles '__assign__' management method calls
successful_t handle_assign_management(ir_builder_t *builder, ir_value_t *value, ir_value_t *destination, ast_type_t *type, bool zero_initialize);

// ---------------- handle_math_management ----------------
// Handles basic math management function calls
ir_value_t* handle_math_management(ir_builder_t *builder, ir_value_t *lhs, ir_value_t *rhs,
    ast_type_t *lhs_type, ast_type_t *rhs_type, ast_type_t *out_type, const char *overload_name);

#endif // IR_BUILDER_H
