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

    printf("[erro] label '%s' não encontrada!\n", nome);
    exit(1);
}

void adicionar_rotulo(const char *nome, int endereco) {
    // evita redefinir label
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0) {
            printf("[erro] label redefinida: %s\n", nome);
            exit(1);
        }
    }

    strcpy(tabela_simbolos[total_simbolos].nome, nome);
    tabela_simbolos[total_simbolos].endereco = endereco;

    printf("[label] %s = %d\n", nome, endereco);
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
    int endereco = 0;
    int linha_num = 1;

    while (fgets(linha, sizeof(linha), fp)) {
        printf("\n[linha %d] %s", linha_num++, linha);

        // remover comentários (opcional futuramente)
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        // check se tem label
        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0';
            adicionar_rotulo(token, endereco);

            token = strtok(NULL, " \t\n");

            if (!token) {
                printf("[label isolada] reservando 1 pos em %d\n", endereco);
                endereco++;
                continue;
            }
        }

        if (strcmp(token, ".word") == 0) {
            printf("[.word] reservado 1 posição em %d\n", endereco);
            endereco += 1;
            continue;
        }

        int opcode = traduz_instrucao(token);
        if (opcode == -1) {
            printf("[ignorado] instrução inválida ou não reconhecida\n");
            continue;
        }

        int usado = 1;
        if (opcode <= 3) usado += 3;
        else if (opcode == 4 || opcode == 5) usado += 2;
        else if (opcode == 6) usado += 1;
        else if (opcode == 7) usado += 3;
        else if (opcode == 8 || opcode == 9) usado += 2;
        else if (opcode == 10 || opcode == 11) usado += 1;
        // stp = 1 já incluso

        printf("[instrução] %s ocupa %d posições a partir de %d\n", token, usado, endereco);
        endereco += usado;
    }
}

void segunda_passagem(FILE *entrada, FILE *saida) {
    rewind(entrada);
    char linha[MAX_LINE];
    int linha_num = 1;

    while (fgets(linha, sizeof(linha), entrada)) {
        printf("\n[segunda_passagem] linha %d: %s", linha_num++, linha);

        // remover comentários se quiser futuramente
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        // pula label se houver
        if (strchr(token, ':')) token = strtok(NULL, " \t\n");
        if (!token) continue;

        // trata .word
        if (strcmp(token, ".word") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                int val = atoi(token);
                printf("[.word] valor: %d\n", val);
                fprintf(saida, "%d\n", val);
            } else {
                printf("[erro] .word sem valor\n");
            }
            continue;
        }

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        printf("[instrução] %s (%d)\n", token, opcode);
        fprintf(saida, "%d\n", opcode);

        // agora processa os operandos com debug
        if (opcode <= 3) {  // add, sub, mul, div
            char *r0 = strtok(NULL, " \t\n");
            char *r1 = strtok(NULL, " \t\n");
            char *r2 = strtok(NULL, " \t\n");
            printf("  args: %s %s %s\n", r0, r1, r2);
            fprintf(saida, "%d\n", traduz_reg(r0));
            fprintf(saida, "%d\n", traduz_reg(r1));
            fprintf(saida, "%d\n", traduz_reg(r2));
        } else if (opcode == 4 || opcode == 5) {  // mv, st
            char *r = strtok(NULL, " \t\n");
            char *label = strtok(NULL, " \t\n");
            printf("  args: %s %s\n", r, label);
            fprintf(saida, "%d\n", traduz_reg(r));
            fprintf(saida, "%d\n", buscar_rotulo(label));
        } else if (opcode == 6) {  // jmp
            char *label = strtok(NULL, " \t\n");
            printf("  arg: %s\n", label);
            fprintf(saida, "%d\n", buscar_rotulo(label));
        } else if (opcode == 7) {  // jeq
            char *r1 = strtok(NULL, " \t\n");
            char *r2 = strtok(NULL, " \t\n");
            char *label = strtok(NULL, " \t\n");
            printf("  args: %s %s %s\n", r1, r2, label);
            fprintf(saida, "%d\n", traduz_reg(r1));
            fprintf(saida, "%d\n", traduz_reg(r2));
            fprintf(saida, "%d\n", buscar_rotulo(label));
        } else if (opcode == 8 || opcode == 9) {  // jgt, jlt
            char *r = strtok(NULL, " \t\n");
            char *label = strtok(NULL, " \t\n");
            printf("  args: %s %s\n", r, label);
            fprintf(saida, "%d\n", traduz_reg(r));
            fprintf(saida, "%d\n", buscar_rotulo(label));
        } else if (opcode == 10 || opcode == 11) {  // w, r
            char *label = strtok(NULL, " \t\n");
            printf("  arg: %s\n", label);
            fprintf(saida, "%d\n", buscar_rotulo(label));
        }
        // stp (12) não tem argumento
    }
}

int main(int argc, char *argv[]) {
    const char *arquivo_entrada = (argc >= 2) ? argv[1] : "exemplo.asm";
    const char *arquivo_saida   = "exercicio";

    printf("[montador] lendo de '%s'\n", arquivo_entrada);

    FILE *fp = fopen(arquivo_entrada, "r");
    if (!fp) {
        printf("[erro] não foi possível abrir '%s'\n", arquivo_entrada);
        return 1;
    }

    primeira_passagem(fp);

    FILE *saida = fopen(arquivo_saida, "w");
    if (!saida) {
        printf("[erro] não foi possível criar o arquivo '%s'\n", arquivo_saida);
        fclose(fp);
        return 1;
    }

    segunda_passagem(fp, saida);

    fclose(fp);
    fclose(saida);

    printf("[montador] montagem finalizada com sucesso → '%s'\n", arquivo_saida);

    // recompila a máquina virtual
    printf("[montador] compilando mv.c...\n");
    int compile_status = system("gcc src/mv.c -o bin/mv");
    if (compile_status != 0) {
        printf("[erro] falha na compilação da máquina virtual.\n");
        return 1;
    }

    // você pode liberar isso aqui depois pra aceitar argv também
    // printf("[montador] executando a MV...\n");
    // system("./bin/mv");

    return 0;
}

