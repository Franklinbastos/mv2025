.word 0
.word 5
.word 10

INICIO:
mv a0 1
mv a1 2
add a2 a0 a1
st a2 0
w 0
jmp FIM

FIM:
stp