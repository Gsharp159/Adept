
#include "UTIL/color.h"
#include "UTIL/ground.h"
#include "UTIL/search.h"
#include "UTIL/filename.h"
#include "UTIL/builtin_type.h"
#include "BRIDGE/any.h"
#include "BRIDGE/rtti.h"
#include "IRGEN/ir_gen.h"
#include "IRGEN/ir_gen_expr.h"
#include "IRGEN/ir_gen_stmt.h"
#include "IRGEN/ir_gen_type.h"

errorcode_t ir_gen(compiler_t *compiler, object_t *object){
    ir_module_t *module = &object->ir_module;
    ast_t *ast = &object->ast;

    ir_module_init(module, ast->funcs_length, ast->globals_length);
    object->compilation_stage = COMPILATION_STAGE_IR_MODULE;

    if(ir_gen_type_mappings(compiler, object)
    || ir_gen_globals(compiler, object)
    || ir_gen_functions(compiler, object)
    || ir_gen_functions_body(compiler, object)) return FAILURE;

    return SUCCESS;
}

errorcode_t ir_gen_functions(compiler_t *compiler, object_t *object){
    // NOTE: Only ir_gens function skeletons

    ast_t *ast = &object->ast;
    ir_module_t *module = &object->ir_module;
    ast_func_t *ast_funcs;
    ir_func_t *module_funcs;
    length_t ast_funcs_length;
    length_t *module_funcs_length;

    ast_funcs = ast->funcs;
    module_funcs = module->funcs;
    ast_funcs_length = ast->funcs_length;
    module_funcs_length = &module->funcs_length;
    module->func_mappings = malloc(sizeof(ir_func_mapping_t) * ast->funcs_length);

    for(length_t f = 0; f != ast_funcs_length; f++){
        ast_func_t *ast_func = &ast_funcs[f];
        ir_func_t *module_func = &module_funcs[f];

        module_func->name = ast_func->name;
        module_func->traits = TRAIT_NONE;
        module_func->return_type = NULL;
        module_func->argument_types = malloc(sizeof(ir_type_t*) * ast_func->arity);
        module_func->arity = 0;
        module_func->basicblocks = NULL; // Will be set after 'basicblocks' contains all of the basicblocks
        module_func->basicblocks_length = 0; // Will be set after 'basicblocks' contains all of the basicblocks
        module_func->var_scope = NULL;
        module_func->variable_count = 0;
        module->func_mappings[f].name = ast_func->name;
        module->func_mappings[f].ast_func = ast_func;
        module->func_mappings[f].module_func = module_func;
        module->func_mappings[f].func_id = f;
        module->func_mappings[f].is_beginning_of_group = -1;
        (*module_funcs_length)++;

        if(ast_func->traits & AST_FUNC_FOREIGN) module_func->traits |= IR_FUNC_FOREIGN;
        if(ast_func->traits & AST_FUNC_VARARG)  module_func->traits |= IR_FUNC_VARARG;
        if(ast_func->traits & AST_FUNC_STDCALL) module_func->traits |= IR_FUNC_STDCALL;

        if(!(ast_func->traits & AST_FUNC_FOREIGN)){
            if(ast_func->arity > 0 && strcmp(ast_func->arg_names[0], "this") == 0){
                // This is a struct method, attach a reference to the struct
                ast_type_t *this_type = &ast_func->arg_types[0];

                // Do basic checking to make sure the type is in the format: *Structure
                if(this_type->elements_length != 2 || this_type->elements[0]->id != AST_ELEM_POINTER
                        || this_type->elements[1]->id != AST_ELEM_BASE){
                    compiler_panic(compiler, this_type->source, "Type of 'this' must be a pointer to a struct");
                    return FAILURE;
                }

                // Check that the base isn't a primitive
                char *base = ((ast_elem_base_t*) this_type->elements[1])->base;
                if(typename_builtin_type(base) != BUILTIN_TYPE_NONE){
                    compiler_panicf(compiler, this_type->source, "Type of 'this' must be a pointer to a struct (%s is a primitive)", base);
                    return FAILURE;
                }

                // Find the target structure
                ast_struct_t *target = ast_struct_find(ast, ((ast_elem_base_t*) this_type->elements[1])->base);
                if(target == NULL){
                    compiler_panicf(compiler, this_type->source, "Undeclared struct '%s'", ((ast_elem_base_t*) this_type->elements[1])->base);
                    return FAILURE;
                }

                // Append the method to the struct's method list
                if(module->methods == NULL){
                    module->methods = malloc(sizeof(ir_method_t) * 4);
                    module->methods_length = 0;
                    module->methods_capacity = 4;
                } else if(module->methods_length == module->methods_capacity){
                    module->methods_capacity *= 2;
                    ir_method_t *new_methods = malloc(sizeof(ir_method_t) * module->methods_capacity);
                    memcpy(new_methods, module->methods, sizeof(ir_method_t) * module->methods_length);
                    free(module->methods);
                    module->methods = new_methods;
                }

                ir_method_t *method = &module->methods[module->methods_length++];
                method->struct_name = ((ast_elem_base_t*) this_type->elements[1])->base;
                method->name = module_func->name;
                method->ast_func = ast_func;
                method->module_func = module_func;
                method->func_id = f;
                method->is_beginning_of_group = -1;
            }
        } else {
            while(module_func->arity != ast_func->arity){
                if(ir_gen_resolve_type(compiler, object, &ast_func->arg_types[module_func->arity], &module_func->argument_types[module_func->arity])) return FAILURE;
                module_func->arity++;
            }
        }

        if(ast_func->traits & AST_FUNC_MAIN && ast_type_is_void(&ast_func->return_type)){
            // If it's the main function and returns void, return int under the hood
            module_func->return_type = ir_pool_alloc(&module->pool, sizeof(ir_type_t));
            module_func->return_type->kind = TYPE_KIND_S32;
            // neglect 'module_func->return_type->extra'
        } else {
            if(ir_gen_resolve_type(compiler, object, &ast_func->return_type, &module_func->return_type)) return FAILURE;
        }
    }

    qsort(module->func_mappings, ast->funcs_length, sizeof(ir_func_mapping_t), ir_func_mapping_cmp);
    qsort(module->methods, module->methods_length, sizeof(ir_method_t), ir_method_cmp);
    return SUCCESS;
}

errorcode_t ir_gen_functions_body(compiler_t *compiler, object_t *object){
    // NOTE: Only ir_gens function body; assumes skeleton already exists

    ast_func_t *ast_funcs = object->ast.funcs;
    ir_func_t *module_funcs = object->ir_module.funcs;
    length_t ast_funcs_length = object->ast.funcs_length;

    for(length_t f = 0; f != ast_funcs_length; f++){
        if(ast_funcs[f].traits & AST_FUNC_FOREIGN) continue;
        if(ir_gen_func_statements(compiler, object, &ast_funcs[f], &module_funcs[f])) return FAILURE;
    }

    return SUCCESS;
}

errorcode_t ir_gen_globals(compiler_t *compiler, object_t *object){
    ast_t *ast = &object->ast;
    ir_module_t *module = &object->ir_module;

    for(length_t g = 0; g != ast->globals_length; g++){
        module->globals[g].name = ast->globals[g].name;
        module->globals[g].traits = ast->globals[g].traits & AST_GLOBAL_EXTERNAL ? IR_GLOBAL_EXTERNAL : TRAIT_NONE;

        if(ir_gen_resolve_type(compiler, object, &ast->globals[g].type, &module->globals[g].type)){
            return FAILURE;
        }

        module->globals_length++;
    }

    return SUCCESS;
}

errorcode_t ir_gen_globals_init(ir_builder_t *builder){
    // Generates instructions for initializing global variables
    ast_global_t *globals = builder->object->ast.globals;
    length_t globals_length = builder->object->ast.globals_length;

    for(length_t g = 0; g != globals_length; g++){
        ast_global_t *ast_global = &globals[g];

        ir_value_t *value;
        ast_type_t value_ast_type;

        if(ast_global->initial == NULL){
            if(!(ast_global->traits & AST_GLOBAL_SPECIAL)) continue;

            // Special global variable
            if(ir_gen_special_global(builder, ast_global, g)) return 1;
            continue;
        }

        if(ir_gen_expression(builder, ast_global->initial, &value, false, &value_ast_type)) return FAILURE;

        if(!ast_types_conform(builder, &value, &value_ast_type, &ast_global->type, CONFORM_MODE_PRIMITIVES)){
            char *a_type_str = ast_type_str(&value_ast_type);
            char *b_type_str = ast_type_str(&ast_global->type);
            compiler_panicf(builder->compiler, ast_global->initial->source, "Incompatible types '%s' and '%s'", a_type_str, b_type_str);
            free(a_type_str);
            free(b_type_str);
            ast_type_free(&value_ast_type);
            return FAILURE;
        }

        ast_type_free(&value_ast_type);

        ir_type_t *ptr_to_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
        ptr_to_type->kind = TYPE_KIND_POINTER;
        ptr_to_type->extra = builder->object->ir_module.globals[g].type;

        ir_value_t *destination = build_gvarptr(builder, ptr_to_type, g);
        build_store(builder, value, destination);
    }
    return SUCCESS;
}

errorcode_t ir_gen_special_global(ir_builder_t *builder, ast_global_t *ast_global, length_t global_variable_id){
    // NOTE: Assumes (ast_global->traits & AST_GLOBAL_SPECIAL)

    ir_type_t *ptr_to_type = ir_pool_alloc(builder->pool, sizeof(ir_type_t));
    ptr_to_type->kind = TYPE_KIND_POINTER;
    ptr_to_type->extra = &ast_global->type;

    ir_value_t *destination = build_gvarptr(builder, ptr_to_type, global_variable_id);

    if(ast_global->traits & AST_GLOBAL___TYPES__){
        ir_type_t *any_type_type, *any_struct_type_type, *any_ptr_type_type,
            *any_type_ptr_type = NULL, *ubyte_ptr_type;
        
        if(builder->compiler->traits & COMPILER_NO_TYPE_INFO){
            if(!ir_type_map_find(builder->type_map, "AnyType", &any_type_type)){
                redprintf("INTERNAL ERROR: Failed to get 'AnyType' which should've been injected");
                redprintf("    (when creating null pointer to initialize __types__ because type info was disabled)")
                return FAILURE;
            }

            any_type_ptr_type = ir_type_pointer_to(builder->pool, any_type_type);
            build_store(builder, build_null_pointer_of_type(builder->pool, ir_type_pointer_to(builder->pool, any_type_ptr_type)), destination);
            return SUCCESS;
        }

        type_table_t *table = builder->object->ast.type_table;
        type_table_reduce(table);

        if(!ir_type_map_find(builder->type_map, "AnyType", &any_type_type)
        || !ir_type_map_find(builder->type_map, "AnyStructType", &any_struct_type_type)
        || !ir_type_map_find(builder->type_map, "AnyPtrType", &any_ptr_type_type)
        || !ir_type_map_find(builder->type_map, "ubyte", &ubyte_ptr_type)){
            redprintf("INTERNAL ERROR: Failed to find types used by the runtime type table that should've been injected\n");
            return FAILURE;
        }

        ir_type_t *usize_type = ir_builder_usize(builder);
        any_type_ptr_type = ir_type_pointer_to(builder->pool, any_type_type);
        ubyte_ptr_type = ir_type_pointer_to(builder->pool, ubyte_ptr_type);

        ir_value_array_literal_t *array_literal = ir_pool_alloc(builder->pool, sizeof(ir_value_array_literal_t));
        array_literal->values = NULL; // Will be set to array_values
        array_literal->length = table->length;

        ir_value_t *array_value = ir_pool_alloc(builder->pool, sizeof(ir_value_t));
        array_value->value_type = VALUE_TYPE_ARRAY_LITERAL;
        array_value->type = ir_type_pointer_to(builder->pool, any_type_ptr_type);
        array_value->extra = array_literal;
        
        ir_value_t **array_values = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * table->length);

        for(length_t i = 0; i != table->length; i++){
            if(ir_gen_resolve_type(builder->compiler, builder->object, &table->records[i].ast_type, &table->records[i].ir_type)){
                return FAILURE;
            }

            switch(table->records[i].ir_type->kind){
            case TYPE_KIND_POINTER:
                array_values[i] = build_anon_global(&builder->object->ir_module, any_ptr_type_type, true);
                break;
            case TYPE_KIND_STRUCTURE:
                array_values[i] = build_anon_global(&builder->object->ir_module, any_struct_type_type, true);
                break;
            default:
                array_values[i] = build_anon_global(&builder->object->ir_module, any_type_type, true);
            }
        }

        for(length_t i = 0; i != table->length; i++){
            ir_type_t *initializer_type;
            ir_value_t **initializer_members;
            length_t initializer_members_length;
            unsigned int any_type_kind_id = ANY_TYPE_KIND_VOID;
            unsigned int type_type_kind = table->records[i].ir_type->kind;

            switch(type_type_kind){
            case TYPE_KIND_POINTER: {
                    /* struct AnyPtrType(kind AnyTypeKind, name *ubyte, is_alias bool, subtype *AnyType) */

                    initializer_members = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * 4);
                    initializer_members_length = 4;

                    maybe_index_t subtype_index = -1;

                    // HACK: We really shouldn't be doing this
                    if(table->records[i].ast_type.elements_length > 1){
                        ast_type_t dereferenced = ast_type_clone(&table->records[i].ast_type);

                        // Modify ast_type_t to remove a pointer element from the front
                        // DANGEROUS: Manually deleting ast_elem_pointer_t
                        free(dereferenced.elements[0]);
                        memmove(dereferenced.elements, &dereferenced.elements[1], sizeof(ast_elem_t*) * (dereferenced.elements_length - 1));
                        dereferenced.elements_length--; // Reduce length accordingly

                        char *dereferenced_name = ast_type_str(&dereferenced);
                        subtype_index = type_table_find(table, dereferenced_name);
                        ast_type_free(&dereferenced);
                        free(dereferenced_name);
                    }

                    if(subtype_index == -1){
                        ir_value_t *null_pointer = build_null_pointer_of_type(builder->pool, any_type_ptr_type);
                        initializer_members[3] = null_pointer; // subtype
                    } else {
                        initializer_members[3] = build_const_bitcast(builder->pool, array_values[subtype_index], any_type_ptr_type); // subtype
                    }

                    initializer_type = any_ptr_type_type;
                }
                break;
            case TYPE_KIND_STRUCTURE: {
                    /* struct AnyStructType (kind AnyTypeKind, name *ubyte, members **AnyType, length usize, offsets *usize, member_names **ubyte, is_packed bool) */

                    ir_type_extra_composite_t *composite = (ir_type_extra_composite_t*) table->records[i].ir_type->extra;
                    initializer_members = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * 8);
                    initializer_members_length = 8;

                    ir_value_t **composite_members = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * composite->subtypes_length);
                    ir_value_t **composite_offsets = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * composite->subtypes_length);
                    ir_value_t **composite_member_names = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * composite->subtypes_length);

                    const char *struct_name = ((ast_elem_base_t*) table->records[i].ast_type.elements[0])->base;
                    ast_struct_t *structure = ast_struct_find(&builder->object->ast, struct_name);

                    if(structure == NULL){
                        redprintf("INTERNAL ERROR: Failed to find struct '%s' that should exist when generating runtime type table!\n", struct_name);
                        return FAILURE;
                    }

                    if(structure->field_count != composite->subtypes_length){
                        redprintf("INTERNAL ERROR: Mismatching member count of IR as AST types for struct '%s' when generating runtime type table!\n", struct_name);
                        return FAILURE;
                    }

                    for(length_t s = 0; s != composite->subtypes_length; s++){
                        char *member_type_name = ast_type_str(&structure->field_types[s]);
                        maybe_index_t subtype_index = type_table_find(table, member_type_name);
                        free(member_type_name);

                        if(subtype_index == -1){
                            composite_members[s] = build_null_pointer(builder->pool); // members[s]
                        } else {
                            composite_members[s] = build_const_bitcast(builder->pool, array_values[subtype_index], any_type_ptr_type); // members[s]
                        }

                        composite_offsets[s] = build_literal_usize(builder->pool, 0);
                        composite_member_names[s] = build_literal_cstr(builder, structure->field_names[s]);
                    }

                    ir_value_t *members_array = build_static_array(builder->pool, any_type_ptr_type, composite_members, composite->subtypes_length);
                    ir_value_t *offsets_array = build_static_array(builder->pool, usize_type, composite_offsets, composite->subtypes_length);
                    ir_value_t *member_names_array = build_static_array(builder->pool, ubyte_ptr_type, composite_member_names, composite->subtypes_length);

                    initializer_members[3] = members_array;
                    initializer_members[4] = build_literal_usize(builder->pool, composite->subtypes_length); // length
                    initializer_members[5] = offsets_array;
                    initializer_members[6] = member_names_array;
                    initializer_members[7] = build_bool(builder->pool, composite->traits & TYPE_KIND_COMPOSITE_PACKED); // is_packed
                    initializer_type = any_struct_type_type;
                }
                break;
            default:
                initializer_members = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * 3);
                initializer_members_length = 3;
                initializer_type = any_type_type;
            }

            switch(type_type_kind){
            case TYPE_KIND_NONE:        any_type_kind_id = ANY_TYPE_KIND_VOID; break;
            case TYPE_KIND_POINTER:     any_type_kind_id = ANY_TYPE_KIND_PTR; break;
            case TYPE_KIND_S8:          any_type_kind_id = ANY_TYPE_KIND_BYTE; break;
            case TYPE_KIND_S16:         any_type_kind_id = ANY_TYPE_KIND_SHORT; break;
            case TYPE_KIND_S32:         any_type_kind_id = ANY_TYPE_KIND_INT; break;
            case TYPE_KIND_S64:         any_type_kind_id = ANY_TYPE_KIND_LONG; break;
            case TYPE_KIND_U8:          any_type_kind_id = ANY_TYPE_KIND_UBYTE; break;
            case TYPE_KIND_U16:         any_type_kind_id = ANY_TYPE_KIND_USHORT; break;
            case TYPE_KIND_U32:         any_type_kind_id = ANY_TYPE_KIND_UINT; break;
            case TYPE_KIND_U64:         any_type_kind_id = ANY_TYPE_KIND_ULONG; break;
            case TYPE_KIND_FLOAT:       any_type_kind_id = ANY_TYPE_KIND_FLOAT; break;
            case TYPE_KIND_DOUBLE:      any_type_kind_id = ANY_TYPE_KIND_DOUBLE; break;
            case TYPE_KIND_BOOLEAN:     any_type_kind_id = ANY_TYPE_KIND_BOOL; break;
            case TYPE_KIND_STRUCTURE:   any_type_kind_id = ANY_TYPE_KIND_STRUCT; break;
            case TYPE_KIND_FUNCPTR:     any_type_kind_id = ANY_TYPE_KIND_FUNC_PTR; break;
            case TYPE_KIND_FIXED_ARRAY: any_type_kind_id = ANY_TYPE_KIND_FIXED_ARRAY; break;

            // Unsupported Type Kinds
            case TYPE_KIND_HALF:        any_type_kind_id = ANY_TYPE_KIND_USHORT; break;
            // case TYPE_KIND_UNION: ignored
            }

            initializer_members[0] = build_literal_usize(builder->pool, any_type_kind_id); // kind
            initializer_members[1] = build_literal_cstr(builder, table->records[i].name); // name
            initializer_members[2] = build_bool(builder->pool, table->records[i].is_alias); // is_alias

            ir_value_t *initializer = build_static_struct(&builder->object->ir_module, initializer_type, initializer_members, initializer_members_length, false);
            build_anon_global_initializer(&builder->object->ir_module, array_values[i], initializer);

            if(initializer_type != any_type_ptr_type){
                array_values[i] = build_const_bitcast(builder->pool, array_values[i], any_type_ptr_type);
            }
        }

        array_literal->values = array_values;
        build_store(builder, array_value, destination);
        return SUCCESS;
    }

    if(ast_global->traits & AST_GLOBAL___TYPES_LENGTH__){
        if(builder->compiler->traits & COMPILER_NO_TYPE_INFO){
            build_store(builder, build_literal_usize(builder->pool, 0), destination);
            return SUCCESS;
        }

        // Reduce type table if haven't
        type_table_t *table = builder->object->ast.type_table;
        type_table_reduce(table);

        ir_value_t *value = build_literal_usize(builder->pool, table->length);
        build_store(builder, value, destination);
        return SUCCESS;
    }

    if(ast_global->traits & AST_GLOBAL___TYPE_KINDS__){
        ir_type_t *ubyte_ptr_type, *ubyte_ptr_ptr_type;

        if(!ir_type_map_find(builder->type_map, "ubyte", &ubyte_ptr_type)){
            redprintf("INTERNAL ERROR: Failed to find types used by the runtime type table that should've been injected\n");
            return FAILURE;
        }

        // Construct IR Types we need
        ubyte_ptr_type = ir_type_pointer_to(builder->pool, ubyte_ptr_type);
        ubyte_ptr_ptr_type = ir_type_pointer_to(builder->pool, ubyte_ptr_type);

        if(builder->compiler->traits & COMPILER_NO_TYPE_INFO){
            build_store(builder, build_null_pointer_of_type(builder->pool, ubyte_ptr_ptr_type), destination);
            return SUCCESS;
        }

        ir_value_array_literal_t *kinds_array_literal = ir_pool_alloc(builder->pool, sizeof(ir_value_array_literal_t));
        kinds_array_literal->values = NULL; // Will be set to array_values
        kinds_array_literal->length = MAX_ANY_TYPE_KIND + 1;

        ir_value_t *kinds_array_value = ir_pool_alloc(builder->pool, sizeof(ir_value_t));
        kinds_array_value->value_type = VALUE_TYPE_ARRAY_LITERAL;
        kinds_array_value->type = ir_type_pointer_to(builder->pool, ubyte_ptr_type);
        kinds_array_value->extra = kinds_array_literal;
        
        ir_value_t **array_values = ir_pool_alloc(builder->pool, sizeof(ir_value_t*) * (MAX_ANY_TYPE_KIND + 1));

        for(length_t i = 0; i != MAX_ANY_TYPE_KIND + 1; i++){
            array_values[i] = build_literal_cstr(builder, (weak_cstr_t) any_type_kind_names[i]);
        }

        kinds_array_literal->values = array_values;
        build_store(builder, kinds_array_value, destination);
        return SUCCESS;
    }

    if(ast_global->traits & AST_GLOBAL___TYPE_KINDS_LENGTH__){
        if(builder->compiler->traits & COMPILER_NO_TYPE_INFO){
            build_store(builder, build_literal_usize(builder->pool, 0), destination);
            return SUCCESS;
        }

        ir_value_t *value = build_literal_usize(builder->pool, MAX_ANY_TYPE_KIND + 1);
        build_store(builder, value, destination);
        return SUCCESS;
    }

    // Should never reach
    redprintf("INTERNAL ERROR: Encountered unknown special global variable '%s'!\n", ast_global->name);
    return FAILURE;
}

int ir_func_mapping_cmp(const void *a, const void *b){
    int diff = strcmp(((ir_func_mapping_t*) a)->module_func->name, ((ir_func_mapping_t*) b)->module_func->name);
    if(diff != 0) return diff;
    return (int) ((ir_func_mapping_t*) a)->func_id - (int) ((ir_func_mapping_t*) b)->func_id;
}

int ir_method_cmp(const void *a, const void *b){
    int diff = strcmp(((ir_method_t*) a)->struct_name, ((ir_method_t*) b)->struct_name);
    if(diff != 0) return diff;
    diff = strcmp(((ir_method_t*) a)->name, ((ir_method_t*) b)->name);
    if(diff != 0) return diff;
    return (int) ((ir_method_t*) a)->func_id - (int) ((ir_method_t*) b)->func_id;
}
