val1:   .word  10
val2:   .word  5
soma:   .word  0
sub:    .word  0
mult:   .word  0
div:    .word  0
cmp:    .word  0

START:
    mv a0 val1
    mv a1 val2

    add a2 a0 a1
    st a2 soma

    sub a2 a0 a1
    st a2 sub

    mul a2 a0 a1
    st a2 mult

    div a2 a0 a1
    st a2 div

    sub a2 a0 a1
    st a2 cmp

    jeq a0 a1 IGUAL
    jgt a2 MAIOR
    jlt a2 MENOR

IGUAL:
    w soma
    jmp FIM

MAIOR:
    w sub
    jmp FIM

MENOR:
    w mult

FIM:
    w div
    stp
