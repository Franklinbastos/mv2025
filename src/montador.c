#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 100
#define MAX_LINE 100

typedef struct {
    char nome[20];
    int endereco;
} Simbolo;

Simbolo tabela_simbolos[MAX_LABELS];
int total_simbolos = 0;

int buscar_rotulo(const char *nome) {
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0)
            return tabela_simbolos[i].endereco;
    }
    return -1;
}

void adicionar_rotulo(const char *nome, int endereco) {
    strcpy(tabela_simbolos[total_simbolos].nome, nome);
    tabela_simbolos[total_simbolos].endereco = endereco;
    total_simbolos++;
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
    return -1;
}

int traduz_reg(const char *reg) {
    if (strcmp(reg, "a0") == 0) return 0;
    if (strcmp(reg, "a1") == 0) return 1;
    if (strcmp(reg, "a2") == 0) return 2;
    if (strcmp(reg, "a3") == 0) return 3;
    return atoi(reg);  // se for número direto
}

void primeira_passagem(FILE *fp) {
    char linha[MAX_LINE];
    int endereco = 0;

    while (fgets(linha, sizeof(linha), fp)) {
        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0';
            char label[MAX_LABELS];
            strcpy(label, token);

            token = strtok(NULL, " \t\n");
            if (!token) continue;

            // se for .word, registra label e avança 1 no endereco
            if (strcmp(token, ".word") == 0) {
                adicionar_rotulo(label, endereco);
                endereco++;  // ocupa 1 posição na memória
                continue;
            }

            // se não, é uma instrução com label na frente
            int opcode = traduz_instrucao(token);
            if (opcode != -1) {
                adicionar_rotulo(label, endereco);
            }
        }

        if (!token) continue;

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        endereco++;  // 1 para o opcode
        if (opcode <= 3) endereco += 3;
        else if (opcode == 4 || opcode == 5) endereco += 2;
        else if (opcode == 6) endereco += 1;
        else if (opcode == 7) endereco += 3;
        else if (opcode == 8 || opcode == 9) endereco += 2;
        else if (opcode == 10 || opcode == 11) endereco += 1;
    }
}


void segunda_passagem(FILE *entrada, FILE *saida) {
    rewind(entrada);
    char linha[MAX_LINE];

    while (fgets(linha, sizeof(linha), entrada)) {
        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        if (strchr(token, ':')) token = strtok(NULL, " \t\n");
        if (!token) continue;

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        fprintf(saida, "%d\n", opcode);

        if (opcode <= 3) {  // add, sub, mul, div
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
        } else if (opcode == 4 || opcode == 5) {  // mv, st
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", buscar_rotulo(strtok(NULL, " \t\n")));
        } else if (opcode == 6) {  // jmp
            fprintf(saida, "%d\n", buscar_rotulo(strtok(NULL, " \t\n")));
        } else if (opcode == 7) {  // jeq
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", buscar_rotulo(strtok(NULL, " \t\n")));
        } else if (opcode == 8 || opcode == 9) {  // jgt, jlt
            fprintf(saida, "%d\n", traduz_reg(strtok(NULL, " \t\n")));
            fprintf(saida, "%d\n", buscar_rotulo(strtok(NULL, " \t\n")));
        } else if (opcode == 10 || opcode == 11) {  // w, r
            fprintf(saida, "%d\n", buscar_rotulo(strtok(NULL, " \t\n")));
        }
        // stp não precisa de nada
    }
}

int main() {
    FILE *fp = fopen("exemplo.asm", "r");
    if (!fp) {
        printf("Erro ao abrir exemplo.asm\n");
        return 1;
    }

    primeira_passagem(fp);

    FILE *saida = fopen("exercicio", "w");
    if (!saida) {
        printf("Erro ao criar arquivo de saída\n");
        return 1;
    }

    segunda_passagem(fp, saida);

    fclose(fp);
    fclose(saida);

    printf("Montagem finalizada com sucesso.\n");
    return 0;
}
