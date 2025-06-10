#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_MEM 320

// carrega um binário para uma posição inicial na memória
void carregar_binario(const char *arquivo, int *mem, int offset) {
    FILE *fp = fopen(arquivo, "r");
    if (!fp) {
        printf("[ligador] erro ao abrir %s\n", arquivo);
        exit(1);
    }

    int val, pos = offset;
    while (fscanf(fp, "%d", &val) != EOF) {
        mem[pos++] = val;
    }

    fclose(fp);
    printf("[ligador] carregado %s em offset %d até %d\n", arquivo, offset, pos - 1);
}

// salva a memória final no arquivo de saída
void salvar_memoria(const char *arquivo_saida, int *mem) {
    FILE *out = fopen(arquivo_saida, "w");
    if (!out) {
        printf("[ligador] erro ao criar %s\n", arquivo_saida);
        exit(1);
    }

    for (int i = 0; i < TAM_MEM; i++) {
        fprintf(out, "%d\n", mem[i]);
    }

    fclose(out);
    printf("[ligador] binário final salvo em %s\n", arquivo_saida);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("uso: %s bin1 bin2 [...]\n", argv[0]);
        return 1;
    }

    int memoria[TAM_MEM] = {0};
    int offset = 0;

    for (int i = 1; i < argc; i++) {
        carregar_binario(argv[i], memoria, offset);

        // avança offset até o fim do programa atual
        FILE *fp = fopen(argv[i], "r");
        int val;
        while (fscanf(fp, "%d", &val) != EOF) offset++;
        fclose(fp);
    }

    salvar_memoria("../exercicio_final", memoria);

    return 0;
}
