#include <stdio.h>
#include <stdlib.h>

int mem[320];
int regs[4] = {0, 0, 0, 0}; // a0 = regs[0], a1 = regs[1], etc
int PC = 0;

void executar() {
    printf("\nExecutando...\n");

    while (1) {
        int opcode = mem[PC++];

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
                break;
            }
            case 2: { // mul
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                regs[r0] = regs[r1] * regs[r2];
                break;
            }
            case 3: { // div
                int r0 = mem[PC++];
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                if (regs[r2] != 0) {
                    regs[r0] = regs[r1] / regs[r2];
                } else {
                    printf("Erro: divisão por zero\n");
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
                PC = pos;
                break;
            }
            case 7: { // jeq r1 r2 pos
                int r1 = mem[PC++];
                int r2 = mem[PC++];
                int pos = mem[PC++];
                if (regs[r1] == regs[r2]) PC = pos;
                break;
            }
            case 8: { // jgt r pos
                int r = mem[PC++];
                int pos = mem[PC++];
                if (regs[r] > 0) PC = pos;
                break;
            }
            case 9: { // jlt r pos
                int r = mem[PC++];
                int pos = mem[PC++];
                if (regs[r] < 0) PC = pos;
                break;
            }
            case 10: { // w pos
                int pos = mem[PC++];
                printf("w: mem[%d] = %d\n", pos, mem[pos]);
                break;
            }
            case 11: { // r pos
                int pos = mem[PC++];
                printf("Digite valor para mem[%d]: ", pos);
                scanf("%d", &mem[pos]);
                break;
            }
            case 12: // stp
                printf("stp: fim da execução\n");
                return;
            default:
                printf("Instrução inválida: %d\n", opcode);
                return;
        }
    }
}

int main(int argc, char *argv[]) {
    const char *arquivo_nome = (argc >= 2) ? argv[1] : "exercicio";

    FILE *arquivo = fopen(arquivo_nome, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo: %s\n", arquivo_nome);
        return 1;
    }

    int valor, pos = 0;
    while (fscanf(arquivo, "%d", &valor) != EOF && pos < 320) {
        mem[pos++] = valor;
    }
    fclose(arquivo);

    executar();
    return 0;
}

