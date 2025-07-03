#ifndef LINKER_DEFS_H
#define LINKER_DEFS_H

#define MAX_SYMBOLS 100
#define MAX_CODE_SIZE 320 

typedef struct {
    char name[20];
    int address;
    int type; // 0 pra GLOBAL, 1 pra EXTERN
} SymbolEntry;

typedef struct {
    int address;
    int symbol_index; // indice da use_table pra EXTERN 
    int type; // 0 pra relativo, 1 pra absolute
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

#endif 


