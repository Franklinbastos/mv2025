num1:    .word 10
num2:    .word 5
res:     .word 0
cmp:     .word 0
val_100: .word 100
val_200: .word 200
val_300: .word 300
temp:    .word 0

START:
    r num1
    r num2

    mv a0 num1
    mv a1 num2
    sub a2 a0 a1
    st a2 cmp

    jeq a0 a1 IGUAL   ; se forem iguais
    jgt a2 MAIOR      ; se a0 > a1 → a2 > 0
    jmp MENOR         ; se não caiu em nenhum, é menor

IGUAL:
    mv a3 val_100
    jmp FIM

MAIOR:
    mv a3 val_200
    jmp FIM

MENOR:
    mv a3 val_300

FIM:
    st a3 res
    w res
    stp
