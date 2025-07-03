#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/linker_defs.h"

#define MAX_SYMBOLS 100
#define MAX_LINE 100


ObjectFile obj_file;


// procura simbolo na use_table e se nao tem adiciona.
// retorna indice do simbolo na use_table.
int adiciona_em_use_table(const char* nome) {
    // percorre tabela pra ver se tem
    for (int i = 0; i < obj_file.use_cont; i++) {
        if (strcmp(obj_file.use_table[i].nome, nome) == 0) {
            return i; // retorna indice
        }
    }

    // se não achou, adiciona novo
    if (obj_file.use_cont >= MAX_SYMBOLS) {
        printf("[montador] Erro: Limite de símbolos de uso excedido.\n");
        exit(1);
    }
    strcpy(obj_file.use_table[obj_file.use_cont].nome, nome);
    printf("[use] símbolo externo: %s\n", nome);
    return obj_file.use_cont++; // retorna índice do adicionado
}

int buscar_rotulo(const char *nome) {
    for (int i = 0; i < obj_file.def_cont; i++) {
        if (strcmp(obj_file.def_table[i].nome, nome) == 0)
            return obj_file.def_table[i].endereco;
    }
    printf("[erro] label '%s' não encontrada!\n", nome);
    return -1;
}

// função interna que trata números ou labels
int traduz_ou_busca(const char *arg, int current_address) {
    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1])))
        return atoi(arg); //ascii pra inteiro

    // descobre se e local
    int address = buscar_rotulo(arg);
    if (address != -1) {
        //registra na tabela de relocação
        if (obj_file.rel_cont >= MAX_SYMBOLS) {
             printf("[montador] Erro: Limite de entradas na tabela de relocação excedido.\n");
             exit(1);
        }

        obj_file.rel_table[obj_file.rel_cont].endereco = current_address;  // endereco do simbolo
        obj_file.rel_table[obj_file.rel_cont].simbolo_index = -1; // como e local seta como -1
        obj_file.rel_table[obj_file.rel_cont].tipo = 0; //relativa
        obj_file.rel_cont++;

        printf("[relativo] símbolo local '%s' na posição %d\n", arg, current_address);
        return address; // já retorna o endereço calculado do label local
    }

    // se for extern
    int symbol_index = adiciona_em_use_table(arg);

    if (obj_file.rel_cont >= MAX_SYMBOLS) {
         printf("[montador] Erro: Limite de entradas na tabela de relocação excedido.\n");
         exit(1);
    }

    obj_file.rel_table[obj_file.rel_cont].endereco = current_address;  // onde o valor vai ficar no binário
    obj_file.rel_table[obj_file.rel_cont].simbolo_index = symbol_index; // índice na tabela de uso
    obj_file.rel_table[obj_file.rel_cont].tipo = 1; // absoluta
    obj_file.rel_cont++;

    printf("[externo] símbolo '%s' na posição %d\n", arg, current_address);

    return 0;
}

// adiciona um novo label ou atualiza GLOBAL na tabela de símbolos locais
void adicionar_rotulo(const char *nome, int endereco, int tipo) {
    // verifica se o label já existe na tabela
    for (int i = 0; i < obj_file.def_cont; i++) {
        if (strcmp(obj_file.def_table[i].nome, nome) == 0) {//ve se tem na tabela
            // se sim atualiza end
            if (obj_file.def_table[i].tipo == 0 && obj_file.def_table[i].endereco == 0) { 
                obj_file.def_table[i].endereco = endereco;
                printf("[label] %s (GLOBAL) atualizado para o endereço %d\n", nome, endereco);
                return;
            }
            printf("[erro] label redefinida: %s\n", nome);
            exit(1);
        }
    }

    // verifica limite da tabela
    if (obj_file.def_cont >= MAX_SYMBOLS) {
        printf("[montador] Erro: Limite de símbolos de definição excedido.\n");
        exit(1);
    }

    // adiciona novo símbolo à tabela de definições
    strcpy(obj_file.def_table[obj_file.def_cont].nome, nome);
    obj_file.def_table[obj_file.def_cont].endereco = endereco;
    obj_file.def_table[obj_file.def_cont].tipo = tipo;

    printf("[label] %s = %d\n", nome, endereco);
    obj_file.def_cont++;
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

// faz a primeira varredura no arquivo ASM:
// calcula endereços das instruções e registra labels, GLOBALs e EXTERNs
void primeira_passagem(FILE *fp) {
    char linha[MAX_LINE];
    int end_instrucao = 0;
    memset(&obj_file, 0, sizeof(ObjectFile)); // zera struct do objeto

    while (fgets(linha, sizeof(linha), fp)) {
        char *comentario = strstr(linha, "//");
        if (comentario) *comentario = '\0'; // remove comentários

        // pega a primeira palavra 
        char *token = strtok(linha, " \t\n");
        if (!token) continue;

        if (strcmp(token, "GLOBAL") == 0) {
            // GLOBAL reserva nome na tabela de definições
            char* simbolo = strtok(NULL, " \t\n");
            if (buscar_rotulo(simbolo) != -1) {
                printf("[erro] Símbolo GLOBAL '%s' já definido como label local.\n", simbolo);
                exit(1);
            }
            adicionar_rotulo(simbolo, 0, 0);
            continue;
        }

        if (strcmp(token, "EXTERN") == 0) {
            // EXTERN coloca na tabela de uso
            char* simbolo = strtok(NULL, " \t\n");
            adiciona_em_use_table(simbolo);
            continue;
        }

        if (strchr(token, ':')) {
            // é uma label local
            token[strlen(token) - 1] = '\0';
            adicionar_rotulo(token, end_instrucao, 0);
            token = strtok(NULL, " \t\n");
            if (!token) continue;
        }

        if (strcmp(token, ".word") == 0) {
            // dados ocupam 1 posição
            end_instrucao += 1;
            continue;
        }

        int opcodigo = traduz_instrucao(token);
        if (opcodigo == -1) continue;

        // ajusta PC conforme instrução
        int usado = 1;
        if (opcodigo <= 3) usado += 3;          // add, sub, mul, div
        else if (opcodigo == 4 || opcodigo == 5) usado += 2; // mv, st
        else if (opcodigo == 6) usado += 1;     // jmp
        else if (opcodigo == 7) usado += 3;     // jeq
        else if (opcodigo == 8 || opcodigo == 9) usado += 2; // jgt, jlt
        else if (opcodigo == 10 || opcodigo == 11) usado += 1; // w, r

        end_instrucao += usado;
    }
    obj_file.tamanho_codigo = end_instrucao;
    printf("[info] tamanho do código: %d\n", end_instrucao);
}

// monta o código binário na memória, resolve operandos  
// grava o objeto final no arquivo de saída
void segunda_passagem(FILE *entrada, FILE *saida) {
    rewind(entrada);  // volta pro início do arquivo

    char linha[MAX_LINE];
    int pc_instrucao = 0;  // contador do endereço/instrucao atual

    while (fgets(linha, sizeof(linha), entrada)) {
        char *comentario = strstr(linha, "//");  // corta comentário
        if (comentario) *comentario = '\0';

        char *token = strtok(linha, " \t\n");  // pega primeiro token
        if (!token || strcmp(token, "GLOBAL") == 0 || strcmp(token, "EXTERN") == 0)
            continue;  // ignora linhas vazias GLOBAL/EXTERN na 2a passagem

        if (strchr(token, ':')) {  // se linha começa com label
            token = strtok(NULL, " \t\n");  // pula label, pega próxima instrução
            if (!token) continue;  // linha só com label, pula
        }

        if (strcmp(token, ".word") == 0) {  // trata .word
            token = strtok(NULL, " \t\n");
            if (token) {
                obj_file.codigo[pc_instrucao] = atoi(token);  // converte número
                printf("[word] mem[%03d] = %d\n", pc_instrucao, obj_file.codigo[pc_instrucao]);
                pc_instrucao++;
            }
            continue;
        }

        int opcodigo = traduz_instrucao(token);
        if (opcodigo == -1) continue;  // instr inválida, ignora

        obj_file.codigo[pc_instrucao++] = opcodigo;  // salva opcodigo

        // dependendo do opcode, busca e traduz os operandos
        if (opcodigo <= 3) {  // add, sub, mul, div: 3 registradores
            obj_file.codigo[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            obj_file.codigo[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
            obj_file.codigo[pc_instrucao++] = traduz_reg(strtok(NULL, " \t\n"));
        } else if (opcodigo >= 4 && opcodigo <= 11) {
            char* arg1 = strtok(NULL, " \t\n");
            if (opcodigo == 4 || opcodigo == 5 || opcodigo == 8 || opcodigo == 9) {
                // mv, st, jgt, jlt: 1 reg + 1 endereço (label ou número)
                obj_file.codigo[pc_instrucao++] = traduz_reg(arg1);
                char* arg2 = strtok(NULL, " \t\n");
                obj_file.codigo[pc_instrucao++] = traduz_ou_busca(arg2, pc_instrucao);
            } else if (opcodigo == 6 || opcodigo == 10 || opcodigo == 11) {
                // jmp, w, r: 1 endereço
                obj_file.codigo[pc_instrucao++] = traduz_ou_busca(arg1, pc_instrucao);
            } else if (opcodigo == 7) {
                // jeq: 2 regs + 1 endereço
                obj_file.codigo[pc_instrucao++] = traduz_reg(arg1);
                char* arg2 = strtok(NULL, " \t\n");
                obj_file.codigo[pc_instrucao++] = traduz_reg(arg2);
                char* arg3 = strtok(NULL, " \t\n");
                obj_file.codigo[pc_instrucao++] = traduz_ou_busca(arg3, pc_instrucao);
            }
        }
    }

    // grava o objeto inteiro (tabela, código, etc) no arquivo de saída
    fwrite(&obj_file, sizeof(ObjectFile), 1, saida);
}

int main(int argc, char *argv[]) {
    // verifica se passou o arquivo.asm na linha de comando
    if (argc < 2) {
        printf("Uso: %s <arquivo.asm>\n", argv[0]);
        return 1;
    }

    const char *arquivo_entrada = argv[1];

    // monta o nome do arquivo de saída 
    char arquivo_saida[100];
    sprintf(arquivo_saida, "%s.o", arquivo_entrada);


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

    // abre arquivo .o para escrever o objeto binário
    FILE *saida = fopen(arquivo_saida, "wb");
    if (!saida) {
        printf("[erro] não foi possível criar o arquivo '%s'\n", arquivo_saida);
        fclose(fp);
        return 1;
    }

    segunda_passagem(fp, saida);

    fclose(fp);
    fclose(saida);

    // imprime o código gerado na tela se definido DEBUG_VISUAL
    printf("\n--- BINÁRIO FINAL GERADO (VISUAL) ---\n");
    for (int i = 0; i < obj_file.tamanho_codigo; i++) {
        printf("mem[%03d] = %d\n", i, obj_file.codigo[i]);
    }

    printf("[montador] montagem finalizada com sucessooo → '%s'\n", arquivo_saida);
    return 0;
}

