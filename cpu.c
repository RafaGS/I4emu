/*
 * cpu.c
 *
 * Copyright (c) 2026 Domenico Calò
 * Proyecto original: https://github.com/Nori-0/intel-4004-emulator
 *
 * Adaptado por RafaGS para I4emu (Intellec 4 Emulator)
 *
 * Este archivo está licenciado bajo la GPLv2. El código original del cual
 * deriva este archivo estaba licenciado bajo la licencia MIT.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: 2026 Domenico Calò
 * SPDX-FileCopyrightText: 2026 Rafa Gómez
 */

#include <stddef.h>
#include "cpu.h"
#include <avr/pgmspace.h>

// ============================================================================
// LECTURA DE MEMORIA ROM (FLASH DE AVR)
// ============================================================================

uint8_t fetch_rom(MCS4_System* sys, uint16_t address) {
    if (sys->active_rom_ptr != NULL && address < sys->active_rom_size) {
        // Lectura directa desde la memoria RAM (SRAM)
        return sys->active_rom_ptr[address];
    }
    return 0x00; // NOP si está fuera de rango
}

// ============================================================================
// INICIALIZACIÓN DEL SISTEMA Y REGISTROS
// ============================================================================

void init_system(MCS4_System* sys) {
    sys->cpu.pc = 0x000;
    sys->cpu.sp = 0;
    sys->cpu.acc = 0;
    sys->cpu.carry = 0;
    sys->cpu.halted = false;

    for (int i = 0; i < 3; i++) {
        sys->cpu.stack[i] = 0x000;
    }

    for (int i = 0; i < 16; i++) {
        sys->cpu.regs[i] = 0;
    }

    // Inicialización limpia de la RAM 4002 emulada
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            sys->ram_chips[i].main_ram[j] = 0;
        }
        for (int j = 0; j < 4; j++) {
            sys->ram_chips[i].status_ram[j] = 0;
        }
        sys->ram_chips[i].output_port = 0;
    }
}

// ============================================================================
// DECODIFICACIÓN Y EJECUCIÓN DE INSTRUCCIONES
// ============================================================================

void execute_instruction(MCS4_System* sys) {
    if (sys->cpu.halted) return;

    // Fetch del Opcode
    uint8_t op = fetch_rom(sys, sys->cpu.pc++);

    // Detección de parada / Fin de ROM (Opcode 0x00 TRAP)
    if (op == 0x00 && sys->cpu.pc > 1) {
        sys->cpu.halted = true;
        return;
    }

    uint8_t high = (op >> 4) & 0x0F;
    uint8_t low  = op & 0x0F;

    switch (high) {
        case 0x0: // NOP
            break;

        case 0x1: { // JCN (Jump Conditional)
            uint8_t cond = low;
            uint8_t target_addr = fetch_rom(sys, sys->cpu.pc++);
            
            bool invert = (cond & 0x08) != 0;
            bool cond_acc_zero = (cond & 0x04) && (sys->cpu.acc == 0);
            bool cond_carry = (cond & 0x02) && (sys->cpu.carry == 1);
            bool cond_test = (cond & 0x01) && false; // Pin TEST no usado

            bool jump_condition = cond_acc_zero || cond_carry || cond_test;
            if (invert) jump_condition = !jump_condition;

            if (jump_condition) {
                // El salto mantiene la página actual del PC
                sys->cpu.pc = (sys->cpu.pc & 0xF00) | target_addr;
            }
            break;
        }

        case 0x2: { // FIM (Fetch Immediate Pair) o SRC
            if ((low & 0x01) == 0) { // FIM
                uint8_t data = fetch_rom(sys, sys->cpu.pc++);
                uint8_t reg_pair = low & 0x0E;
                sys->cpu.regs[reg_pair]     = (data >> 4) & 0x0F;
                sys->cpu.regs[reg_pair + 1] = data & 0x0F;
            } else { // SRC (Send Register Control)
                // Usado para direccionamiento de memoria/puertos
            }
            break;
        }

        case 0x3: { // FIN o JIN
            if ((low & 0x01) == 0) { // FIN
                uint8_t reg_pair = low & 0x0E;
                uint16_t addr = (sys->cpu.pc & 0xF00) | ((sys->cpu.regs[0] << 4) | sys->cpu.regs[1]);
                uint8_t data = fetch_rom(sys, addr);
                sys->cpu.regs[reg_pair]     = (data >> 4) & 0x0F;
                sys->cpu.regs[reg_pair + 1] = data & 0x0F;
            } else { // JIN
                uint8_t reg_pair = low & 0x0E;
                uint16_t target = (sys->cpu.regs[reg_pair] << 4) | sys->cpu.regs[reg_pair + 1];
                sys->cpu.pc = (sys->cpu.pc & 0xF00) | target;
            }
            break;
        }

        case 0x4: { // JUN (Jump Unconditional)
            uint8_t data = fetch_rom(sys, sys->cpu.pc++);
            uint16_t target = ((uint16_t)low << 8) | data;
            sys->cpu.pc = target & 0x0FFF;
            break;
        }

        case 0x5: { // JMS (Jump to Subroutine)
            uint8_t data = fetch_rom(sys, sys->cpu.pc++);
            uint16_t target = ((uint16_t)low << 8) | data;
            
            // Push a la pila de 3 niveles
            if (sys->cpu.sp < 3) {
                sys->cpu.stack[sys->cpu.sp++] = sys->cpu.pc;
            }
            sys->cpu.pc = target & 0x0FFF;
            break;
        }

        case 0x6: // INC (Increment Register)
            sys->cpu.regs[low] = (sys->cpu.regs[low] + 1) & 0x0F;
            break;

        case 0x7: { // ISZ (Increment and Skip if Zero)
            uint8_t target_addr = fetch_rom(sys, sys->cpu.pc++);
            sys->cpu.regs[low] = (sys->cpu.regs[low] + 1) & 0x0F;
            if (sys->cpu.regs[low] != 0) {
                sys->cpu.pc = (sys->cpu.pc & 0xF00) | target_addr;
            }
            break;
        }

        case 0x8: { // ADD
            uint8_t sum = sys->cpu.acc + sys->cpu.regs[low] + sys->cpu.carry;
            sys->cpu.acc = sum & 0x0F;
            sys->cpu.carry = (sum > 15) ? 1 : 0;
            break;
        }

        case 0x9: { // SUB
            uint8_t reg_val = sys->cpu.regs[low];
            uint8_t borrow = sys->cpu.carry ? 0 : 1;
            int diff = sys->cpu.acc - reg_val - borrow;
            if (diff < 0) {
                sys->cpu.acc = (diff + 16) & 0x0F;
                sys->cpu.carry = 0; // Borrow
            } else {
                sys->cpu.acc = diff & 0x0F;
                sys->cpu.carry = 1; // No borrow
            }
            break;
        }

        case 0xA: // LD (Load)
            sys->cpu.acc = sys->cpu.regs[low];
            break;

        case 0xB: // XCH (Exchange)
            {
                uint8_t tmp = sys->cpu.acc;
                sys->cpu.acc = sys->cpu.regs[low];
                sys->cpu.regs[low] = tmp;
            }
            break;

        case 0xC: { // BBL (Branch Back and Load)
            if (sys->cpu.sp > 0) {
                sys->cpu.pc = sys->cpu.stack[--sys->cpu.sp];
            }
            sys->cpu.acc = low;
            break;
        }

        case 0xD: // LDM (Load Immediate)
            sys->cpu.acc = low;
            break;

        case 0xE: // Operaciones de E/S / RAM
            // En un sistema extendido, aquí se procesan las instrucciones RDM, RDR, WRR, etc.
            break;

        case 0xF: // Operaciones del Acumulador y Carry
            switch (low) {
                case 0x0: sys->cpu.acc = 0; break;                           // CLB
                case 0x1: sys->cpu.carry = 0; break;                         // CLC
                case 0x2: sys->cpu.acc = 0; sys->cpu.carry = 0; break;       // IAC (Complement Carry/Acc)
                case 0x3: sys->cpu.carry = 1; break;                         // CMC
                case 0x4: sys->cpu.acc = (~sys->cpu.acc) & 0x0F; break;      // CMA
                case 0x5: {                                                  // RAL (Rotate Left)
                    uint8_t new_carry = (sys->cpu.acc >> 3) & 0x01;
                    sys->cpu.acc = ((sys->cpu.acc << 1) | sys->cpu.carry) & 0x0F;
                    sys->cpu.carry = new_carry;
                    break;
                }
                case 0x6: {                                                  // RAR (Rotate Right)
                    uint8_t new_carry = sys->cpu.acc & 0x01;
                    sys->cpu.acc = (sys->cpu.acc >> 1) | (sys->cpu.carry << 3);
                    sys->cpu.carry = new_carry;
                    break;
                }
                case 0x7: sys->cpu.acc = (sys->cpu.acc + 1) & 0x0F;          // TCC
                          if (sys->cpu.acc == 0) sys->cpu.carry = 1;
                          break;
                case 0x8: sys->cpu.acc = (sys->cpu.acc + 1) & 0x0F;          // DAC
                          sys->cpu.carry = (sys->cpu.acc == 0) ? 0 : 1;
                          break;
                case 0x9: sys->cpu.acc = (sys->cpu.acc + sys->cpu.carry) & 0x0F; // TCS
                          sys->cpu.carry = 0;
                          break;
                case 0xA: sys->cpu.acc = sys->cpu.carry; break;              // STC
                case 0xB: sys->cpu.acc = (sys->cpu.acc + 6) & 0x0F; break;   // DAA
                case 0xC: sys->cpu.acc = (sys->cpu.acc + 10) & 0x0F; break;  // KAB
                default: break;
            }
            break;
    }
}
