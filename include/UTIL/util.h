
#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    ================================== util.h ==================================
    Module for general memory-related utilities
    ----------------------------------------------------------------------------
*/

#include "UTIL/ground.h"

// -------------------- UTIL_USE_REALLOC --------------------
// Specifies whether or not memory utils should use realloc()
#ifndef TRACK_MEMORY_USAGE
#define UTIL_USE_REALLOC
#endif
// ----------------------------------------------------------

// ---------------- expand ----------------
// Expands an array if it won't be able hold new elements
void expand(void **inout_memory, length_t unit_size, length_t length, length_t *inout_capacity,
    length_t amount, length_t default_capacity);

// ---------------- coexpand ----------------
// Expands two arrays if they won't be able hold new elements
void coexpand(void **inout_memory1, length_t unit_size1, void **inout_memory2, length_t unit_size2,
    length_t length, length_t *inout_capacity, length_t amount, length_t default_capacity);

// ---------------- grow ----------------
// Forces growth of an array to a certain length
#ifdef UTIL_USE_REALLOC
#define grow(a, b, c, d) grow_impl(a, b, d)
void grow_impl(void **inout_memory, length_t unit_size, length_t new_length);
#else
#define grow(a, b, c, d) grow_impl(a, b, c, d)
void grow_impl(void **inout_memory, length_t unit_size, length_t old_length, length_t new_length);
#endif

// ---------------- strclone ----------------
// Clones a string, producing a duplicate
strong_cstr_t strclone(const char *src);

#ifdef __cplusplus
}
#endif

#endif // UTIL_H
