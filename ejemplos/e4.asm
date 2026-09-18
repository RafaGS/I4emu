// Carga en Cascada de Registros (R0, R1, R2):
// - Utiliza instrucciones LDM e intercambio XCH para poblar secuencialmente los registros R0=1, R1=2 y R2=3.
// - Valida el volcado de registros en el monitor. 
LDM 1
XCH R0
LDM 2
XCH R1
LDM 3
XCH R2 
NOP
