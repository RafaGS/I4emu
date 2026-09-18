// Rotación de Bits y Manejo de Carry (RAL):
// - Carga el valor 0x9 (1001 en binario) en el acumulador, limpia el Carry y realiza dos rotaciones a la izquierda a través del Carry (RAL).
// - Permite verificar el desplazamiento de bits.
LDM 0x09
CLC
RAL
RAL
NOP
