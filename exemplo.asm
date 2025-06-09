val1:   .word 0
val2:   .word 0
soma:   .word 0
ctrl:   .word 3

INICIO:
    r val1
    r val2

    mv a2 val1
    mv a3 val2
    add a0 a2 a3
    st a0 soma
    w soma

    mv a1 ctrl
    sub a1 a1 a3
    st a1 ctrl

    jgt a1 INICIO

    stp
