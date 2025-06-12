#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linker_defs.h"

#define MAX_FILES 10

ObjectFile object_files[MAX_FILES];
int num_object_files = 0;

int final_memory[MAX_CODE_SIZE];
int final_memory_size = 0;

// Global symbol table for the linker
SymbolEntry global_symbol_table[MAX_SYMBOLS];
int global_symbol_count = 0;

void add_global_symbol(const char* name, int address) {
    printf("[DEBUG] Tentando adicionar símbolo global: %s -> %d\n", name, address);
    for (int i = 0; i < global_symbol_count; i++) {
        if (strcmp(global_symbol_table[i].name, name) == 0) {
            printf("[ligador] Erro: Símbolo global redefinido: %s\n", name);
            exit(1);
        }
    }
    strcpy(global_symbol_table[global_symbol_count].name, name);
    global_symbol_table[global_symbol_count].address = address;
    global_symbol_table[global_symbol_count].type = 0; // GLOBAL
    global_symbol_count++;
}

int find_global_symbol(const char* name) {
    for (int i = 0; i < global_symbol_count; i++) {
        if (strcmp(global_symbol_table[i].name, name) == 0) {
            return global_symbol_table[i].address;
        }
    }
    return -1;
}

void load_object_file(const char *filename) {
    printf("\n[ligador] === Carregando objeto: %s ===\n", filename);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("[ligador] Erro: Não foi possível abrir o arquivo objeto 	%s	\n", filename);
        exit(1);
    }

    if (num_object_files >= MAX_FILES) {
        printf("[ligador] Erro: Limite de arquivos objeto excedido.\n");
        exit(1);
    }

    fread(&object_files[num_object_files], sizeof(ObjectFile), 1, file);
    fclose(file);

    ObjectFile *obj = &object_files[num_object_files];
    printf("[DEBUG] → Código lido (tamanho %d):\n", obj->code_size);
    for (int i = 0; i < obj->code_size; i++) {
        printf("  code[%d] = %d\n", i, obj->code[i]);
    }

    printf("[DEBUG] → Tabela de definições (%d):\n", obj->def_count);
    for (int i = 0; i < obj->def_count; i++) {
        printf("  def[%d]: name='%s', addr=%d, type=%d\n", i, obj->def_table[i].name, obj->def_table[i].address, obj->def_table[i].type);
    }

    printf("[DEBUG] → Tabela de usos (%d):\n", obj->use_count);
    for (int i = 0; i < obj->use_count; i++) {
        printf("  use[%d]: name='%s'\n", i, obj->use_table[i].name);
    }

    printf("[DEBUG] → Tabela de relocações (%d):\n", obj->rel_count);
    for (int i = 0; i < obj->rel_count; i++) {
        printf("  rel[%d]: addr=%d, type=%d, symbol_index=%d\n", i, obj->rel_table[i].address, obj->rel_table[i].type, obj->rel_table[i].symbol_index);
    }

    printf("[ligador] Arquivo objeto 	%s	 carregado com sucesso.\n", filename);
    num_object_files++;
}

void resolve_symbols() {
    printf("\n[ligador] === Resolvendo símbolos globais e relocando ===\n");
    int current_address_offset = 0;

    for (int i = 0; i < num_object_files; i++) {
        printf("\n[DEBUG] Módulo %d — Offset base: %d\n", i, current_address_offset);
        for (int j = 0; j < object_files[i].def_count; j++) {
            if (object_files[i].def_table[j].type == 0) {
                add_global_symbol(
                    object_files[i].def_table[j].name,
                    object_files[i].def_table[j].address + current_address_offset
                );
                printf("[ligador] GLOBAL: %s → %d (módulo %d)\n",
                    object_files[i].def_table[j].name,
                    object_files[i].def_table[j].address,
                    i
                );
            }
        }
        current_address_offset += object_files[i].code_size;
    }
    final_memory_size = current_address_offset;

    current_address_offset = 0;
    for (int i = 0; i < num_object_files; i++) {
        ObjectFile *obj = &object_files[i];
        printf("\n[DEBUG] Relocando módulo %d — Offset base: %d\n", i, current_address_offset);
        for (int j = 0; j < obj->rel_count; j++) {
            int addr = obj->rel_table[j].address;
            if (obj->rel_table[j].type == 0) {
                obj->code[addr] += current_address_offset;
                printf("[ligador] Relativo: addr=%d → %d\n", addr, obj->code[addr]);
            } else {
                int sym_idx = obj->rel_table[j].symbol_index;
                char *symbol = obj->use_table[sym_idx].name;
                int final_addr = find_global_symbol(symbol);
                if (final_addr != -1) {
                    obj->code[addr] = final_addr;
                    printf("[ligador] EXTERN: %s → %d (em addr %d)\n", symbol, final_addr, addr);
                } else {
                    printf("[ligador] Erro: símbolo externo '%s' não resolvido.\n", symbol);
                    exit(1);
                }
            }
        }
        current_address_offset += obj->code_size;
    }
}

void generate_executable(const char *output_filename) {
    printf("\n[ligador] === Gerando executável: %s ===\n", output_filename);
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("[ligador] Erro ao criar: %s\n", output_filename);
        exit(1);
    }

    int addr = 0;
    for (int i = 0; i < num_object_files; i++) {
        for (int j = 0; j < object_files[i].code_size; j++) {
            fprintf(output, "%d\n", object_files[i].code[j]);
            final_memory[addr++] = object_files[i].code[j];
        }
    }

    final_memory_size = addr;
    fclose(output);
    printf("[ligador] Executável '%s' gerado com %d palavras.\n", output_filename, final_memory_size);

    printf("\n--- MEMÓRIA FINAL (compactada) ---\n");
    int i = 0;
    while (i < final_memory_size) {
        if (final_memory[i] != 0) {
            printf("mem[%03d] = %d\n", i, final_memory[i]);
            i++;
        } else {
            int start = i;
            while (i < final_memory_size && final_memory[i] == 0) i++;
            int end = i - 1;
            if (start == end)
                printf("mem[%03d] = 0\n", start);
            else
                printf("mem[%03d–%03d] = 0\n", start, end);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_saida> <arquivo_objeto1> [arquivo_objeto2 ...]\n", argv[0]);
        return 1;
    }

    const char *output = argv[1];
    printf("\n[ligador] ==== INÍCIO DO LIGADOR ====\n");

    for (int i = 2; i < argc; i++) {
        printf("[ligador] Recebido arquivo objeto: %s\n", argv[i]);
        load_object_file(argv[i]);
    }

    resolve_symbols();
    generate_executable(output);

    printf("\n[ligador] ==== FIM DO PROCESSO ====\n");
    return 0;
}


