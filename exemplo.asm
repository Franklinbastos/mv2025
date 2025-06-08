val1: .word 10
val2: .word 20
result: .word 0

mv a2 val1
mv a3 val2
add a0 a2 a3
st a0 result
w result
stp
