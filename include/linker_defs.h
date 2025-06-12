#ifndef LINKER_DEFS_H
#define LINKER_DEFS_H

#define MAX_SYMBOLS 100
#define MAX_CODE_SIZE 320 // Assuming max memory size for now

typedef struct {
    char name[20];
    int address;
    int type; // 0 for GLOBAL, 1 for EXTERN
} SymbolEntry;

typedef struct {
    int address;
    int symbol_index; // Index into the use_table for EXTERN symbols
    int type; // 0 for relative, 1 for absolute (if needed)
} RelocationEntry;

typedef struct {
    int code[MAX_CODE_SIZE];
    int code_size;
    SymbolEntry def_table[MAX_SYMBOLS];
    int def_count;
    SymbolEntry use_table[MAX_SYMBOLS];
    int use_count;
    RelocationEntry rel_table[MAX_SYMBOLS];
    int rel_count;
} ObjectFile;

#endif // LINKER_DEFS_H


