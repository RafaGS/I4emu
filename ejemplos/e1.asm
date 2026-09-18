// Contador e Incrementador con Bucle (Bucle R0 + Incremento R1):
// - Carga R0=0 y R1=A (10). 
// - Incrementa R1 en cada iteración mientras R0 hace un bucle mediante ISZ hasta desbordar de F a 0. 
FIM R0R1, 0x0A
INC R1
ISZ R0, 0x02
NOP
