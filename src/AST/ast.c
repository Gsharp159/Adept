
#include "AST/ast.h"
#include "UTIL/util.h"

void ast_init(ast_t *ast){
    ast->funcs = malloc(sizeof(ast_func_t) * 8);
    ast->funcs_length = 0;
    ast->funcs_capacity = 8;
    ast->structs = malloc(sizeof(ast_struct_t) * 4);
    ast->structs_length = 0;
    ast->structs_capacity = 4;
    ast->aliases = malloc(sizeof(ast_alias_t) * 8);
    ast->aliases_length = 0;
    ast->aliases_capacity = 8;
    ast->constants = NULL;
    ast->constants_length = 0;
    ast->constants_capacity = 0;
    ast->globals = NULL;
    ast->globals_length = 0;
    ast->globals_capacity = 0;
    ast->enums = NULL;
    ast->enums_length = 0;
    ast->enums_capacity = 0;
    ast->libraries = NULL;
    ast->libraries_are_framework = NULL;
    ast->libraries_length = 0;
    ast->libraries_capacity = 0;
    ast->common.ast_usize_type = NULL;

    ast->type_table = NULL;

    ast->meta_definitions = NULL;
    ast->meta_definitions_length = 0;
    ast->meta_definitions_capacity = 0;

    // Add relevant standard meta definitions
    #ifdef _WIN32
    meta_definition_add_bool(&ast->meta_definitions, &ast->meta_definitions_length, &ast->meta_definitions_capacity, "__windows__", true);
    #endif

    #if defined(__APPLE__) || defined(__MACH__)
    meta_definition_add_bool(&ast->meta_definitions, &ast->meta_definitions_length, &ast->meta_definitions_capacity, "__macos__", true);
    #endif

    #if defined(__unix__) || defined(__unix) || defined(unix)
    meta_definition_add_bool(&ast->meta_definitions, &ast->meta_definitions_length, &ast->meta_definitions_capacity, "__unix__", true);
    #endif

    #if defined(__linux__) || defined(__linux) || defined(linux)
    meta_definition_add_bool(&ast->meta_definitions, &ast->meta_definitions_length, &ast->meta_definitions_capacity, "__linux__", true);
    #endif
}

void ast_free(ast_t *ast){
    length_t i;

    ast_free_enums(ast->enums, ast->enums_length);
    ast_free_functions(ast->funcs, ast->funcs_length);
    ast_free_structs(ast->structs, ast->structs_length);
    ast_free_globals(ast->globals, ast->globals_length);
    ast_free_constants(ast->constants, ast->constants_length);

    for(i = 0; i != ast->aliases_length; i++){
        // Delete stuff within alias
        ast_type_free(&ast->aliases[i].type);
    }

    for(length_t l = 0; l != ast->libraries_length; l++){
        free(ast->libraries[l]);
    }

    free(ast->enums);
    free(ast->funcs);
    free(ast->structs);
    free(ast->constants);
    free(ast->globals);
    free(ast->aliases);
    free(ast->libraries);
    free(ast->libraries_are_framework);

    if(ast->common.ast_usize_type != NULL){
        ast_type_free_fully(ast->common.ast_usize_type);
    }

    type_table_free(ast->type_table);
    free(ast->type_table);

    for(i = 0; i != ast->meta_definitions_length; i++){
        meta_expr_free_fully(ast->meta_definitions[i].value);
    }
    free(ast->meta_definitions);
}

void ast_free_functions(ast_func_t *functions, length_t functions_length){
    for(length_t i = 0; i != functions_length; i++){
        ast_func_t *func = &functions[i];
        free(func->name);
 
        if(func->traits & AST_FUNC_FOREIGN){
            for(length_t a = 0; a != func->arity; a++){
                ast_type_free(&func->arg_types[a]);
            }
        } else {
            for(length_t a = 0; a != func->arity; a++){
                free(func->arg_names[a]);
                ast_type_free(&func->arg_types[a]);
            }
        }

        free(func->arg_names);
        free(func->arg_types);
        free(func->arg_sources);
        free(func->arg_flows);
        free(func->arg_type_traits);
        ast_free_statements(func->statements, func->statements_length);
        free(func->statements);
        ast_type_free(&func->return_type);
    }
}

void ast_free_statements(ast_expr_t **statements, length_t length){
    for(length_t s = 0; s != length; s++){
        ast_expr_free(statements[s]); // Free statement-specific memory
        free(statements[s]); // Free statement memory
    }
}

void ast_free_statements_fully(ast_expr_t **statements, length_t length){
    ast_free_statements(statements, length);
    free(statements);
}

void ast_free_structs(ast_struct_t *structs, length_t structs_length){
    for(length_t i = 0; i != structs_length; i++){
        free(structs[i].name);
        for(length_t f = 0; f != structs[i].field_count; f++){
            free(structs[i].field_names[f]);
            ast_type_free(&structs[i].field_types[f]);
        }
        free(structs[i].field_names);
        free(structs[i].field_types);
    }
}

void ast_free_constants(ast_constant_t *constants, length_t constants_length){
    for(length_t i = 0; i != constants_length; i++){
        ast_constant_t *constant = &constants[i];
        ast_expr_free_fully(constant->expression);
    }
}

void ast_free_globals(ast_global_t *globals, length_t globals_length){
    for(length_t i = 0; i != globals_length; i++){
        ast_global_t *global = &globals[i];
        ast_type_free(&global->type);
        if(global->initial != NULL){
            ast_expr_free_fully(global->initial);
        }
    }
}

void ast_free_enums(ast_enum_t *enums, length_t enums_length){
    for(length_t i = 0; i != enums_length; i++){
        ast_enum_t *inum = &enums[i];
        free(inum->kinds);
    }
}

void ast_dump(ast_t *ast, const char *filename){
    FILE *file = fopen(filename, "w");
    length_t i;

    if(file == NULL){
        printf("INTERNAL ERROR: Failed to open ast dump file\n");
        return;
    }

    ast_dump_enums(file, ast->enums, ast->enums_length);
    ast_dump_structs(file, ast->structs, ast->structs_length);
    ast_dump_globals(file, ast->globals, ast->globals_length);
    ast_dump_functions(file, ast->funcs, ast->funcs_length);

    for(i = 0; i != ast->aliases_length; i++){
        // Dump stuff within alias
    }

    for(i = 0; i != ast->libraries_length; i++){
        fprintf(file, "foreign '%s'\n", ast->libraries[i]);
    }

    fclose(file);
}

void ast_dump_functions(FILE *file, ast_func_t *functions, length_t functions_length){
    for(length_t i = 0; i != functions_length; i++){
        char *arguments_string = malloc(256);
        length_t arguments_string_length = 0;
        length_t arguments_string_capacity = 256;
        ast_func_t *func = &functions[i];
        arguments_string[0] = '\0';

        for(length_t a = 0; a != func->arity; a++){
            char *type_string = ast_type_str(&func->arg_types[a]);
            length_t name_length = (func->arg_names == NULL) ? 0 : strlen(func->arg_names[a]);
            length_t type_length = strlen(type_string);

            while(arguments_string_length + name_length + type_length + 3 >= arguments_string_capacity){
                char *new_arguments_string = malloc(arguments_string_capacity * 2);
                memcpy(new_arguments_string, arguments_string, arguments_string_length + 1);
                free(arguments_string);
                arguments_string = new_arguments_string;
                arguments_string_capacity *= 2;
            }

            if(func->traits & AST_FUNC_FOREIGN){
                if(a + 1 == func->arity){
                    memcpy(&arguments_string[arguments_string_length], type_string, type_length + 1);
                    arguments_string_length += type_length;
                } else {
                    memcpy(&arguments_string[arguments_string_length], type_string, type_length);
                    memcpy(&arguments_string[arguments_string_length + type_length], ", ", 3);
                    arguments_string_length += type_length + 2;
                }
            } else {
                memcpy(&arguments_string[arguments_string_length], func->arg_names[a], name_length);
                memcpy(&arguments_string[arguments_string_length + name_length], " ", 1);

                if(a + 1 == func->arity){
                    memcpy(&arguments_string[arguments_string_length + name_length + 1], type_string, type_length + 1);
                    arguments_string_length += name_length + type_length + 1;
                } else {
                    memcpy(&arguments_string[arguments_string_length + name_length + 1], type_string, type_length);
                    memcpy(&arguments_string[arguments_string_length + name_length + 1 + type_length], ", ", 3);
                    arguments_string_length += name_length + type_length + 3;
                }
            }

            free(type_string);
        }

        if(func->traits & AST_FUNC_VARARG){
            if(arguments_string_length + 6 >= arguments_string_capacity){
                arguments_string_capacity += 6;
                char *new_arguments_string = malloc(arguments_string_capacity);
                memcpy(new_arguments_string, arguments_string, arguments_string_length + 1);
                free(arguments_string);
                arguments_string = new_arguments_string;
            }

            memcpy(&arguments_string[arguments_string_length], ", ...", 6);
        }


        char *return_type_string = ast_type_str(&func->return_type);

        if(func->traits & AST_FUNC_FOREIGN){
            fprintf(file, "foreign %s(%s) %s\n", func->name, arguments_string, return_type_string);
        } else {
            fprintf(file, "func %s(%s) %s {\n", func->name, arguments_string, return_type_string);
            if(func->statements != NULL) ast_dump_statements(file, func->statements, func->statements_length, 1);
            fprintf(file, "}\n");
        }

        free(arguments_string);
        free(return_type_string);
    }
}

void ast_dump_statements(FILE *file, ast_expr_t **statements, length_t length, length_t indentation){
    for(length_t s = 0; s != length; s++){
        // Print indentation
        for(length_t ind = 0; ind != indentation; ind++) fprintf(file, "    ");

        // Print statement
        switch(statements[s]->id){
        case EXPR_RETURN: {
                ast_expr_t *return_value = ((ast_expr_return_t*) statements[s])->value;
                char *return_value_str = return_value == NULL ? "" : ast_expr_str(return_value);
                fprintf(file, "return %s\n", return_value_str);
                if(return_value != NULL) free(return_value_str);
            }
            break;
        case EXPR_CALL: {
                ast_expr_call_t *call_stmt = (ast_expr_call_t*) statements[s];
                fprintf(file, "%s(", call_stmt->name);
                for(length_t arg_index = 0; arg_index != call_stmt->arity; arg_index++){
                    char *arg_str = ast_expr_str(call_stmt->args[arg_index]);
                    if(arg_index + 1 != call_stmt->arity) fprintf(file, "%s, ", arg_str);
                    else fprintf(file, "%s", arg_str);
                    free(arg_str);
                }
                fprintf(file, ")\n");
            }
            break;
        case EXPR_DECLARE: {
                ast_expr_declare_t *declare_stmt = (ast_expr_declare_t*) statements[s];
                char *variable_type_str = ast_type_str(&declare_stmt->type);
                fprintf(file, (declare_stmt->value == NULL) ? "%s %s\n" : "%s %s = ", declare_stmt->name, variable_type_str);
                free(variable_type_str);

                if(declare_stmt->value != NULL){
                    char *initial_value = ast_expr_str(declare_stmt->value);
                    fprintf(file, "%s\n", initial_value);
                    free(initial_value);
                }
            }
            break;
        case EXPR_ASSIGN: {
                ast_expr_assign_t *assign_stmt = (ast_expr_assign_t*) statements[s];
                char *destination_str = ast_expr_str(assign_stmt->destination);
                char *value_str = ast_expr_str(assign_stmt->value);
                fprintf(file, "%s = %s\n", destination_str, value_str);
                free(destination_str);
                free(value_str);
            }
            break;
        case EXPR_IF: case EXPR_UNLESS: case EXPR_WHILE: case EXPR_UNTIL: {
                ast_expr_t *if_value = ((ast_expr_if_t*) statements[s])->value;
                char *if_value_str = ast_expr_str(if_value);
                char *keyword_name;

                switch(statements[s]->id){
                case EXPR_IF:     keyword_name = "if";     break;
                case EXPR_UNLESS: keyword_name = "unless"; break;
                case EXPR_WHILE:  keyword_name = "while";  break;
                case EXPR_UNTIL:  keyword_name = "until";  break;
                default: keyword_name = "<unknown conditional keyword>";
                }

                fprintf(file, "%s %s {\n", keyword_name, if_value_str);
                ast_dump_statements(file, ((ast_expr_if_t*) statements[s])->statements, ((ast_expr_if_t*) statements[s])->statements_length, indentation+1);
                for(length_t ind = 0; ind != indentation; ind++) fprintf(file, "    ");
                fprintf(file, "}\n");
                free(if_value_str);
            }
            break;
        case EXPR_IFELSE: case EXPR_UNLESSELSE: {
                ast_expr_t *if_value = ((ast_expr_ifelse_t*) statements[s])->value;
                char *if_value_str = ast_expr_str(if_value);
                fprintf(file, "%s %s {\n", (statements[s]->id == EXPR_IFELSE ? "if" : "unless"), if_value_str);
                ast_dump_statements(file, ((ast_expr_ifelse_t*) statements[s])->statements, ((ast_expr_ifelse_t*) statements[s])->statements_length, indentation+1);
                for(length_t ind = 0; ind != indentation; ind++) fprintf(file, "    ");
                fprintf(file, "} else {\n");
                ast_dump_statements(file, ((ast_expr_ifelse_t*) statements[s])->else_statements, ((ast_expr_ifelse_t*) statements[s])->else_statements_length, indentation+1);
                for(length_t ind = 0; ind != indentation; ind++) fprintf(file, "    ");
                fprintf(file, "}\n");
                free(if_value_str);
            }
            break;
        case EXPR_CALL_METHOD: {
                ast_expr_call_method_t *call_stmt = (ast_expr_call_method_t*) statements[s];
                char *value_str = ast_expr_str(call_stmt->value);
                fprintf(file, "%s.%s(", value_str, call_stmt->name);
                free(value_str);
                for(length_t arg_index = 0; arg_index != call_stmt->arity; arg_index++){
                    char *arg_str = ast_expr_str(call_stmt->args[arg_index]);
                    if(arg_index + 1 != call_stmt->arity) fprintf(file, "%s, ", arg_str);
                    else fprintf(file, "%s", arg_str);
                    free(arg_str);
                }
                fprintf(file, ")\n");
            }
            break;
        case EXPR_DELETE:{
                ast_expr_t *delete_value = ((ast_expr_unary_t*) statements[s])->value;
                char *delete_value_str = ast_expr_str(delete_value);
                fprintf(file, "delete %s\n", delete_value_str);
                free(delete_value_str);
            }
            break;
        default:
            fprintf(file, "<unknown statement>\n");
        }
    }
}

void ast_dump_structs(FILE *file, ast_struct_t *structs, length_t structs_length){
    char *fields_string = malloc(256);
    length_t fields_string_capacity = 256;
    fields_string[0] = '\0';

    for(length_t i = 0; i != structs_length; i++){
        ast_struct_t *structure = &structs[i];
        length_t fields_string_length = 0;

        for(length_t f = 0; f != structure->field_count; f++){
            char *typename = ast_type_str(&structure->field_types[f]);
            length_t typename_length = strlen(typename);
            length_t name_length = strlen(structure->field_names[f]);
            length_t append_length = (f + 1 == structure->field_count) ? (name_length + 1 + typename_length) : (name_length + 1 + typename_length + 2);

            while(fields_string_length + append_length + 1 >= fields_string_capacity){
                fields_string_capacity *= 2;
                char *new_fields_string = malloc(fields_string_capacity);
                memcpy(new_fields_string, fields_string, fields_string_length);
                free(fields_string);
                fields_string = new_fields_string;
            }

            memcpy(&fields_string[fields_string_length], structure->field_names[f], name_length);
            fields_string[fields_string_length + name_length] = ' ';
            memcpy(&fields_string[fields_string_length + name_length + 1], typename, typename_length);

            // Some unnecessary copying of '\0', but whatever it doens't really matter cause
            //     this is just ast dump code
            if(f + 1 == structure->field_count){
                fields_string[fields_string_length + name_length + 1 + typename_length] = '\0';
            } else {
                memcpy(&fields_string[fields_string_length + name_length + 1 + typename_length], ", ", 3);
            }

            fields_string_length += append_length;
            free(typename);
        }

        fprintf(file, "struct %s (%s)\n", structure->name, fields_string);
    }

    free(fields_string);
}

void ast_dump_globals(FILE *file, ast_global_t *globals, length_t globals_length){
    for(length_t i = 0; i != globals_length; i++){
        ast_global_t *global = &globals[i];
        char *global_typename = ast_type_str(&global->type);

        if(global->initial == NULL){
            fprintf(file, "%s %s\n", global->name, global_typename);
        } else {
            char *value = ast_expr_str(global->initial);
            fprintf(file, "%s %s = %s\n", global->name, global_typename, value);
            free(value);
        }

        free(global_typename);
    }
}

void ast_dump_enums(FILE *file, ast_enum_t *enums, length_t enums_length){
    for(length_t i = 0; i != enums_length; i++){
        ast_enum_t *inum = &enums[i];

        char *kinds_string = NULL;
        length_t kinds_string_length = 0;
        length_t kinds_string_capacity = 0;

        for(length_t i = 0; i != inum->length; i++){
            const char *kind_name = inum->kinds[i];
            length_t kind_name_length = strlen(kind_name);
            expand((void**) &kinds_string, sizeof(char), kinds_string_length, &kinds_string_capacity, kind_name_length + 2, 64);

            memcpy(&kinds_string[kinds_string_length], kind_name, kind_name_length);
            kinds_string_length += kind_name_length;

            if(i + 1 == inum->length){
                kinds_string[kinds_string_length] = '\0';
            } else {
                memcpy(&kinds_string[kinds_string_length], ", ", 2);
                kinds_string_length += 2;
            }
        }


        fprintf(file, "%s (%s)\n", inum->name, kinds_string ? kinds_string : "");
        free(kinds_string);
    }
}

void ast_func_create_template(ast_func_t *func, strong_cstr_t name, bool is_stdcall, bool is_foreign, source_t source){
    func->name = name;
    func->arg_names = NULL;
    func->arg_types = NULL;
    func->arg_sources = NULL;
    func->arg_flows = NULL;
    func->arg_type_traits = NULL;
    func->arity = 0;
    func->return_type.elements = NULL;
    func->return_type.elements_length = 0;
    func->return_type.source = NULL_SOURCE;
    func->return_type.source.object_index = source.object_index;
    func->traits = TRAIT_NONE;
    func->statements = NULL;
    func->statements_length = 0;
    func->statements_capacity = 0;
    func->source = source;

    if(strcmp(name, "main") == 0) func->traits |= AST_FUNC_MAIN;
    if(is_stdcall)                func->traits |= AST_FUNC_STDCALL;
    if(is_foreign)                func->traits |= AST_FUNC_FOREIGN;
}

void ast_struct_init(ast_struct_t *structure, strong_cstr_t name, strong_cstr_t *names, ast_type_t *types,
        length_t length, trait_t traits, source_t source){
    structure->name = name;
    structure->field_names = names;
    structure->field_types = types;
    structure->field_count = length;
    structure->traits = traits;
    structure->source = source;
}

void ast_alias_init(ast_alias_t *alias, weak_cstr_t name, ast_type_t type, trait_t traits, source_t source){
    alias->name = name;
    alias->type = type;
    alias->traits = TRAIT_NONE;
    alias->source = source;
}

void ast_enum_init(ast_enum_t *inum,  weak_cstr_t name, weak_cstr_t *kinds, length_t length, source_t source){
    inum->name = name;
    inum->kinds = kinds;
    inum->length = length;
    inum->source = source;
}

ast_struct_t *ast_struct_find(ast_t *ast, const char *name){
    // TODO: Maybe sort and do a binary serach or something
    for(length_t i = 0; i != ast->structs_length; i++){
        if(strcmp(ast->structs[i].name, name) == 0){
            return &ast->structs[i];
        }
    }
    return NULL;
}

successful_t ast_struct_find_field(ast_struct_t *ast_struct, const char *name, length_t *out_index){
    for(length_t i = 0; i != ast_struct->field_count; i++){
        if(strcmp(ast_struct->field_names[i], name) == 0){
            *out_index = i;
            return true;
        }
    }
    return false;
}

successful_t ast_enum_find_kind(ast_enum_t *ast_enum, const char *name, length_t *out_index){
    for(length_t i = 0; i != ast_enum->length; i++){
        if(strcmp(ast_enum->kinds[i], name) == 0){
            *out_index = i;
            return true;
        }
    }
    return false;
}

maybe_index_t find_alias(ast_alias_t *aliases, length_t aliases_length, const char *alias){
    // If not found returns -1 else returns index inside array

    maybe_index_t first, middle, last, comparison;
    first = 0; last = aliases_length - 1;

    while(first <= last){
        middle = (first + last) / 2;
        comparison = strcmp(aliases[middle].name, alias);

        if(comparison == 0) return middle;
        else if(comparison > 0) last = middle - 1;
        else first = middle + 1;
    }

    return -1;
}

maybe_index_t find_constant(ast_constant_t *constants, length_t constants_length, const char *constant){
    // If not found returns -1 else returns index inside array

    maybe_index_t first, middle, last, comparison;
    first = 0; last = constants_length - 1;

    while(first <= last){
        middle = (first + last) / 2;
        comparison = strcmp(constants[middle].name, constant);

        if(comparison == 0) return middle;
        else if(comparison > 0) last = middle - 1;
        else first = middle + 1;
    }

    return -1;
}

maybe_index_t find_enum(ast_enum_t *enums, length_t enums_length, const char *inum){
    // If not found returns -1 else returns index inside array

    maybe_index_t first, middle, last, comparison;
    first = 0; last = enums_length - 1;

    while(first <= last){
        middle = (first + last) / 2;
        comparison = strcmp(enums[middle].name, inum);

        if(comparison == 0) return middle;
        else if(comparison > 0) last = middle - 1;
        else first = middle + 1;
    }

    return -1;
}

void ast_add_enum(ast_t *ast, weak_cstr_t name, weak_cstr_t *kinds, length_t length, source_t source){
    expand((void**) &ast->enums, sizeof(ast_enum_t), ast->enums_length, &ast->enums_capacity, 1, 4);
    ast_enum_init(&ast->enums[ast->enums_length++], name, kinds, length, source);
}

void ast_add_struct(ast_t *ast, strong_cstr_t name, strong_cstr_t *names, ast_type_t *types,
        length_t length, trait_t traits, source_t source){
    expand((void**) &ast->structs, sizeof(ast_struct_t), ast->structs_length, &ast->structs_capacity, 1, 4);
    ast_struct_t *structure = &ast->structs[ast->structs_length++];
    ast_struct_init(structure, name, names, types, length, traits, source);
}

void ast_add_global(ast_t *ast, weak_cstr_t name, ast_type_t type, ast_expr_t *initial_value, trait_t traits, source_t source){
    expand((void**) &ast->globals, sizeof(ast_global_t), ast->globals_length, &ast->globals_capacity, 1, 8);

    ast_global_t *global = &ast->globals[ast->globals_length++];
    global->name = name;
    global->type = type;
    global->initial = initial_value;
    global->traits = traits;
    global->source = source;
}

void ast_add_foreign_library(ast_t *ast, strong_cstr_t library, bool is_framework){
    coexpand((void**) &ast->libraries, sizeof(char*), (void**) &ast->libraries_are_framework, sizeof(bool), ast->libraries_length, &ast->libraries_capacity, 1, 4);
    ast->libraries[ast->libraries_length] = library;
    ast->libraries_are_framework[ast->libraries_length++] = is_framework;
}

ast_type_t* ast_get_usize(ast_t *ast){
    ast_type_t *usize_type = ast->common.ast_usize_type;

    if(usize_type == NULL){
        usize_type = malloc(sizeof(ast_type_t));
        usize_type->elements = malloc(sizeof(ast_elem_t*));
        usize_type->elements[0] = malloc(sizeof(ast_elem_base_t));
        ((ast_elem_base_t*) usize_type->elements[0])->id = AST_ELEM_BASE;
        ((ast_elem_base_t*) usize_type->elements[0])->base = strclone("usize");
        ((ast_elem_base_t*) usize_type->elements[0])->source = NULL_SOURCE;
        usize_type->elements_length = 1;
        usize_type->source = NULL_SOURCE;
        ast->common.ast_usize_type = usize_type;
    }

    return usize_type;
}

int ast_aliases_cmp(const void *a, const void *b){
    return strcmp(((ast_alias_t*) a)->name, ((ast_alias_t*) b)->name);
}

int ast_constants_cmp(const void *a, const void *b){
    return strcmp(((ast_constant_t*) a)->name, ((ast_constant_t*) b)->name);
}

int ast_enums_cmp(const void *a, const void *b){
    return strcmp(((ast_enum_t*) a)->name, ((ast_enum_t*) b)->name);
}
