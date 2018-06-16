
#ifndef PKG_H
#define PKG_H

/*
    ================================== pkg.h ==================================
    Module for writing & reading pre-lexed packages
    ---------------------------------------------------------------------------
*/

#include "token.h"
#include "ground.h"
#include "object.h"
#include "compiler.h"

// ---------------- pkg_version_t ----------------
// Container for package version information
typedef struct {
    unsigned long long magic_number;
    unsigned short endianness;
    unsigned long long iteration_version;
} __attribute__((packed)) pkg_version_t;

// ---------------- pkg_header_t ----------------
// Package header information
typedef struct {
    unsigned long long length;
} __attribute__((packed)) pkg_header_t;

// ---------------- pkg_write ----------------
// Creates and writes a package to disk
int pkg_write(const char *filename, tokenlist_t *tokenlist);

// ---------------- pkg_read ----------------
// Reads a package from disk
int pkg_read(compiler_t *compiler, object_t *object);

// ---------------- pkg_compress_word ----------------
// Attempts to compress a word, then writes it to disk
void pkg_compress_word(FILE *file, token_t *token);

#endif // PKG_H