GLOBAL _start          
GLOBAL _val_to_print   
GLOBAL label
EXTERN _print_val      


_start:
    mv a0 val1         // move o val1 pro registrador a0
    mv a1 val2         // move o val2 pro registrador a1
    add a2 a0 a1       // soma a0 + a1 e poe em a2
    st a2 _val_to_print // armazena a2 no endereço _val_to_print
    jmp _print_val    

val1: .word 10         // reserva uma posição de memória chamada val1 e coloca o valor 10
val2: .word 20         
_val_to_print: .word 0 // reserva espaço para a soma, inicia com 0

label:
w val1
  stp


