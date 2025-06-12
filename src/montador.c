#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/linker_defs.h"

#define MAX_LINE 100
#define DEBUG_VISUAL  // descomente esta linha pra ativar visualização do binário final

ObjectFile obj_file;

int find_symbol_in_use_table(const char* name) {
    for (int i = 0; i < obj_file.use_count; i++) {
        if (strcmp(obj_file.use_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_symbol_to_use_table(const char* name) {
    int index = find_symbol_in_use_table(name);
    if (index != -1) {
        return index;
    }
    if (obj_file.use_count >= MAX_SYMBOLS) {
        printf("[montador] Erro: Limite de símbolos de uso excedido.\n");
        exit(1);
    }
    strcpy(obj_file.use_table[obj_file.use_count].name, name);
    printf("[use] símbolo externo: %s\n", name);
    return obj_file.use_count++;
}

int buscar_rotulo(const char *nome) {
    for (int i = 0; i < obj_file.def_count; i++) {
        if (strcmp(obj_file.def_table[i].name, nome) == 0)
            return obj_file.def_table[i].address;
    }
    return -1;
}

int traduz_ou_busca(const char *arg, int current_address) {
    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1])))
        return atoi(arg);

    int address = buscar_rotulo(arg);
    if (address != -1) {
        if (obj_file.rel_count >= MAX_SYMBOLS) {
             printf("[montador] Erro: Limite de entradas na tabela de relocação excedido.\n");
             exit(1);
        }
        obj_file.rel_table[obj_file.rel_count].address = current_address;
        obj_file.rel_table[obj_file.rel_count].symbol_index = -1;
        obj_file.rel_table[obj_file.rel_count].type = 0;
        obj_file.rel_count++;
        printf("[relativo] símbolo local '%s' na posição %d\n", arg, current_address);
        return address;
    }

    int symbol_index = add_symbol_to_use_table(arg);
    if (obj_file.rel_count >= MAX_SYMBOLS) {
         printf("[montador] Erro: Limite de entradas na tabela de relocação excedido.\n");
         exit(1);
    }
    obj_file.rel_table[obj_file.rel_count].address = current_address;
    obj_file.rel_table[obj_file.rel_count].symbol_index = symbol_index;
    obj_file.rel_table[obj_file.rel_count].type = 1;
    obj_file.rel_count++;
    printf("[externo] símbolo '%s' na posição %d\n", arg, current_address);
    return 0;
}

void adicionar_rotulo(const char *nome, int endereco, int type) {
    for (int i = 0; i < obj_file.def_count; i++) {
        if (strcmp(obj_file.def_table[i].name, nome) == 0) {
            if (obj_file.def_table[i].type == 0 && obj_file.def_table[i].address == 0) { 
                obj_file.def_table[i].address = endereco;
                printf("[label] %s (GLOBAL) atualizado para o endereço %d\n", nome, endereco);
                return;
            }
            printf("[erro] label redefinida: %s\n", nome);
            exit(1);
        }
    }
    if (obj_file.def_count >= MAX_SYMBOLS) {
        printf("[montador] Erro: Limite de símbolos de definição excedido.\n");
        exit(1);
    }

    strcpy(obj_file.def_table[obj_file.def_count].name, nome);
    obj_file.def_table[obj_file.def_count].address = endereco;
    obj_file.def_table[obj_file.def_count].type = type;

    printf("[label] %s = %d\n", nome, endereco);
    obj_file.def_count++;
}

int traduz_instrucao(const char *instr) {
    if (strcmp(instr, "add") == 0) return 0;
    if (strcmp(instr, "sub") == 0) return 1;
    if (strcmp(instr, "mul") == 0) return 2;
    if (strcmp(instr, "div") == 0) return 3;
    if (strcmp(instr, "mv")  == 0) return 4;
    if (strcmp(instr, "st")  == 0) return 5;
    if (strcmp(instr, "jmp") == 0) return 6;
    if (strcmp(instr, "jeq") == 0) return 7;
    if (strcmp(instr, "jgt") == 0) return 8;
    if (strcmp(instr, "jlt") == 0) return 9;
    if (strcmp(instr, "w")   == 0) return 10;
    if (strcmp(instr, "r")   == 0) return 11;
    if (strcmp(instr, "stp") == 0) return 12;
    if (strcmp(instr, "GLOBAL") == 0) return 13;
    if (strcmp(instr, "EXTERN") == 0) return 14;

    printf("[erro] instrução inválida: '%s'\n", instr);
    return -1;
}

int traduz_reg(const char *reg) {
    if (strcmp(reg, "a0") == 0) return 0;
    if (strcmp(reg, "a1") == 0) return 1;
    if (strcmp(reg, "a2") == 0) return 2;
    if (strcmp(reg, "a3") == 0) return 3;

    int valor = atoi(reg);
    if (valor < 0 || valor > 3) {
        printf("[erro] registrador inválido: '%s'\n", reg);
        exit(1);
    }

    return valor;
}

void primeira_passagem(FILE *fp) {
    char linha[MAX_LINE];
    int end_instrucao = 0;
    memset(&obj_file, 0, sizeof(ObjectFile));

    while (fgets(linha, sizeof(linha), fp)) {
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        if (strcmp(token, "GLOBAL") == 0) {
            char* symbol = strtok(NULL, " \t\n");
            if (buscar_rotulo(symbol) != -1) {
                printf("[erro] Símbolo GLOBAL '%s' já definido como label local.\n", symbol);
                exit(1);
            }
            adicionar_rotulo(symbol, 0, 0);
            continue;
        }

        if (strcmp(token, "EXTERN") == 0) {
            char* symbol = strtok(NULL, " \t\n");
            add_symbol_to_use_table(symbol);
            continue;
        }

        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0';
            adicionar_rotulo(token, end_instrucao, 0);
            token = strtok(NULL, " \t\n");
            if (!token) continue;
        }

        if (strcmp(token, ".word") == 0) {
            end_instrucao += 1;
            continue;
        }

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        int usado = 1;
        if (opcode <= 3) usado += 3;
        else if (opcode == 4 || opcode == 5) usado += 2;
        else if (opcode == 6) usado += 1;
        else if (opcode == 7) usado += 3;
        else if (opcode == 8 || opcode == 9) usado += 2;
        else if (opcode == 10 || opcode == 11) usado += 1;

        end_instrucao += usado;
    }
    obj_file.code_size = end_instrucao;
    printf("[info] tamanho do código: %d\n", end_instrucao);
}

void segunda_passagem(FILE *entrada, FILE *saida) {
    rewind(entrada);
    char linha[MAX_LINE];
    int pc_instrucao = 0;

    while (fgets(linha, sizeof(linha), entrada)) {
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token || strcmp(token, "GLOBAL") == 0 || strcmp(token, "EXTERN") == 0) continue;

        if (strchr(token, ':')) {
            token = strtok(NULL, " \t\n");
            if (!token) continue;
        }

        if (strcmp(token, ".word") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                obj_file.code[pc_instrucao] = atoi(token);
                printf("[word] mem[%03d] = %d\n", pc_instrucao, obj_file.code[pc_instrucao]);
                pc_instrucao++;
            }
            continue;
        }

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        obj_file.code[pc_instrucao++] = opcode;

        if (opcode <= 3) {
            obj_file.code[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            obj_file.code[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            obj_file.code[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
        } else if (opcode >= 4 && opcode <= 11) {
            char* arg1 = strtok(NULL, " \t\n");
            if (opcode == 4 || opcode == 5 || opcode == 8 || opcode == 9) {
                 obj_file.code[pc_instrucao] = traduz_reg(arg1);
                 pc_instrucao++;
                 char* arg2 = strtok(NULL, " \t\n");
                 obj_file.code[pc_instrucao] = traduz_ou_busca(arg2, pc_instrucao);
                 pc_instrucao++;
            } else if (opcode == 6 || opcode == 10 || opcode == 11) {
                 obj_file.code[pc_instrucao] = traduz_ou_busca(arg1, pc_instrucao);
                 pc_instrucao++;
            } else if (opcode == 7) {
                 obj_file.code[pc_instrucao] = traduz_reg(arg1);
                 pc_instrucao++;
                 char* arg2 = strtok(NULL, " \t\n");
                 obj_file.code[pc_instrucao] = traduz_reg(arg2);
                 pc_instrucao++;
                 char* arg3 = strtok(NULL, " \t\n");
                 obj_file.code[pc_instrucao] = traduz_ou_busca(arg3, pc_instrucao);
                 pc_instrucao++;
            }
        }
    }

    fwrite(&obj_file, sizeof(ObjectFile), 1, saida);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.asm>\n", argv[0]);
        return 1;
    }

    const char *arquivo_entrada = argv[1];
    char arquivo_saida[100];
    sprintf(arquivo_saida, "%s.o", arquivo_entrada);

    FILE *fp = fopen(arquivo_entrada, "r");
    if (!fp) {
        printf("[erro] não foi possível abrir '%s'\n", arquivo_entrada);
        return 1;
    }

    primeira_passagem(fp);

    FILE *saida = fopen(arquivo_saida, "wb");
    if (!saida) {
        printf("[erro] não foi possível criar o arquivo '%s'\n", arquivo_saida);
        fclose(fp);
        return 1;
    }

    segunda_passagem(fp, saida);

    fclose(fp);
    fclose(saida);

#ifdef DEBUG_VISUAL
    printf("\n--- BINÁRIO FINAL GERADO (VISUAL) ---\n");
    for (int i = 0; i < obj_file.code_size; i++) {
        printf("mem[%03d] = %d\n", i, obj_file.code[i]);
    }
#endif

    printf("[montador] montagem finalizada com sucesso -> '%s'\n", arquivo_saida);

    return 0;
}
