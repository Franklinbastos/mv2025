#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linker_defs.h"

#define MAX_FILES 10
#define MAX_SYMBOLS 100
#define TAM_MEM 320

ObjectFile object_files[MAX_FILES];
int num_object_files = 0;

int final_memory[TAM_MEM];
int final_memory_size = 0;

// Global symbol table for the linker
Simbolo global_table[MAX_SYMBOLS];
int global_cont = 0;

// adiciona símbolo global na tabela ou retorna end se já existe
void adiciona_global(const char* nome, int endereco) {
    printf("[DEBUG] Tentando adicionar símbolo global: %s -> %d\n", nome, endereco);
    for (int i = 0; i < global_cont; i++) {
        if (strcmp(global_table[i].nome, nome) == 0) {
            printf("[ligador] Erro: Símbolo global redefinido: %s\n", nome);
            exit(1);
        }
    }
    strcpy(global_table[global_cont].nome, nome);
    global_table[global_cont].endereco = endereco;
    global_table[global_cont].tipo = 0; // GLOBAL
    global_cont++;
}

int procura_global(const char* nome) {
    for (int i = 0; i < global_cont; i++) {
        if (strcmp(global_table[i].nome, nome) == 0) {
            return global_table[i].endereco;
        }
    }
    return -1;
}

void carrega_object(const char *filenome) {
    printf("\n[ligador] === Carregando objeto: %s ===\n", filenome);
    FILE *file = fopen(filenome, "rb");
    if (!file) {
        printf("[ligador] Erro: Não foi possível abrir o arquivo objeto %s\n", filenome);
        exit(1);
    }

    if (num_object_files >= MAX_FILES) {
        printf("[ligador] Erro: Limite de arquivos objeto excedido.\n");
        exit(1);
    }

    // Lê do arquivo para a posição atual do array de ObjectFiles
    fread(&object_files[num_object_files], sizeof(ObjectFile), 1, file);
    fclose(file);

    ObjectFile *obj = &object_files[num_object_files];

    // DEBUG: imprime o código lido
    printf("[DEBUG] → Código lido (tamanho %d):\n", obj->tamanho_codigo);
    for (int i = 0; i < obj->tamanho_codigo; i++) {
        printf("  codigo[%d] = %d\n", i, obj->codigo[i]);
    }

    // DEBUG: imprime tabela de definições
    printf("[DEBUG] → Tabela de definições (%d):\n", obj->def_cont);
    for (int i = 0; i < obj->def_cont; i++) {
        printf("  def[%d]: nome='%s', addr=%d, tipo=%d\n",
               i, obj->def_table[i].nome, obj->def_table[i].endereco, obj->def_table[i].tipo);
    }

    // DEBUG: imprime tabela de usos
    printf("[DEBUG] → Tabela de usos (%d):\n", obj->use_cont);
    for (int i = 0; i < obj->use_cont; i++) {
        printf("  use[%d]: nome='%s'\n", i, obj->use_table[i].nome);
    }

    // DEBUG: imprime tabela de relocações
    printf("[DEBUG] → Tabela de relocações (%d):\n", obj->rel_cont);
    for (int i = 0; i < obj->rel_cont; i++) {
        printf("  rel[%d]: addr=%d, tipo=%d, simbolo_index=%d\n",
               i, obj->rel_table[i].endereco, obj->rel_table[i].tipo, obj->rel_table[i].simbolo_index);
    }

    printf("[ligador] Arquivo objeto %s carregado com sucesso.\n", filenome);
    num_object_files++;
}

// Monta tabela GLOBAL e resolve relocations  
void resolve_simbolos() {
    printf("\n[ligador] === Resolvendo símbolos globais e relocando ===\n");
    int current_endereco_offset = 0;

    //registra globais na tabela final com offset
    for (int i = 0; i < num_object_files; i++) {
        printf("\n[DEBUG] Módulo %d — Offset base: %d\n", i, current_endereco_offset);
        for (int j = 0; j < object_files[i].def_cont; j++) {
            if (object_files[i].def_table[j].tipo == 0) { // 0 = GLOBAL
                adiciona_global(
                    object_files[i].def_table[j].nome,
                    object_files[i].def_table[j].endereco + current_endereco_offset
                );
                printf("[ligador] GLOBAL: %s → %d (módulo %d)\n",
                    object_files[i].def_table[j].nome,
                    object_files[i].def_table[j].endereco,
                    i
                );
            }
        }
        current_endereco_offset += object_files[i].tamanho_codigo;
    }
    final_memory_size = current_endereco_offset;

    //aplica relocations
    current_endereco_offset = 0;
    for (int i = 0; i < num_object_files; i++) {
        ObjectFile *obj = &object_files[i];
        printf("\n[DEBUG] Relocando módulo %d — Offset base: %d\n", i, current_endereco_offset);
        for (int j = 0; j < obj->rel_cont; j++) {
            int addr = obj->rel_table[j].endereco;
            if (obj->rel_table[j].tipo == 0) {
                // local soma offset
                obj->codigo[addr] += current_endereco_offset;
                printf("[ligador] Relativo: addr=%d → %d\n", addr, obj->codigo[addr]);
            } else {
                // externo busca endereço final na tabela global
                int sym_idx = obj->rel_table[j].simbolo_index;
                char *symbol = obj->use_table[sym_idx].nome;
                int final_addr = procura_global(symbol);
                if (final_addr != -1) {
                    obj->codigo[addr] = final_addr;
                    printf("[ligador] EXTERN: %s → %d (em addr %d)\n", symbol, final_addr, addr);
                } else {
                    printf("[ligador] Erro: símbolo externo '%s' não resolvido.\n", symbol);
                    exit(1);
                }
            }
        }
        current_endereco_offset += obj->tamanho_codigo;
    }
}

void gera_executavel(const char *output_filenome) {
    printf("\n[ligador] === Gerando executável: %s ===\n", output_filenome);
    FILE *output = fopen(output_filenome, "w");
    if (!output) {
        printf("[ligador] Erro ao criar: %s\n", output_filenome);
        exit(1);
    }

    int addr = 0;
    for (int i = 0; i < num_object_files; i++) {
        for (int j = 0; j < object_files[i].tamanho_codigo; j++) {
            fprintf(output, "%d\n", object_files[i].codigo[j]);
            final_memory[addr++] = object_files[i].codigo[j];
        }
    }

    final_memory_size = addr;
    fclose(output);
    printf("[ligador] Executável '%s' gerado com %d palavras.\n", output_filenome, final_memory_size);

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
    // ve se passou pelo menos um object e o nome do executável
    if (argc < 3) {
        printf("Uso: %s <arquivo_saida> <arquivo_objeto1> [arquivo_objeto2 ...]\n", argv[0]);
        return 1;
    }

    const char *output = argv[1];
    printf("\n[ligador] ==== INÍCIO DO LIGADOR ====\n");

    // carregar todos os arquivos
    for (int i = 2; i < argc; i++) {
        printf("[ligador] Recebido arquivo objeto: %s\n", argv[i]);
        carrega_object(argv[i]);
    }

    resolve_simbolos();

    gera_executavel(output);

    printf("\n[ligador] ==== FIM DO PROCESSO ====\n");

    // --- COMPILE A MV ---
    printf("[ligador] Compilando mv.c para bin/mv...\n");
    if (system("gcc src/mv.c -o bin/mv") != 0) {
        printf("[ligador] Erro ao compilar mv.c\n");
        return 1;
    }

    // --- EXECUTE A MV ---
    char comando[200];
    sprintf(comando, "./bin/mv %s", output);
    printf("[ligador] Executando MV com programa '%s'...\n", output);
    if (system(comando) != 0) {
        printf("[ligador] Erro ao executar mv\n");
        return 1;
    }

    return 0;
}