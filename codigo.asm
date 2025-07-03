GLOBAL _start
GLOBAL _val_to_print
EXTERN _print_val

_start:
    mv a0 val1
    mv a1 val2
    add a2 a0 a1
    st a2 _val_to_print
    jmp _print_val

val1: .word 10
val2: .word 20
_val_to_print: .word 0
