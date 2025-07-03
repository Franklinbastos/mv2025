#ifndef LINKER_DEFS_H
#define LINKER_DEFS_H

#define MAX_SYMBOLS 100
#define TAM_MEM 320

typedef struct {
    char nome[20];
    int endereco;
    int tipo; // 0 GLOBAL, 1 EXTERN
} Simbolo;

typedef struct {
    int endereco;
    int simbolo_index; // indice dentro da use_table pra EXTERN
    int tipo; // 0 relativa (offset), 1 absoluta
} Relocacao;

typedef struct {
    int codigo[TAM_MEM];
    int tamanho_codigo;
    Simbolo def_table[MAX_SYMBOLS];
    int def_cont;
    Simbolo use_table[MAX_SYMBOLS];
    int use_cont;
    Relocacao rel_table[MAX_SYMBOLS];
    int rel_cont;
} ObjectFile;

#endif 


