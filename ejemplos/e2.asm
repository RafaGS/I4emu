// Sumador de 4 Bits (7 + 8 = 15):
// - Carga los valores 7 y 8 en el par de registros R0R1, mueve R0 al acumulador y le suma R1. 
// - Permite comprobar el flag de Carry y el acumulador al ejecutar R.
FIM R0R1, 0x78
LD R0
ADD R1
NOP
