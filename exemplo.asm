val1: .word 7
val2: .word 7
result: .word 0

mv a2 val1
mv a3 val2
jeq a2 a3 IGUAL
mv a0 val1
mv a0 111
jmp FIM

IGUAL:
mv a0 999

FIM:
st a0 result
w result
stp
