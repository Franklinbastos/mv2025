#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_MEM 320

typedef struct {
    char rotulo[50];
    int endereco;
} Simbolo;

Simbolo tabela_simbolos[100];
int qtde_simbolos = 0;

void primeira_passagem(FILE *entrada);
void segunda_passagem(FILE *entrada, FILE *saida);
int buscar_endereco(char *rotulo);
void adicionar_simbolo(char *rotulo, int endereco);

int main() {
    FILE *entrada = fopen("exemplo.asm", "r");
    FILE *saida = fopen("exercicio", "w");

    if (!entrada || !saida) {
        printf("Erro ao abrir arquivos\n");
        return 1;
    }

    primeira_passagem(entrada);
    rewind(entrada);
    segunda_passagem(entrada, saída);

    fclose(entrada);
    fclose(saida);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INSTR 320
#define MAX_LABELS 100
#define MAX_LINE 100

// estrutura pra associar label com posição de memória
typedef struct {
    char label[50];
    int address;
} Label;

Label tabela_simbolos[MAX_LABELS];
int count_labels = 0;
int memoria[MAX_INSTR];
int pos_mem = 0;

int registrar_label(char *label, int endereco) {
    for (int i = 0; i < count_labels; i++) {
        if (strcmp(tabela_simbolos[i].label, label) == 0) return i;
    }
    strcpy(tabela_simbolos[count_labels].label, label);
    tabela_simbolos[count_labels].address = endereco;
    return count_labels++;
}

int buscar_label(char *label) {
    for (int i = 0; i < count_labels; i++) {
        if (strcmp(tabela_simbolos[i].label, label) == 0) {
            return tabela_simbolos[i].address;
        }
    }
    printf("Label nao encontrada: %s\n", label);
    exit(1);
}

int reg_para_int(char *reg) {
    if (strcmp(reg, "a0") == 0) return 0;
    if (strcmp(reg, "a1") == 0) return 1;
    if (strcmp(reg, "a2") == 0) return 2;
    if (strcmp(reg, "a3") == 0) return 3;
    printf("Registrador invalido: %s\n", reg);
    exit(1);
}

void primeira_passagem(FILE *fp) {
    char linha[MAX_LINE];
    int endereco = 0;
    while (fgets(linha, sizeof(linha), fp)) {
        char *token = strtok(linha, " \n");
        if (!token) continue;

        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0';
            registrar_label(token, endereco);
            token = strtok(NULL, " \n");
        }

        if (!token) continue;

        if (strcmp(token, ".word") == 0) {
            endereco++;
        } else if (strcmp(token, "mv") == 0 || strcmp(token, "st") == 0) {
            endereco += 3;
        } else if (strcmp(token, "add") == 0 || strcmp(token, "sub") == 0 || strcmp(token, "mul") == 0 || strcmp(token, "div") == 0) {
            endereco += 4;
        } else if (strcmp(token, "w") == 0) {
            endereco += 2;
        } else if (strcmp(token, "stp") == 0) {
            endereco++;
        }
    }
}

void segunda_passagem(FILE *fp) {
    rewind(fp);
    char linha[MAX_LINE];

    while (fgets(linha, sizeof(linha), fp)) {
        char *token = strtok(linha, " \n");
        if (!token) continue;

        if (strchr(token, ':')) {
            token = strtok(NULL, " \n");
        }

        if (!token) continue;

        if (strcmp(token, ".word") == 0) {
            token = strtok(NULL, " \n");
            memoria[pos_mem++] = atoi(token);

        } else if (strcmp(token, "mv") == 0) {
            char *r = strtok(NULL, " \n");
            char *label = strtok(NULL, " \n");
            memoria[pos_mem++] = 4;
            memoria[pos_mem++] = reg_para_int(r);
            memoria[pos_mem++] = buscar_label(label);

        } else if (strcmp(token, "add") == 0) {
            char *r0 = strtok(NULL, " \n");
            char *r1 = strtok(NULL, " \n");
            char *r2 = strtok(NULL, " \n");
            memoria[pos_mem++] = 0;
            memoria[pos_mem++] = reg_para_int(r0);
            memoria[pos_mem++] = reg_para_int(r1);
            memoria[pos_mem++] = reg_para_int(r2);

        } else if (strcmp(token, "st") == 0) {
            char *r = strtok(NULL, " \n");
            char *label = strtok(NULL, " \n");
            memoria[pos_mem++] = 5;
            memoria[pos_mem++] = reg_para_int(r);
            memoria[pos_mem++] = buscar_label(label);

        } else if (strcmp(token, "w") == 0) {
            char *label = strtok(NULL, " \n");
            memoria[pos_mem++] = 10;
            memoria[pos_mem++] = buscar_label(label);

        } else if (strcmp(token, "stp") == 0) {
            memoria[pos_mem++] = 12;
        }
    }
}

int main() {
    FILE *fp = fopen("exemplo.asm", "r");
    if (!fp) {
        printf("Erro ao abrir exemplo.asm\n");
        return 1;
    }

    primeira_passagem(fp);
    segunda_passagem(fp);

    FILE *out = fopen("exercicio", "w");
    for (int i = 0; i < pos_mem; i++) {
        fprintf(out, "%d\n", memoria[i]);
    }
    fclose(out);
    fclose(fp);
    printf("Montagem concluida com sucesso!\n");
    return 0;
}
