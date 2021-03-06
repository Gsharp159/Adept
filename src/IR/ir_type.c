
#include "IR/ir_pool.h"
#include "IR/ir_type.h"

strong_cstr_t ir_type_str(ir_type_t *type){
    // NOTE: Returns allocated string of that type
    // NOTE: This function is recusive

    #define RET_CLONE_STR_MACRO(d, s) { \
        memory = malloc(s); memcpy(memory, d, s); return memory; \
    }

    char *memory;
    char *chained;
    length_t chained_length;

    switch(type->kind){
    case TYPE_KIND_NONE:
        RET_CLONE_STR_MACRO("__nul_type_kind", 16);
    case TYPE_KIND_POINTER:
        chained = ir_type_str((ir_type_t*) type->extra);
        chained_length = strlen(chained);
        memory = malloc(chained_length + 2);
        memcpy(memory, "*", 1);
        memcpy(&memory[1], chained, chained_length + 1);
        free(chained);
        return memory;
    case TYPE_KIND_S8:      RET_CLONE_STR_MACRO("s8", 3);
    case TYPE_KIND_U8:      RET_CLONE_STR_MACRO("u8", 3);
    case TYPE_KIND_S16:     RET_CLONE_STR_MACRO("s16", 4);
    case TYPE_KIND_U16:     RET_CLONE_STR_MACRO("u16", 4);
    case TYPE_KIND_S32:     RET_CLONE_STR_MACRO("s32", 4);
    case TYPE_KIND_U32:     RET_CLONE_STR_MACRO("u32", 4);
    case TYPE_KIND_S64:     RET_CLONE_STR_MACRO("s64", 4);
    case TYPE_KIND_U64:     RET_CLONE_STR_MACRO("u64", 4);
    case TYPE_KIND_HALF:    RET_CLONE_STR_MACRO("h", 2);
    case TYPE_KIND_FLOAT:   RET_CLONE_STR_MACRO("f", 2);
    case TYPE_KIND_DOUBLE:  RET_CLONE_STR_MACRO("d", 2);
    case TYPE_KIND_BOOLEAN: RET_CLONE_STR_MACRO("bool", 5);
    case TYPE_KIND_VOID:    RET_CLONE_STR_MACRO("void", 5);
    case TYPE_KIND_FUNCPTR: RET_CLONE_STR_MACRO("__funcptr_type_kind", 20);
    case TYPE_KIND_FIXED_ARRAY: {
        ir_type_extra_fixed_array_t *fixed = (ir_type_extra_fixed_array_t*) type->extra;
        chained = ir_type_str(fixed->subtype);
        memory = malloc(strlen(chained) + 24);
        sprintf(memory, "[%d] %s", (int) fixed->length, chained);
        free(chained);
        return memory;
    }
    default: RET_CLONE_STR_MACRO("__unk_type_kind", 16);
    }

    #undef RET_CLONE_STR_MACRO
}

bool ir_types_identical(ir_type_t *a, ir_type_t *b){
    // NOTE: Returns true if the two types are identical
    // NOTE: The two types must match element to element to be considered identical
    // [pointer] [pointer] [u8]    [pointer] [pointer] [s32]  -> false
    // [pointer] [u8]              [pointer] [u8]             -> true

    if(a->kind != b->kind) return false;

    switch(a->kind){
    case TYPE_KIND_POINTER:
        return ir_types_identical((ir_type_t*) a->extra, (ir_type_t*) b->extra);
    case TYPE_KIND_UNION:
        if(((ir_type_extra_composite_t*) a->extra)->subtypes_length != ((ir_type_extra_composite_t*) b->extra)->subtypes_length) return false;
        for(length_t i = 0; i != ((ir_type_extra_composite_t*) a->extra)->subtypes_length; i++){
            if(!ir_types_identical(((ir_type_extra_composite_t*) a->extra)->subtypes[i], ((ir_type_extra_composite_t*) b->extra)->subtypes[i])) return false;
        }
        return true;
    case TYPE_KIND_STRUCTURE:
        if(((ir_type_extra_composite_t*) a->extra)->subtypes_length != ((ir_type_extra_composite_t*) b->extra)->subtypes_length) return false;
        for(length_t i = 0; i != ((ir_type_extra_composite_t*) a->extra)->subtypes_length; i++){
            if(!ir_types_identical(((ir_type_extra_composite_t*) a->extra)->subtypes[i], ((ir_type_extra_composite_t*) b->extra)->subtypes[i])) return false;
        }
        return true;
    }

    return true;
}

ir_type_t* ir_type_pointer_to(ir_pool_t *pool, ir_type_t *base){
    ir_type_t *ptr_type = ir_pool_alloc(pool, sizeof(ir_type_t));
    ptr_type->kind = TYPE_KIND_POINTER;
    ptr_type->extra = base;
    return ptr_type;
}

ir_type_t* ir_type_dereference(ir_type_t *type){
    if(type->kind != TYPE_KIND_POINTER) return NULL;
    return (ir_type_t*) type->extra;
}

// (For 64 bit systems)
unsigned int global_type_kind_sizes_64[] = {
     0, // TYPE_KIND_NONE
    64, // TYPE_KIND_POINTER
     8, // TYPE_KIND_S8
    16, // TYPE_KIND_S16
    32, // TYPE_KIND_S32
    64, // TYPE_KIND_S64
     8, // TYPE_KIND_U8
    16, // TYPE_KIND_U16
    32, // TYPE_KIND_U32
    64, // TYPE_KIND_U64
    16, // TYPE_KIND_HALF
    32, // TYPE_KIND_FLOAT
    64, // TYPE_KIND_DOUBLE
     1, // TYPE_KIND_BOOLEAN
     0, // TYPE_KIND_UNION
     0, // TYPE_KIND_STRUCTURE
     0, // TYPE_KIND_VOID
    64, // TYPE_KIND_FUNCPTR
     0, // TYPE_KIND_FIXED_ARRAY
};

bool global_type_kind_signs[] = { // (0 == unsigned, 1 == signed)
    0, // TYPE_KIND_NONE
    0, // TYPE_KIND_POINTER
    1, // TYPE_KIND_S8
    1, // TYPE_KIND_S16
    1, // TYPE_KIND_S32
    1, // TYPE_KIND_S64
    0, // TYPE_KIND_U8
    0, // TYPE_KIND_U16
    0, // TYPE_KIND_U32
    0, // TYPE_KIND_U64
    1, // TYPE_KIND_HALF
    1, // TYPE_KIND_FLOAT
    1, // TYPE_KIND_DOUBLE
    0, // TYPE_KIND_BOOLEAN
    0, // TYPE_KIND_UNION
    0, // TYPE_KIND_STRUCTURE
    0, // TYPE_KIND_VOID
    0, // TYPE_KIND_FUNCPTR
    0, // TYPE_KIND_FIXED_ARRAY
};
