#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LABELS 100
#define MAX_LINE 100
#define TAM_MEM 320

typedef struct {
    char nome[20];
    int endereco;
} Simbolo;

Simbolo tabela_simbolos[MAX_LABELS];
int total_simbolos = 0;

int pos_dados = 270;  // início da área de dados

int buscar_rotulo(const char *nome) {
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0)
            return tabela_simbolos[i].endereco;
    }

    printf("[erro] label '%s' não encontrada!\n", nome);
    exit(1);
}

// função interna que trata números ou labels
int traduz_ou_busca(const char *arg) {
    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1])))
        return atoi(arg);
    return buscar_rotulo(arg);
}

// registra label: se for .word, vai pra área de dados; senão, usa o PC atual
void adicionar_rotulo(const char *nome, int endereco) {
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0) {
            printf("[erro] label redefinida: %s\n", nome);
            exit(1);
        }
    }

    // label de .word começa com "__DATA__:"
    if (strncmp(nome, "__DATA__:", 9) == 0) {
        nome += 9;
        endereco = pos_dados++;
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
    int end_instrucao = 0;
    int end_dado = 270;
    int linha_num = 1;

    while (fgets(linha, sizeof(linha), fp)) {
        printf("\n[linha %d] %s", linha_num++, linha);

        // remover comentários
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        int usando_word = 0;

        // se tem label
        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0'; // remove ':'
            char *prox = strtok(NULL, " \t\n");

            // se próxima token for .word, label aponta pra área de dados
            if (prox && strcmp(prox, ".word") == 0) {
                adicionar_rotulo(token, end_dado);
                usando_word = 1;
                token = prox;
            } else {
                adicionar_rotulo(token, end_instrucao);
                token = prox;
            }

            if (!token) continue;
        }

        if (strcmp(token, ".word") == 0) {
            printf("[.word] reservado 1 posição em %d\n", end_dado);
            end_dado += 1;
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

        printf("[instrução] %s ocupa %d posições a partir de %d\n", token, usado, end_instrucao);
        end_instrucao += usado;
    }
}

void segunda_passagem(FILE *entrada, FILE *saida) {
    rewind(entrada);
    char linha[MAX_LINE];
    int linha_num = 1;

    int memoria[320] = {0};
    int pc_instrucao = 0;
    int pos_dados = 270;

    while (fgets(linha, sizeof(linha), entrada)) {
        printf("\n[segunda_passagem] linha %d: %s", linha_num++, linha);

        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        // verifica se é uma label
        char *label_nome = NULL;
        if (strchr(token, ':')) {
            token[strlen(token) - 1] = '\0'; // remove ':'
            label_nome = token;             // salva o nome
            token = strtok(NULL, " \t\n");  // pega próximo token
            if (!token) continue;
        }

        // trata .word
        if (strcmp(token, ".word") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                int val = atoi(token);
                int destino = pos_dados++;
                memoria[destino] = val;

                if (label_nome) {

                } else {
                    printf("[erro] .word sem label\n");
                    exit(1);
                }

                printf("[.word] valor %d em mem[%d]\n", val, destino);
            } else {
                printf("[erro] .word sem valor\n");
                exit(1);
            }
            continue;
        }

        int opcode = traduz_instrucao(token);
        if (opcode == -1) continue;

        printf("[instrução] %s (%d)\n", token, opcode);
        memoria[pc_instrucao++] = opcode;

        if (opcode <= 3) {
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
        } else if (opcode == 4 || opcode == 5) {
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_ou_busca(strtok(NULL, " \t\n"));
        } else if (opcode == 6) {
            memoria[pc_instrucao++] = traduz_ou_busca(strtok(NULL, " \t\n"));
        } else if (opcode == 7) {
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_ou_busca(strtok(NULL, " \t\n"));
        } else if (opcode == 8 || opcode == 9) {
            memoria[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            memoria[pc_instrucao++] = traduz_ou_busca(strtok(NULL, " \t\n"));
        } else if (opcode == 10 || opcode == 11) {
            memoria[pc_instrucao++] = traduz_ou_busca(strtok(NULL, " \t\n"));
        }
    }

    for (int i = 0; i < 320; i++) {
        fprintf(saida, "%d\n", memoria[i]);
    }

    printf("\n--- BINÁRIO GERADO ---\n");
    int i = 0;
    while (i < TAM_MEM) {
        if (memoria[i] != 0) {
            printf("mem[%03d] = %d\n", i, memoria[i]);
            i++;
        } else {
            int start = i;
            while (i < TAM_MEM && memoria[i] == 0) i++;
            int end = i - 1;
            if (start == end)
                printf("mem[%03d] = 0\n", start);
            else
                printf("mem[%03d–%03d] = 0\n", start, end);
        }
    }
}

int main(int argc, char *argv[]) {
    const char *arquivo_entrada = (argc >= 2) ? argv[1] : "exemplo.asm";
    const char *arquivo_saida   = "exercicio";

    printf("[montador] lendo de '%s'\n", arquivo_entrada);

    FILE *fp = fopen(arquivo_entrada, "r");

    // DEBUG: mostrar o código ASM original
    printf("\n--- CÓDIGO ASM ORIGINAL (%s) ---\n", arquivo_entrada);
    FILE *leitura = fopen(arquivo_entrada, "r");
    if (leitura) {
        char linha[MAX_LINE];
        int lin = 1;
        while (fgets(linha, sizeof(linha), leitura)) {
            printf("%2d: %s", lin++, linha);
        }
        fclose(leitura);
    } else {
        printf("[erro] falha ao reabrir '%s'\n", arquivo_entrada);
    }

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

   // DEBUG: mostrar o binário gerado com resumo de zeros
    printf("\n--- BINÁRIO GERADO ---\n");
    FILE *verif = fopen(arquivo_saida, "r");
    if (verif) {
        int val, i = 0;
        int zero_ini = -1;

        while (fscanf(verif, "%d", &val) != EOF) {
            if (val == 0) {
                if (zero_ini == -1)
                    zero_ini = i; // início da sequência de zeros
            } else {
                if (zero_ini != -1) {
                    if (i - 1 == zero_ini)
                        printf("mem[%03d] = 0\n", zero_ini);
                    else
                        printf("mem[%03d–%03d] = 0\n", zero_ini, i - 1);
                    zero_ini = -1;
                }
                printf("mem[%03d] = %d\n", i, val);
            }
            i++;
        }

        if (zero_ini != -1) {
            if (i - 1 == zero_ini)
                printf("mem[%03d] = 0\n", zero_ini);
            else
                printf("mem[%03d–%03d] = 0\n", zero_ini, i - 1);
        }

        fclose(verif);
    } else {
        printf("[erro] não foi possível reabrir '%s'\n", arquivo_saida);
    }

    // recompila a máquina virtual
    printf("[montador] compilando mv.c...\n");
    int compile_status = system("gcc src/mv.c -o bin/mv");
    if (compile_status != 0) {
        printf("[erro] falha na compilação da máquina virtual.\n");
        return 1;
    }

    // você pode liberar isso aqui depois pra aceitar argv também
    printf("[montador] executando a MV...\n");
    system("./bin/mv");

    return 0;
}

