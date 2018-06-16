
#include "ground.h"
#include "assemble.h"
#include "filename.h"
#include "assemble_find.h"
#include "assemble_type.h"
#include "assemble_expr.h"

int assemble_expression(ir_builder_t *builder, ast_expr_t *expr, ir_value_t **ir_value, bool leave_mutable, ast_type_t *out_expr_type){
    // NOTE: Generates an ir_value_t from an ast_expr_t
    // NOTE: Will write determined ast_type_t to 'out_expr_type' (Can use NULL to ignore)

    ir_instr_t *instruction;

    #define BUILD_MATH_OP_IvF_MACRO(i, f, o, E) { \
        instruction = assemble_math_operands(builder, expr, ir_value, o, out_expr_type); \
        if(instruction == NULL) return 1; \
        if(i_vs_f_instruction((ir_instr_math_t*) instruction, i, f)){ \
            compiler_panic(builder->compiler, ((ast_expr_math_t*) expr)->source, E); \
            ast_type_free(out_expr_type); \
            return 1; \
        } \
    }

    #define BUILD_MATH_OP_UvSvF_MACRO(u, s, f, o, E) { \
        instruction = assemble_math_operands(builder, expr, ir_value, o, out_expr_type); \
        if(instruction == NULL) return 1; \
        if(u_vs_s_vs_float_instruction((ir_instr_math_t*) instruction, u, s, f)){ \
            compiler_panic(builder->compiler, ((ast_expr_math_t*) expr)->source, E); \
            ast_type_free(out_expr_type); \
            return 1; \
        } \
    }

    #define BUILD_MATH_OP_BOOL_MACRO(i, E) { \
        instruction = assemble_math_operands(builder, expr, ir_value, MATH_OP_ALL_BOOL, out_expr_type); \
        if(instruction == NULL) return 1; \
        instruction->id = i; \
    }

    #define BUILD_LITERAL_IR_VALUE(ast_expr_type, typename, storage_type) { \
        if(out_expr_type != NULL) ast_type_make_base_newstr(out_expr_type, typename); \
        *ir_value = ir_pool_alloc(builder->pool, sizeof(ir_value_t)); \
        (*ir_value)->value_type = VALUE_TYPE_LITERAL; \
        ir_type_map_find(builder->type_map, typename, &((*ir_value)->type)); \
        (*ir_value)->extra = ir_pool_alloc(builder->pool, sizeof(storage_type)); \
        *((storage_type*) (*ir_value)->extra) = ((ast_expr_type*) expr)->value; \
    }

    switch(expr->id){
    case EXPR_BYTE:
        BUILD_LITERAL_IR_VALUE(ast_expr_byte_t, "byte", char); break;
    case EXPR_UBYTE:
        BUILD_LITERAL_IR_VALUE(ast_expr_ubyte_t, "ubyte", unsigned char); break;
    case EXPR_SHORT:
        BUILD_LITERAL_IR_VALUE(ast_expr_short_t, "short", /*stored w/ extra precision*/ int); break;
    case EXPR_USHORT:
        BUILD_LITERAL_IR_VALUE(ast_expr_ushort_t, "ushort", /*stored w/ extra precision*/ unsigned int); break;
    case EXPR_INT:
        BUILD_LITERAL_IR_VALUE(ast_expr_int_t, "int", long long); break;
    case EXPR_UINT:
        BUILD_LITERAL_IR_VALUE(ast_expr_uint_t, "uint", unsigned long long); break;
    case EXPR_LONG:
        BUILD_LITERAL_IR_VALUE(ast_expr_long_t, "long", long long); break;
    case EXPR_ULONG:
        BUILD_LITERAL_IR_VALUE(ast_expr_ulong_t, "ulong", unsigned long long); break;
    case EXPR_FLOAT:
        BUILD_LITERAL_IR_VALUE(ast_expr_float_t, "float", /*stored w/ extra precision*/ double); break;
    case EXPR_DOUBLE:
        BUILD_LITERAL_IR_VALUE(ast_expr_double_t, "double", double); break;
    case EXPR_BOOLEAN:
        BUILD_LITERAL_IR_VALUE(ast_expr_boolean_t, "bool", bool); break;
    case EXPR_NULL:
        if(out_expr_type != NULL) ast_type_make_base_newstr(out_expr_type, "ptr");
        (*ir_value) = ir_pool_alloc(builder->pool, sizeof(ir_value_t));
        (*ir_value)->value_type = VALUE_TYPE_NULLPTR;
        ir_type_map_find(builder->type_map, "ptr", &((*ir_value)->type));
        break;
    case EXPR_ADD:
        BUILD_MATH_OP_IvF_MACRO(INSTRUCTION_ADD, INSTRUCTION_FADD, MATH_OP_RESULT_MATCH, "Can't add those types"); break;
    case EXPR_SUBTRACT:
        BUILD_MATH_OP_IvF_MACRO(INSTRUCTION_SUBTRACT, INSTRUCTION_FSUBTRACT, MATH_OP_RESULT_MATCH, "Can't subtract those types"); break;
    case EXPR_MULTIPLY:
        BUILD_MATH_OP_IvF_MACRO(INSTRUCTION_MULTIPLY, INSTRUCTION_FMULTIPLY, MATH_OP_RESULT_MATCH, "Can't multiply those types"); break;
    case EXPR_DIVIDE:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_UDIVIDE, INSTRUCTION_SDIVIDE, INSTRUCTION_FDIVIDE, MATH_OP_RESULT_MATCH, "Can't divide those types"); break;
    case EXPR_MODULUS:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_UMODULUS, INSTRUCTION_SMODULUS, INSTRUCTION_FMODULUS, MATH_OP_RESULT_MATCH, "Can't take the modulus of those types"); break;
    case EXPR_EQUALS:
        BUILD_MATH_OP_IvF_MACRO(INSTRUCTION_EQUALS, INSTRUCTION_FEQUALS, MATH_OP_RESULT_BOOL, "Can't test equality for those types"); break;
    case EXPR_NOTEQUALS:
        BUILD_MATH_OP_IvF_MACRO(INSTRUCTION_NOTEQUALS, INSTRUCTION_FNOTEQUALS, MATH_OP_RESULT_BOOL, "Can't test inequality for those types"); break;
    case EXPR_GREATER:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_UGREATER, INSTRUCTION_SGREATER, INSTRUCTION_FGREATER, MATH_OP_RESULT_BOOL, "Can't compare those types"); break;
    case EXPR_LESSER:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_ULESSER, INSTRUCTION_SLESSER, INSTRUCTION_FLESSER, MATH_OP_RESULT_BOOL, "Can't compare those types"); break;
    case EXPR_GREATEREQ:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_UGREATEREQ, INSTRUCTION_SGREATEREQ, INSTRUCTION_FGREATEREQ, MATH_OP_RESULT_BOOL, "Can't compare those types"); break;
    case EXPR_LESSEREQ:
        BUILD_MATH_OP_UvSvF_MACRO(INSTRUCTION_ULESSEREQ, INSTRUCTION_SLESSEREQ, INSTRUCTION_FLESSEREQ, MATH_OP_RESULT_BOOL, "Can't compare those types"); break;
    case EXPR_AND:
        BUILD_MATH_OP_BOOL_MACRO(INSTRUCTION_AND, "Can't use operator 'and' on those types"); break;
    case EXPR_OR:
        BUILD_MATH_OP_BOOL_MACRO(INSTRUCTION_OR, "Can't use operator 'or' on those types"); break;
    case EXPR_CSTR:
        if(out_expr_type != NULL) ast_type_make_baseptr_newstr(out_expr_type, "ubyte");
        build_string_literal(builder, ((ast_expr_cstr_t*) expr)->value, ir_value); break;
    case EXPR_VARIABLE: {
            char *variable_name = ((ast_expr_variable_t*) expr)->name;
            length_t var_index;

            // Attempt to find variable in local scope
            if(ir_var_map_find(&builder->module_func->var_map, variable_name, &var_index) == false){
                // No error occured, so it exists in the local scope
                // Assume it exists in the function's ast_var_list_t
                if(out_expr_type != NULL) {
                    ast_var_list_t *var_list = &builder->ast_func->var_list;
                    for(length_t v = 0; v != var_list->length; v++) if(strcmp(variable_name, var_list->names[v]) == 0){
                        *out_expr_type = ast_type_clone(var_list->types[v]);
                        break;
                    }
                }

                ir_var_t *var = &builder->module_func->var_map.mappings[var_index].var;
                ir_type_t *var_pointer_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
                var_pointer_type->kind = TYPE_KIND_POINTER;
                var_pointer_type->extra = var->type;

                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_varptr_t));
                ((ir_instr_varptr_t*) instruction)->id = INSTRUCTION_VARPTR;
                ((ir_instr_varptr_t*) instruction)->result_type = var_pointer_type;
                ((ir_instr_varptr_t*) instruction)->index = var_index;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);

                // If not requested to leave the expression mutable, dereference it
                if(!leave_mutable){
                    ir_basicblock_new_instructions(builder->current_block, 1);
                    instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                    ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                    ((ir_instr_load_t*) instruction)->result_type = var->type;
                    ((ir_instr_load_t*) instruction)->value = *ir_value;
                    builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                    *ir_value = build_value_from_prev_instruction(builder);
                }
                break;
            }

            ast_t *ast = &builder->object->ast;
            ir_module_t *ir_module = &builder->object->ir_module;

            // Attempt to find global variable
            bool found_global = false;
            for(var_index = 0; var_index != ir_module->globals_length; var_index++){
                if(strcmp(variable_name, ir_module->globals[var_index].name) == 0){
                    found_global = true; break;
                }
            }

            if(found_global){
                // Assume it exists in ast's globals
                if(out_expr_type != NULL) for(length_t g = 0; g != ast->globals_length; g++){
                    if(strcmp(variable_name, ast->globals[g].name) == 0){
                        *out_expr_type = ast_type_clone(&ast->globals[g].type);
                        break;
                    }
                }

                ir_global_t *global = &ir_module->globals[var_index];
                ir_type_t *global_pointer_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
                global_pointer_type->kind = TYPE_KIND_POINTER;
                global_pointer_type->extra = global->type;

                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_varptr_t));
                ((ir_instr_varptr_t*) instruction)->id = INSTRUCTION_GLOBALVARPTR;
                ((ir_instr_varptr_t*) instruction)->result_type = global_pointer_type;
                ((ir_instr_varptr_t*) instruction)->index = var_index;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);

                // If not requested to leave the expression mutable, dereference it
                if(!leave_mutable){
                    ir_basicblock_new_instructions(builder->current_block, 1);
                    instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                    ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                    ((ir_instr_load_t*) instruction)->result_type = global->type;
                    ((ir_instr_load_t*) instruction)->value = *ir_value;
                    builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                    *ir_value = build_value_from_prev_instruction(builder);
                }
                break;
            }

            // No suitable variable or global found
            compiler_panicf(builder->compiler, ((ast_expr_variable_t*) expr)->source, "Undeclared variable '%s'", variable_name);
            const char *nearest = ast_var_list_nearest(&builder->ast_func->var_list, variable_name);
            if(nearest) printf("\nDid you mean '%s'?\n", nearest);
            return 1;
        }
        break;
    case EXPR_CALL: {
            ast_expr_call_t *call_expr = (ast_expr_call_t*) expr;
            ir_value_t **arg_values = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * call_expr->arity);
            ast_type_t *arg_types = malloc(sizeof(ast_type_t) * call_expr->arity);

            bool found_variable = false;
            ast_type_t *variable_type;
            ir_type_t *ir_var_type;
            ir_type_t *ir_return_type;
            ast_t *ast = &builder->object->ast;

            // Resolve & Assemble function arguments
            for(length_t a = 0; a != call_expr->arity; a++){
                if(assemble_expression(builder, call_expr->args[a], &arg_values[a], false, &arg_types[a])){
                    for(length_t t = 0; t != a; t++) ast_type_free(&arg_types[t]);
                    free(arg_types);
                    return 1;
                }
            }

            for(length_t a = 0; a != builder->ast_func->var_list.length; a++){
                if(strcmp(builder->ast_func->var_list.names[a], call_expr->name) == 0){
                    variable_type = builder->ast_func->var_list.types[a];

                    // We could search the ir var map, but this is easier
                    // TODO: Create easy way to get between ast and ir var maps
                    ir_var_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
                    ir_var_type->kind = TYPE_KIND_POINTER;

                    if(assemble_resolve_type(builder->compiler, builder->object, variable_type, (ir_type_t**) &ir_var_type->extra)){
                        for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                        free(arg_types);
                        return 1;
                    }

                    if(variable_type->elements[0]->id != AST_ELEM_FUNC){
                        char *s = ast_type_str(variable_type);
                        compiler_panicf(builder->compiler, call_expr->source, "Can't call value of non function type '%s'", s);
                        free(s);
                        return 1;
                    }

                    ast_type_t *ast_function_return_type = ((ast_elem_func_t*) variable_type->elements[0])->return_type;

                    if(assemble_resolve_type(builder->compiler, builder->object, ast_function_return_type, &ir_return_type)){
                        for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                        free(arg_types);
                        return 1;
                    }

                    if(variable_type->elements_length == 1 && variable_type->elements[0]->id == AST_ELEM_FUNC){
                        ir_basicblock_new_instructions(builder->current_block, 2);
                        instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_varptr_t));
                        ((ir_instr_varptr_t*) instruction)->id = INSTRUCTION_VARPTR;
                        ((ir_instr_varptr_t*) instruction)->result_type = ir_var_type;
                        ((ir_instr_varptr_t*) instruction)->index = a;
                        builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                        *ir_value = build_value_from_prev_instruction(builder);

                        instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                        ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                        ((ir_instr_load_t*) instruction)->result_type = (ir_type_t*) ir_var_type->extra;
                        ((ir_instr_load_t*) instruction)->value = *ir_value;
                        builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                        *ir_value = build_value_from_prev_instruction(builder);

                        found_variable = true;
                    }
                    break;
                }
            }

            if(!found_variable) for(length_t g = 0; g != ast->globals_length; g++){
                if(strcmp(ast->globals[g].name, call_expr->name) == 0){
                    variable_type = &ast->globals[g].type;

                    // We could search the ir var map, but this is easier
                    // TODO: Create easy way to get between ast and ir var maps
                    ir_var_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
                    ir_var_type->kind = TYPE_KIND_POINTER;

                    if(assemble_resolve_type(builder->compiler, builder->object, variable_type, (ir_type_t**) &ir_var_type->extra)){
                        for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                        free(arg_types);
                        return 1;
                    }

                    if(variable_type->elements[0]->id != AST_ELEM_FUNC){
                        char *s = ast_type_str(variable_type);
                        compiler_panicf(builder->compiler, call_expr->source, "Can't call value of non function type '%s'", s);
                        free(s);
                        return 1;
                    }

                    ast_type_t *ast_function_return_type = ((ast_elem_func_t*) variable_type->elements[0])->return_type;

                    if(assemble_resolve_type(builder->compiler, builder->object, ast_function_return_type, &ir_return_type)){
                        for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                        free(arg_types);
                        return 1;
                    }

                    if(variable_type->elements_length == 1 && variable_type->elements[0]->id == AST_ELEM_FUNC){
                        ir_basicblock_new_instructions(builder->current_block, 2);
                        instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_varptr_t));
                        ((ir_instr_varptr_t*) instruction)->id = INSTRUCTION_GLOBALVARPTR;
                        ((ir_instr_varptr_t*) instruction)->result_type = ir_var_type;
                        ((ir_instr_varptr_t*) instruction)->index = g;
                        builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                        *ir_value = build_value_from_prev_instruction(builder);

                        instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                        ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                        ((ir_instr_load_t*) instruction)->result_type = (ir_type_t*) ir_var_type->extra;
                        ((ir_instr_load_t*) instruction)->value = *ir_value;
                        builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                        *ir_value = build_value_from_prev_instruction(builder);

                        found_variable = true;
                    }
                    break;
                }
            }

            if(found_variable){
                // This is ok since we previously checked that (variable_type->elements[0]->id == AST_ELEM_FUNC)
                ast_elem_func_t *function_elem = (ast_elem_func_t*) variable_type->elements[0];

                if(function_elem->traits & AST_FUNC_VARARG){
                    if(function_elem->arity > call_expr->arity){
                        compiler_panicf(builder->compiler, call_expr->source, "Incorrect argument count (at least %d expected, %d given)", (int) function_elem->arity, (int) call_expr->arity);
                        return 1;
                    }
                } else if(function_elem->arity != call_expr->arity){
                    compiler_panicf(builder->compiler, call_expr->source, "Incorrect argument count (%d expected, %d given)", (int) function_elem->arity, (int) call_expr->arity);
                    return 1;
                }

                for(length_t a = 0; a != function_elem->arity; a++){
                    if(!ast_types_conform(builder, &arg_values[a], &arg_types[a], &function_elem->arg_types[a], CONFORM_MODE_PRIMITIVES)){
                        char *s1 = ast_type_str(&function_elem->arg_types[a]);
                        char *s2 = ast_type_str(&arg_types[a]);
                        compiler_panicf(builder->compiler, call_expr->args[a]->source, "Required argument type '%s' is incompatible with type '%s'", s1, s2);
                        free(s1);
                        free(s2);
                        return 1;
                    }
                }

                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_call_address_t));
                ((ir_instr_call_address_t*) instruction)->id = INSTRUCTION_CALL_ADDRESS;
                ((ir_instr_call_address_t*) instruction)->result_type = ir_return_type;
                ((ir_instr_call_address_t*) instruction)->address = *ir_value;
                ((ir_instr_call_address_t*) instruction)->values = arg_values;
                ((ir_instr_call_address_t*) instruction)->values_length = call_expr->arity;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);

                for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                free(arg_types);

                if(out_expr_type != NULL) *out_expr_type = ast_type_clone(function_elem->return_type);
            } else {
                // Find function that fits given name and arguments
                funcpair_t pair;
                if(assemble_find_func_conforming(builder, call_expr->name, arg_values, arg_types, call_expr->arity, &pair)){
                    compiler_panicf(builder->compiler, expr->source, "Undeclared function '%s'", call_expr->name);
                    for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                    free(arg_types);
                    return 1;
                }

                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_call_t));
                ((ir_instr_call_t*) instruction)->id = INSTRUCTION_CALL;
                ((ir_instr_call_t*) instruction)->result_type = pair.ir_func->return_type;
                ((ir_instr_call_t*) instruction)->values = arg_values;
                ((ir_instr_call_t*) instruction)->values_length = call_expr->arity;
                ((ir_instr_call_t*) instruction)->func_id = pair.func_id;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);

                for(length_t t = 0; t != call_expr->arity; t++) ast_type_free(&arg_types[t]);
                free(arg_types);

                if(out_expr_type != NULL) *out_expr_type = ast_type_clone(&pair.ast_func->return_type);
            }
        }
        break;
    case EXPR_MEMBER: {
            ast_expr_member_t *member_expr = (ast_expr_member_t*) expr;
            ir_value_t *struct_value;
            ast_type_t struct_value_ast_type;

            // This expression should be able to be mutable (Checked during parsing)
            if(assemble_expression(builder, member_expr->value, &struct_value, true, &struct_value_ast_type)) return 1;

            // Auto-derefernce '*something' types
            if(struct_value_ast_type.elements_length > 1 && struct_value_ast_type.elements[0]->id == AST_ELEM_POINTER
                && struct_value_ast_type.elements[1]->id != AST_ELEM_POINTER){
                // Modify ast_type_t to remove a pointer element from the front
                // DANGEROUS: Manually deleting ast_elem_pointer_t
                free(struct_value_ast_type.elements[0]);
                memmove(struct_value_ast_type.elements, &struct_value_ast_type.elements[1], sizeof(ast_elem_t*) * (struct_value_ast_type.elements_length - 1));
                struct_value_ast_type.elements_length--; // Reduce length accordingly

                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                ((ir_instr_load_t*) instruction)->result_type = ((ir_type_t*) struct_value->type->extra);
                ((ir_instr_load_t*) instruction)->value = struct_value;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                struct_value = build_value_from_prev_instruction(builder);
            }

            if(struct_value_ast_type.elements_length == 0 || struct_value_ast_type.elements[0]->id != AST_ELEM_BASE){
                char *nonstruct_typename = ast_type_str(&struct_value_ast_type);
                compiler_panicf(builder->compiler, expr->source, "Can't use member operator on non-struct type '%s'", nonstruct_typename);
                ast_type_free(&struct_value_ast_type);
                free(nonstruct_typename);
                return 1;
            }

            char *struct_name = ((ast_elem_base_t*) struct_value_ast_type.elements[0])->base;
            ast_struct_t *target = ast_struct_find(&builder->object->ast, struct_name);

            if(target == NULL){
                compiler_panicf(builder->compiler, ((ast_expr_member_t*) expr)->source, "INTERNAL ERROR: Failed to find struct '%s' that should exist", struct_name);
                ast_type_free(&struct_value_ast_type);
                return 1;
            }

            length_t field_index;
            if(ast_struct_find_field(target, member_expr->member, &field_index)){
                char *struct_typename = ast_type_str(&struct_value_ast_type);
                compiler_panicf(builder->compiler, ((ast_expr_member_t*) expr)->source, "Field '%s' doesn't exist in struct '%s'", member_expr->member, struct_typename);
                ast_type_free(&struct_value_ast_type);
                free(struct_typename);
                return 1;
            }

            ir_type_t *field_type;
            if(assemble_resolve_type(builder->compiler, builder->object, &target->field_types[field_index], &field_type)) return 1;

            ir_type_t *field_ptr_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
            field_ptr_type->kind = TYPE_KIND_POINTER;
            field_ptr_type->extra = field_type;

            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_member_t));
            ((ir_instr_member_t*) instruction)->id = INSTRUCTION_MEMBER;
            ((ir_instr_member_t*) instruction)->result_type = field_ptr_type;
            ((ir_instr_member_t*) instruction)->value = struct_value;
            ((ir_instr_member_t*) instruction)->member = field_index;
            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            // If not requested to leave the expression mutable, dereference it
            if(!leave_mutable){
                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                ((ir_instr_load_t*) instruction)->result_type = field_type;
                ((ir_instr_load_t*) instruction)->value = *ir_value;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);
            }

            ast_type_free(&struct_value_ast_type);
            if(out_expr_type != NULL) *out_expr_type = ast_type_clone(&target->field_types[field_index]);
        }
        break;
    case EXPR_ADDRESS: {
            // This expression should be able to be mutable (Checked during parsing)
            if(assemble_expression(builder, ((ast_expr_address_t*) expr)->value, ir_value, true, out_expr_type)) return 1;
            if(out_expr_type != NULL) ast_type_prepend_ptr(out_expr_type);
        }
        break;
    case EXPR_FUNC_ADDR: {
            ast_expr_func_addr_t *func_addr_expr = (ast_expr_func_addr_t*) expr;

            funcpair_t pair;
            if(assemble_find_func_named(builder->compiler, builder->object, func_addr_expr->name, &pair)){
                compiler_panicf(builder->compiler, expr->source, "Undeclared function '%s'", func_addr_expr->name);
                return 1;
            }

            ir_type_t *ir_funcptr_type = builder->object->ir_module.common.ir_funcptr_type;
            if(ir_funcptr_type == NULL){
                ir_funcptr_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
                ir_funcptr_type->kind = TYPE_KIND_FUNCPTR;
                // 'ir_funcptr_type->extra' not set because never used
                builder->object->ir_module.common.ir_funcptr_type = ir_funcptr_type;
            }

            const char *maybe_name = pair.ast_func->traits & AST_FUNC_FOREIGN ||
                pair.ast_func->traits & AST_FUNC_MAIN ? func_addr_expr->name : NULL;

            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_func_address_t));
            ((ir_instr_func_address_t*) instruction)->id = INSTRUCTION_FUNC_ADDRESS;
            ((ir_instr_func_address_t*) instruction)->result_type = ir_funcptr_type;
            ((ir_instr_func_address_t*) instruction)->name = maybe_name;
            ((ir_instr_func_address_t*) instruction)->func_id = pair.func_id;
            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            if(out_expr_type != NULL){
                out_expr_type->elements = malloc(sizeof(ast_elem_t*));
                out_expr_type->elements_length = 1;
                out_expr_type->source = func_addr_expr->source;

                ast_elem_func_t *function_elem = malloc(sizeof(ast_elem_func_t));
                function_elem->id = AST_ELEM_FUNC;
                function_elem->source = func_addr_expr->source;
                function_elem->arg_types = pair.ast_func->arg_types;
                function_elem->arg_flows = pair.ast_func->arg_flows;
                function_elem->arity = pair.ast_func->arity;
                function_elem->return_type = &pair.ast_func->return_type;
                function_elem->traits = pair.ast_func->traits;
                function_elem->ownership = false;

                out_expr_type->elements[0] = (ast_elem_t*) function_elem;
            }
        }
        break;
    case EXPR_DEREFERENCE: {
            ast_expr_deref_t *dereference_expr = (ast_expr_deref_t*) expr;
            ast_type_t expr_type;
            ir_value_t *expr_value;

            if(assemble_expression(builder, dereference_expr->value, &expr_value, false, &expr_type)) return 1;

            // Ensure that the result ast_type_t is a pointer type
            if(expr_type.elements_length < 2 || expr_type.elements[0]->id != AST_ELEM_POINTER){
                char *expr_type_str = ast_type_str(&expr_type);
                compiler_panicf(builder->compiler, dereference_expr->source, "Can't dereference non-pointer type '%s'", expr_type_str);
                ast_type_free(&expr_type);
                free(expr_type_str);
                return 1;
            }

            // Modify ast_type_t to remove a pointer element from the front
            // DANGEROUS: Manually deleting ast_elem_pointer_t
            free(expr_type.elements[0]);
            memmove(expr_type.elements, &expr_type.elements[1], sizeof(ast_elem_t*) * (expr_type.elements_length - 1));
            expr_type.elements_length--; // Reduce length accordingly

            // If not requested to leave the expression mutable, dereference it
            if(!leave_mutable){
                // ir_type_t is expected to be of kind pointer
                if(expr_value->type->kind != TYPE_KIND_POINTER){
                    compiler_panic(builder->compiler, dereference_expr->source, "INTERNAL ERROR: Expected ir_type_t to be a pointer inside EXPR_DEREFERENCE of assemble_expression()");
                    ast_type_free(&expr_type);
                    return 1;
                }

                // Build and append a load instruction
                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                ((ir_instr_load_t*) instruction)->result_type = (ir_type_t*) expr_value->type->extra;
                ((ir_instr_load_t*) instruction)->value = expr_value;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);

                // ir_type_t is expected to be of kind pointer
                if(expr_value->type->kind != TYPE_KIND_POINTER){
                    compiler_panic(builder->compiler, dereference_expr->source, "INTERNAL ERROR: Expected ir_type_t to be a pointer inside EXPR_DEREFERENCE of assemble_expression()");
                    ast_type_free(&expr_type);
                    return 1;
                }
            } else {
                *ir_value = expr_value;
            }

            if(out_expr_type != NULL) *out_expr_type = expr_type;
            else ast_type_free(&expr_type);
        }
        break;
    case EXPR_ARRAY_ACCESS: {
            ast_expr_array_access_t *array_access_expr = (ast_expr_array_access_t*) expr;
            ast_type_t index_type, array_type;
            ir_value_t *index_value, *array_value;

            if(assemble_expression(builder, array_access_expr->value, &array_value, false, &array_type)) return 1;
            if(assemble_expression(builder, array_access_expr->index, &index_value, false, &index_type)){
                ast_type_free(&array_type);
                return 1;
            }

            if(index_value->type->kind < TYPE_KIND_S8 || index_value->type->kind > TYPE_KIND_U64){
                compiler_panic(builder->compiler, array_access_expr->index->source, "Array index value must be an integer type");
                ast_type_free(&array_type);
                ast_type_free(&index_type);
                return 1;
            }

            // Ensure array type is a pointer
            if(array_value->type->kind != TYPE_KIND_POINTER || array_type.elements_length < 2 || array_type.elements[0]->id != AST_ELEM_POINTER){
                char *given_type = ast_type_str(&array_type);
                compiler_panicf(builder->compiler, array_access_expr->source, "Can't use operator [] on non-array type '%s'", given_type);
                free(given_type);
                ast_type_free(&array_type);
                ast_type_free(&index_type);
                return 1;
            }

            // Change 'array_type' to be the type of an element instead of the whole array
            // Modify ast_type_t to remove a pointer element from the front
            // DANGEROUS: Manually deleting ast_elem_pointer_t
            free(array_type.elements[0]);
            memmove(array_type.elements, &array_type.elements[1], sizeof(ast_elem_t*) * (array_type.elements_length - 1));
            array_type.elements_length--; // Reduce length accordingly

            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_array_access_t));
            ((ir_instr_array_access_t*) instruction)->id = INSTRUCTION_ARRAY_ACCESS;
            ((ir_instr_array_access_t*) instruction)->result_type = array_value->type;
            ((ir_instr_array_access_t*) instruction)->value = array_value;
            ((ir_instr_array_access_t*) instruction)->index = index_value;
            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            // If not requested to leave the expression mutable, dereference it
            if(!leave_mutable){
                ir_basicblock_new_instructions(builder->current_block, 1);
                instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                ((ir_instr_load_t*) instruction)->id = INSTRUCTION_LOAD;
                ((ir_instr_load_t*) instruction)->result_type = (ir_type_t*) array_value->type->extra;
                ((ir_instr_load_t*) instruction)->value = *ir_value;
                builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
                *ir_value = build_value_from_prev_instruction(builder);
            }

            ast_type_free(&index_type);

            if(out_expr_type != NULL) *out_expr_type = array_type; // array_type was modified to be the element type (bad ik but whatever)
            else ast_type_free(&array_type);
        }
        break;
    case EXPR_CAST: {
            ast_expr_cast_t *cast_expr = (ast_expr_cast_t*) expr;
            ast_type_t from_type;

            if(assemble_expression(builder, cast_expr->from, ir_value, false, &from_type)) return 1;

            if(!ast_types_conform(builder, ir_value, &from_type, &cast_expr->to, CONFORM_MODE_ALL)){
                char *a_type_str = ast_type_str(&from_type);
                char *b_type_str = ast_type_str(&cast_expr->to);
                compiler_panicf(builder->compiler, expr->source, "Can't cast type '%s' to type '%s'", a_type_str, b_type_str);
                free(a_type_str);
                free(b_type_str);
                ast_type_free(&from_type);
                return 1;
            }

            ast_type_free(&from_type);
            if(out_expr_type != NULL) *out_expr_type = ast_type_clone(&cast_expr->to);
        }
        break;
    case EXPR_SIZEOF: {
            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_sizeof_t));
            ((ir_instr_sizeof_t*) instruction)->id = INSTRUCTION_SIZEOF;
            ((ir_instr_sizeof_t*) instruction)->result_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
            ((ir_instr_sizeof_t*) instruction)->result_type->kind = TYPE_KIND_U64;
            // result_type->extra should never be accessed

            if(assemble_resolve_type(builder->compiler, builder->object, &((ast_expr_sizeof_t*) expr)->type,
                    &((ir_instr_sizeof_t*) instruction)->type)) return 1;

            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            if(out_expr_type != NULL) ast_type_make_base_newstr(out_expr_type, "usize");
        }
        break;
    case EXPR_CALL_METHOD: {
            ast_expr_call_method_t *call_expr = (ast_expr_call_method_t*) expr;
            ir_value_t **arg_values = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * (call_expr->arity + 1));
            ast_type_t *arg_types = malloc(sizeof(ast_type_t) * (call_expr->arity + 1));

            // Resolve & Assemble function arguments
            if(assemble_expression(builder, call_expr->value, &arg_values[0], true, &arg_types[0])){
                free(arg_types);
                return 1;
            }

            ast_elem_t **type_elems = arg_types[0].elements;
            length_t type_elems_length = arg_types[0].elements_length;

            if(type_elems_length == 1 && type_elems[0]->id == AST_ELEM_BASE){
                if(!EXPR_IS_MUTABLE(call_expr->value->id)){
                    compiler_panic(builder->compiler, call_expr->source, "Can't access field of immutable value");
                    ast_type_free(&arg_types[0]);
                    free(arg_types);
                    return 1;
                }

                ast_type_prepend_ptr(&arg_types[0]);
            } else if(type_elems_length == 2 && type_elems[0]->id == AST_ELEM_POINTER && type_elems[1]->id == AST_ELEM_BASE){
                // Load the value that's being called on if the expression is mutable
                if(EXPR_IS_MUTABLE(call_expr->value->id)){
                    ir_basicblock_new_instructions(builder->current_block, 1);
                    ir_instr_load_t *load_mutable = (ir_instr_load_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_load_t));
                    load_mutable->id = INSTRUCTION_LOAD;
                    load_mutable->result_type = (ir_type_t*) arg_values[0]->type->extra;
                    load_mutable->value = arg_values[0];
                    builder->current_block->instructions[builder->current_block->instructions_length++] = (ir_instr_t*) load_mutable;
                    arg_values[0] = build_value_from_prev_instruction(builder);
                }
            } else {
                char *s = ast_type_str(&arg_types[0]);
                compiler_panicf(builder->compiler, call_expr->source, "Can't call methods on type '%s'", s);
                ast_type_free(&arg_types[0]);
                free(arg_types);
                free(s);
                return 1;
            }

            const char *struct_name = ((ast_elem_base_t*) arg_types[0].elements[1])->base;

            for(length_t a = 0; a != call_expr->arity; a++){
                if(assemble_expression(builder, call_expr->args[a], &arg_values[a + 1], false, &arg_types[a + 1])){
                    ast_types_free_fully(arg_types, a + 1);
                    return 1;
                }
            }

            // Find function that fits given name and arguments
            funcpair_t pair;
            if(assemble_find_method_conforming(builder, struct_name, call_expr->name, arg_values, arg_types, call_expr->arity + 1, &pair)){
                compiler_panicf(builder->compiler, call_expr->source, "Undeclared function '%s'", call_expr->name);
                ast_types_free_fully(arg_types, call_expr->arity + 1);
                return 1;
            }

            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_call_t));
            ((ir_instr_call_t*) instruction)->id = INSTRUCTION_CALL;
            ((ir_instr_call_t*) instruction)->result_type = pair.ir_func->return_type;
            ((ir_instr_call_t*) instruction)->values = arg_values;
            ((ir_instr_call_t*) instruction)->values_length = call_expr->arity + 1;
            ((ir_instr_call_t*) instruction)->func_id = pair.func_id;
            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            ast_types_free_fully(arg_types, call_expr->arity + 1);
            if(out_expr_type != NULL) *out_expr_type = ast_type_clone(&pair.ast_func->return_type);
        }
        break;
    case EXPR_NOT: {
            ast_expr_not_t *not_expr = (ast_expr_not_t*) expr;
            ast_type_t expr_type;
            ir_value_t *expr_value;

            if(assemble_expression(builder, not_expr->value, &expr_value, false, &expr_type)) return 1;

            if(ir_type_get_catagory(expr_value->type) == 0){
                char *s = ast_type_str(&expr_type);
                compiler_panicf(builder->compiler, expr->source, "Can't use '!' operator on type '%s'", s);
                ast_type_free(&expr_type);
                free(s);
                return 1;
            }

            // Build and append a load instruction
            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_cast_t));
            ((ir_instr_cast_t*) instruction)->id = INSTRUCTION_ISZERO;
            ((ir_instr_cast_t*) instruction)->result_type = (ir_type_t*) ir_pool_alloc(builder->pool, sizeof(ir_type_t));
            ((ir_instr_cast_t*) instruction)->result_type->kind = TYPE_KIND_BOOLEAN;
            ((ir_instr_cast_t*) instruction)->value = expr_value;
            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            ast_type_free(&expr_type);
            if(out_expr_type != NULL) ast_type_make_base_newstr(out_expr_type, "bool");
        }
        break;
    case EXPR_NEW: {
            ir_type_t *ir_type;
            ast_type_t multiplier_type;
            ir_value_t *amount = NULL;
            if(assemble_resolve_type(builder->compiler, builder->object, &((ast_expr_new_t*) expr)->type, &ir_type)) return 1;

            if( ((ast_expr_new_t*) expr)->amount != NULL){
                if(assemble_expression(builder, ((ast_expr_new_t*) expr)->amount, &amount, false, &multiplier_type)) return 1;
                unsigned int multiplier_typekind = amount->type->kind;

                if(multiplier_typekind < TYPE_KIND_S8 || multiplier_typekind > TYPE_KIND_U64){
                    char *typename = ast_type_str(&multiplier_type);
                    compiler_panicf(builder->compiler, expr->source, "Can't specify length using non-integer type '%s'", typename);
                    ast_type_free(&multiplier_type);
                    free(typename);
                    return 1;
                }

                ast_type_free(&multiplier_type);
            }

            ir_basicblock_new_instructions(builder->current_block, 1);
            instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_malloc_t));
            ((ir_instr_malloc_t*) instruction)->id = INSTRUCTION_MALLOC;
            ((ir_instr_malloc_t*) instruction)->result_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
            ((ir_instr_malloc_t*) instruction)->result_type->kind = TYPE_KIND_POINTER;
            ((ir_instr_malloc_t*) instruction)->result_type->extra = ir_type;
            ((ir_instr_malloc_t*) instruction)->type = ir_type;
            ((ir_instr_malloc_t*) instruction)->amount = amount;

            builder->current_block->instructions[builder->current_block->instructions_length++] = instruction;
            *ir_value = build_value_from_prev_instruction(builder);

            if(out_expr_type != NULL){
                *out_expr_type = ast_type_clone(&((ast_expr_new_t*) expr)->type);
                ast_type_prepend_ptr(out_expr_type);
            }
        }
        break;
    default:
        compiler_panic(builder->compiler, expr->source, "Unknown expression type id in expression");
        return 1;
    }

    #undef BUILD_LITERAL_IR_VALUE
    #undef BUILD_MATH_OP_IvF_MACRO
    #undef BUILD_MATH_OP_UvSvF_MACRO

    return 0;
}

ir_instr_t* assemble_math_operands(ir_builder_t *builder, ast_expr_t *expr, ir_value_t **ir_value, unsigned int op_res, ast_type_t *out_expr_type){
    // Assemble the operends of an expression into instructions and gives back an instruction with unknown id

    // NOTE: Returns the generated instruction
    // NOTE: The instruction id still must be specified after this call
    // NOTE: Returns NULL if an error occured
    // NOTE:'op_res' should be either MATH_OP_RESULT_MATCH, MATH_OP_RESULT_BOOL or MATH_OP_ALL_BOOL

    ir_value_t *a, *b;
    ir_instr_t **instruction;
    ast_type_t ast_type_a, ast_type_b;
    *ir_value = ir_pool_alloc(builder->pool, sizeof(ir_value_t));
    (*ir_value)->value_type = VALUE_TYPE_RESULT;

    if(assemble_expression(builder, ((ast_expr_math_t*) expr)->a, &a, false, &ast_type_a)) return NULL;
    if(assemble_expression(builder, ((ast_expr_math_t*) expr)->b, &b, false, &ast_type_b)){
        ast_type_free(&ast_type_a);
        return NULL;
    }

    if(op_res == MATH_OP_ALL_BOOL){
        // Conform expression 'a' to type 'bool' to ensure 'b' will also have to conform
        // Use 'out_expr_type' to store bool type (will stay there anyways cause resulting type is a bool)
        ast_type_make_base_newstr(out_expr_type, "bool");

        if(!ast_types_identical(&ast_type_a, out_expr_type) && !ast_types_conform(builder, &a, &ast_type_a, out_expr_type, CONFORM_MODE_PRIMITIVES)){
            char *a_type_str = ast_type_str(&ast_type_a);
            compiler_panicf(builder->compiler, expr->source, "Failed to convert value of type '%s' to type 'bool'", a_type_str);
            free(a_type_str);
            ast_type_free(&ast_type_a);
            ast_type_free(&ast_type_b);
            ast_type_free(out_expr_type);
            return NULL;
        }

        if(!ast_types_identical(&ast_type_b, out_expr_type) && !ast_types_conform(builder, &b, &ast_type_b, out_expr_type, CONFORM_MODE_PRIMITIVES)){
            char *b_type_str = ast_type_str(&ast_type_b);
            compiler_panicf(builder->compiler, expr->source, "Failed to convert value of type '%s' to type 'bool'", b_type_str);
            free(b_type_str);
            ast_type_free(&ast_type_a);
            ast_type_free(&ast_type_b);
            ast_type_free(out_expr_type);
            return NULL;
        }

        ast_type_free(&ast_type_a);
        ast_type_free(&ast_type_b);
        ast_type_a = *out_expr_type;
        ast_type_b = *out_expr_type;
    }

    if(!ast_types_conform(builder, &b, &ast_type_b, &ast_type_a, CONFORM_MODE_PRIMITIVES)){
        char *a_type_str = ast_type_str(&ast_type_a);
        char *b_type_str = ast_type_str(&ast_type_b);
        compiler_panicf(builder->compiler, expr->source, "Incompatible types '%s' and '%s'", a_type_str, b_type_str);
        free(a_type_str);
        free(b_type_str);
        if(op_res != MATH_OP_ALL_BOOL){
            ast_type_free(&ast_type_a);
            ast_type_free(&ast_type_b);
        } else {
            ast_type_free(out_expr_type);
        }
        return NULL;
    }

    (*ir_value)->extra = ir_pool_alloc(builder->pool, sizeof(ir_value_result_t));
    ((ir_value_result_t*) (*ir_value)->extra)->block_id = builder->current_block_id;
    ((ir_value_result_t*) (*ir_value)->extra)->instruction_id = builder->current_block->instructions_length;

    ir_basicblock_new_instructions(builder->current_block, 1);
    instruction = &builder->current_block->instructions[builder->current_block->instructions_length++];
    *instruction = (ir_instr_t*) ir_pool_alloc(builder->pool, sizeof(ir_instr_math_t));
    ((ir_instr_math_t*) *instruction)->a = a;
    ((ir_instr_math_t*) *instruction)->b = b;
    ((ir_instr_math_t*) *instruction)->id = INSTRUCTION_NONE; // For safety

    switch(op_res){
    case MATH_OP_RESULT_MATCH:
        if(out_expr_type != NULL) *out_expr_type = ast_type_clone(&ast_type_a);
        ((ir_instr_math_t*) *instruction)->result_type = a->type;
        (*ir_value)->type = a->type;
        break;
    case MATH_OP_RESULT_BOOL:
        if(out_expr_type != NULL) ast_type_make_base_newstr(out_expr_type, "bool");
        // Fall through to MATH_OP_ALL_BOOL
    case MATH_OP_ALL_BOOL:
        ((ir_instr_math_t*) *instruction)->result_type = (ir_type_t*) ir_pool_alloc(builder->pool, sizeof(ir_type_t));
        ((ir_instr_math_t*) *instruction)->result_type->kind = TYPE_KIND_BOOLEAN;
        (*ir_value)->type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
        (*ir_value)->type->kind = TYPE_KIND_BOOLEAN;
        // Assigning result_type->extra to null is unnecessary
        break;
    default:
        compiler_panic(builder->compiler, expr->source, "assemble_math_operands() got invalid op_res value");
        if(op_res != MATH_OP_ALL_BOOL){
            ast_type_free(&ast_type_a);
            ast_type_free(&ast_type_b);
        } else {
            ast_type_free(out_expr_type);
        }
        return NULL;
    }

    if(op_res != MATH_OP_ALL_BOOL){
        ast_type_free(&ast_type_a);
        ast_type_free(&ast_type_b);
    }

    return *instruction;
}

int i_vs_f_instruction(ir_instr_math_t *instruction, unsigned int i_instr, unsigned int f_instr){
    // NOTE: Sets the instruction id to 'i_instr' if operating on intergers
    //       Sets the instruction id to 'f_instr' if operating on floats
    // NOTE: If target instruction id is INSTRUCTION_NONE, then 1 is returned
    // NOTE: Returns 1 if unsupported type

    switch(instruction->a->type->kind){
    case TYPE_KIND_POINTER: case TYPE_KIND_BOOLEAN:
    case TYPE_KIND_U8: case TYPE_KIND_U16: case TYPE_KIND_U32: case TYPE_KIND_U64:
    case TYPE_KIND_S8: case TYPE_KIND_S16: case TYPE_KIND_S32: case TYPE_KIND_S64:
        if(i_instr == INSTRUCTION_NONE) return 1;
        instruction->id = i_instr; break;
    case TYPE_KIND_HALF: case TYPE_KIND_FLOAT: case TYPE_KIND_DOUBLE:
        if(f_instr == INSTRUCTION_NONE) return 1;
        instruction->id = f_instr; break;
    default: return 1;
    }
    return 0;
}

int u_vs_s_vs_float_instruction(ir_instr_math_t *instruction, unsigned int u_instr, unsigned int s_instr, unsigned int f_instr){
    // NOTE: Sets the instruction id to 'u_instr' if operating on unsigned intergers
    //       Sets the instruction id to 's_instr' if operating on signed intergers
    //       Sets the instruction id to 'f_instr' if operating on floats
    // NOTE: If target instruction id is INSTRUCTION_NONE, then 1 is returned
    // NOTE: Returns 1 if unsupported type

    switch(instruction->a->type->kind){
    case TYPE_KIND_POINTER: case TYPE_KIND_BOOLEAN:
    case TYPE_KIND_U8: case TYPE_KIND_U16: case TYPE_KIND_U32: case TYPE_KIND_U64:
        if(u_instr == INSTRUCTION_NONE) return 1;
        instruction->id = u_instr; break;
    case TYPE_KIND_S8: case TYPE_KIND_S16: case TYPE_KIND_S32: case TYPE_KIND_S64:
        if(s_instr == INSTRUCTION_NONE) return 1;
        instruction->id = s_instr; break;
    case TYPE_KIND_HALF: case TYPE_KIND_FLOAT: case TYPE_KIND_DOUBLE:
        if(f_instr == INSTRUCTION_NONE) return 1;
        instruction->id = f_instr; break;
    default: return 1;
    }
    return 0;
}

char ir_type_get_catagory(ir_type_t *type){
    switch(type->kind){
    case TYPE_KIND_POINTER: case TYPE_KIND_BOOLEAN:
    case TYPE_KIND_U8: case TYPE_KIND_U16: case TYPE_KIND_U32: case TYPE_KIND_U64:
        return PRIMITIVE_UI;
    case TYPE_KIND_S8: case TYPE_KIND_S16: case TYPE_KIND_S32: case TYPE_KIND_S64:
        return PRIMITIVE_SI;
    case TYPE_KIND_HALF: case TYPE_KIND_FLOAT: case TYPE_KIND_DOUBLE:
        return PRIMITIVE_FP;
    }
    return PRIMITIVE_NA; // No catagory
}