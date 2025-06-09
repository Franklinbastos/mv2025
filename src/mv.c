#include <stdio.h>
#include <stdlib.h>

int mem[320];
int regs[4] = {0, 0, 0, 0}; // a0 = regs[0], a1 = regs[1], etc
int PC = 0;

void dump_estado() {
    printf("\n[estado] PC: %d | a0: %d | a1: %d | a2: %d | a3: %d\n",
           PC, regs[0], regs[1], regs[2], regs[3]);
}

void executar() {
    printf("\n[mv] executando...\n");

    while (1) {
        int opcode = mem[PC++];
        printf("\n[mv] PC=%d | opcode=%d\n", PC - 1, opcode);

        switch (opcode) {
            case 0: { // add
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                int result = regs[r1] + regs[r2];
                regs[r0] = result;
                printf("add: regs[%d] = regs[%d] + regs[%d] → %d + %d = %d\n",
                       r0, r1, r2, regs[r1], regs[r2], result);
                break;
            }
            case 1: { // sub
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                regs[r0] = regs[r1] - regs[r2];
                printf("sub: regs[%d] = %d - %d = %d\n", r0, regs[r1], regs[r2], regs[r0]);
                break;
            }
            case 2: { // mul
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                regs[r0] = regs[r1] * regs[r2];
                printf("mul: regs[%d] = %d * %d = %d\n", r0, regs[r1], regs[r2], regs[r0]);
                break;
            }
            case 3: { // div
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                if (regs[r2] != 0) {
                    regs[r0] = regs[r1] / regs[r2];
                    printf("div: regs[%d] = %d / %d = %d\n", r0, regs[r1], regs[r2], regs[r0]);
                } else {
                    printf("[erro] divisão por zero: regs[%d] / regs[%d]\n", r1, r2);
                    exit(1);
                }
                break;
            }
            case 4: { // mv reg = mem[pos]
                int r = mem[PC++];
                int pos = mem[PC++];
                regs[r] = mem[pos];
                printf("mv: regs[%d] = mem[%d] → %d\n", r, pos, mem[pos]);
                break;
            }
            case 5: { // st mem[pos] = reg
                int r = mem[PC++];
                int pos = mem[PC++];
                mem[pos] = regs[r];
                printf("st: mem[%d] = regs[%d] → %d\n", pos, r, regs[r]);
                break;
            }
            case 6: { // jmp
                int pos = mem[PC++];
                printf("jmp: PC ← %d\n", pos);
                PC = pos;
                break;
            }
            case 7: { // jeq r1 r2 pos
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                int pos = mem[PC++];
                printf("jeq: if regs[%d] == regs[%d] → %d == %d\n", r1, r2, regs[r1], regs[r2]);
                if (regs[r1] == regs[r2]) {
                    PC = pos;
                    printf(" → salto para %d\n", pos);
                }
                break;
            }
            case 8: { // jgt r pos
                int r = mem[PC++];
                int pos = mem[PC++];
                printf("jgt: if regs[%d] > 0 → %d > 0\n", r, regs[r]);
                if (regs[r] > 0) {
                    PC = pos;
                    printf(" → salto para %d\n", pos);
                }
                break;
            }
            case 9: { // jlt r pos
                int r = mem[PC++];
                int pos = mem[PC++];
                printf("jlt: if regs[%d] < 0 → %d < 0\n", r, regs[r]);
                if (regs[r] < 0) {
                    PC = pos;
                    printf(" → salto para %d\n", pos);
                }
                break;
            }
            case 10: { // w pos
                int pos = mem[PC++];
                printf("w: mem[%d] = %d\n", pos, mem[pos]);
                break;
            }
            case 11: { // r pos
                int pos = mem[PC++];
                printf("r: digite valor para mem[%d]: ", pos);
                scanf("%d", &mem[pos]);
                break;
            }
            case 12: {
                printf("stp: fim da execução\n");
                dump_estado();
                return;
            }
            default:
                printf("[erro] opcode inválido: %d\n", opcode);
                dump_estado();
                return;
        }

        dump_estado(); // mostra estado ao final de cada instrução
    }
}

int main(int argc, char *argv[]) {
    const char *arquivo_nome = (argc >= 2) ? argv[1] : "exercicio";

    printf("[mv] carregando programa de '%s'...\n", arquivo_nome);
    FILE *arquivo = fopen(arquivo_nome, "r");
    if (!arquivo) {
        printf("[erro] falha ao abrir '%s'\n", arquivo_nome);
        return 1;
    }

    int valor, pos = 0;
    while (fscanf(arquivo, "%d", &valor) != EOF && pos < 320) {
        mem[pos++] = valor;
    }
    fclose(arquivo);
    printf("[mv] carregamento concluído: %d instruções lidas\n", pos);

    executar();
    return 0;
}