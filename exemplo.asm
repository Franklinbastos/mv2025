val1:  .word 8
val2:  .word 2
temp:  .word 0
res:   .word 0
input: .word 0

mv a2 val1
mv a3 val2

add a0 a2 a3
st a0 res

sub a0 a2 a3
st a0 res

mul a0 a2 a3
st a0 res

div a0 a2 a3
st a0 res

jmp FIM

SALTO:
mv a0 val1
st a0 temp

OK:
w res
r input

FIM:
stp
