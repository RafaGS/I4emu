#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ESTRUCTURAS DEL PROCESADOR INTEL 4004 Y CHIPS RAM 4002
// ============================================================================

typedef struct {
    uint16_t pc;        // Program Counter (12 bits)
    uint16_t stack[3];  // Pila de retorno de 3 niveles (12 bits c/u)
    uint8_t sp;         // Puntero de pila (0 a 3)
    uint8_t acc;        // Acumulador (4 bits)
    uint8_t carry;      // Banderas / Carry (1 bit)
    uint8_t regs[16];   // Registros de propósito general R0-R15 (4 bits c/u)
    bool halted;        // Bandera de fin de programa / trampa para el Monitor
} CPU_4004;

typedef struct {
    uint8_t main_ram[16];   // 16 caracteres de 4 bits por registro
    uint8_t status_ram[4];  // 4 caracteres de estado
    uint8_t output_port;    // Puerto de salida del chip 4002
} RAM_4002;

typedef struct {
    CPU_4004 cpu;
    RAM_4002 ram_chips[4];      // Hasta 4 bancos/chips RAM emulados
    const uint8_t* active_rom_ptr; // Puntero a la ROM seleccionada en Flash (PROGMEM)
    uint16_t active_rom_size;   // Tamaño del binario cargado
} MCS4_System;

// ============================================================================
// PROTOTIPOS DE FUNCIONES
// ============================================================================

/**
 * Inicializa el estado de la CPU, registros y memoria RAM.
 */
void init_system(MCS4_System* sys);

/**
 * Obtiene el byte de la ROM desde la memoria Flash (PROGMEM).
 */
uint8_t fetch_rom(MCS4_System* sys, uint16_t address);

/**
 * Decodifica y ejecuta una instrucción completa del 4004.
 */
void execute_instruction(MCS4_System* sys);

#ifdef __cplusplus
}
#endif

#endif // CPU_H
