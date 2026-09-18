/*
 * I4emu: Intellec 4 Emulator (RAM Optimized)
 *
 * Created by RafaGS
 * more info at https://minibots.wordpress.com
 *
 * SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: 2026 Rafa Gómez
 */

#include <Arduino.h>
#include <avr/pgmspace.h>

extern "C" {
  #include "cpu.h"
}

// Secuencias de Escape ANSI para Colores
#define ANSI_RESET   "\033[0m"
#define ANSI_PROMPT  "\033[1;32m"
#define ANSI_ECHO    "\033[1;33m"
#define ANSI_HEADER  "\033[1;36m"
#define ANSI_ERROR   "\033[1;31m"
#define ANSI_INFO    "\033[0;35m"
#define ANSI_CLEAR   "\033[2J\033[H"

#define ROM_SIZE 512  // Reducido a 512 bytes (2x Intel 4001 ROMs) para liberar 512B de RAM

uint8_t user_rom_buffer[ROM_SIZE];
MCS4_System sys;
char shared_buf[64]; // Búfer único reutilizable para formateo de texto

// Helper para leer líneas por Serial sin usar String (evita fragmentar la RAM)
size_t read_serial_line(char* buffer, size_t max_len) {
    size_t pos = 0;
    while (true) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            if (c == '\r' || c == '\n') {
                break;
            } else if (pos < max_len - 1) {
                buffer[pos++] = c;
            }
        }
    }
    buffer[pos] = '\0';
    return pos;
}

// ============================================================================
// INTEL HEX PARSER & WRITER
// ============================================================================

uint8_t parse_hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

uint8_t parse_hex_byte(const char* s, int offset) {
    return (parse_hex_nibble(s[offset]) << 4) | parse_hex_nibble(s[offset + 1]);
}

void read_intel_hex_papertape() {
    Serial.println(F("\n=========================================="));
    Serial.println(F(" INTELLEC 4 - PAPER TAPE READER (Intel HEX)"));
    Serial.println(F("=========================================="));
    Serial.println(F("Paste .hex lines below. End with EOF (:00000001FF)\n"));

    memset(user_rom_buffer, 0x00, sizeof(user_rom_buffer));
    uint16_t bytes_loaded = 0;
    bool loading = true;

    while (loading) {
        if (Serial.available() > 0) {
            read_serial_line(shared_buf, sizeof(shared_buf));

            if (shared_buf[0] != '\0') {
                Serial.print(F(ANSI_ECHO));
                Serial.println(shared_buf);
                Serial.print(F(ANSI_RESET));
            }

            if (shared_buf[0] == ':') {
                uint8_t len = parse_hex_byte(shared_buf, 1);
                uint16_t addr = (parse_hex_byte(shared_buf, 3) << 8) | parse_hex_byte(shared_buf, 5);
                uint8_t type = parse_hex_byte(shared_buf, 7);

                if (type == 1) {
                    Serial.println(F("\n[+] EOF record detected. Paper tape read complete."));
                    loading = false;
                } 
                else if (type == 0) {
                    for (uint8_t i = 0; i < len; i++) {
                        if ((addr + i) < sizeof(user_rom_buffer)) {
                            user_rom_buffer[addr + i] = parse_hex_byte(shared_buf, 9 + (i * 2));
                            bytes_loaded++;
                        }
                    }
                    Serial.print(F("."));
                }
            }
        }
    }

    Serial.print(F("\n[+] Total bytes written to memory: "));
    Serial.println(bytes_loaded);
}

void export_intel_hex_papertape() {
    Serial.println(F("\n=========================================="));
    Serial.println(F(" INTELLEC 4 - PAPER TAPE PUNCH (Intel HEX)"));
    Serial.println(F("=========================================="));

    uint16_t total_len = sizeof(user_rom_buffer);
    uint16_t addr = 0;

    while (addr < total_len) {
        uint8_t line_len = 16;
        if (addr + line_len > total_len) line_len = total_len - addr;

        uint8_t checksum = line_len + (addr >> 8) + (addr & 0xFF) + 0x00;
        int pos = snprintf(shared_buf, sizeof(shared_buf), ":%02X%04X00", line_len, addr);

        for (uint8_t i = 0; i < line_len; i++) {
            uint8_t val = user_rom_buffer[addr + i];
            checksum += val;
            pos += snprintf(shared_buf + pos, sizeof(shared_buf) - pos, "%02X", val);
        }

        checksum = (uint8_t)(~checksum + 1);
        snprintf(shared_buf + pos, sizeof(shared_buf) - pos, "%02X", checksum);

        Serial.println(shared_buf);
        addr += line_len;
    }

    Serial.println(F(":00000001FF"));
    Serial.println(F("[+] Paper tape punch export complete.\n"));
}

// ============================================================================
// DUMP & SUBSTITUTE MEMORY
// ============================================================================

void dump_memory(uint16_t start_addr, uint16_t lines) {
    Serial.println(F("\n--- MEMORY DUMP ---"));
    Serial.println(F("ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
    Serial.println(F("-----------------------------------------------------"));

    for (uint16_t i = 0; i < lines * 16; i += 16) {
        uint16_t current_addr = start_addr + i;
        if (current_addr >= sizeof(user_rom_buffer)) break;

        snprintf(shared_buf, sizeof(shared_buf), "%04X: ", current_addr);
        Serial.print(shared_buf);

        for (uint8_t j = 0; j < 16; j++) {
            if ((current_addr + j) < sizeof(user_rom_buffer)) {
                snprintf(shared_buf, sizeof(shared_buf), "%02X ", user_rom_buffer[current_addr + j]);
                Serial.print(shared_buf);
            } else {
                Serial.print(F("   "));
            }
        }
        Serial.println();
    }
}

void substitute_memory() {
    dump_memory(0x0000, 4);

    Serial.print(F("\nEdit byte - Enter Address in HEX (000-1FF) or press ENTER to skip: "));
    read_serial_line(shared_buf, sizeof(shared_buf));

    if (shared_buf[0] != '\0') {
        Serial.print(F(ANSI_ECHO));
        Serial.println(shared_buf);
        Serial.print(F(ANSI_RESET));
    }
    else {
        Serial.println("");
        return;
    }

    uint16_t addr = (uint16_t) strtol(shared_buf, NULL, 16);
    if (addr >= sizeof(user_rom_buffer)) {
        Serial.println(F(ANSI_ERROR "Address out of range!" ANSI_RESET));
        return;
    }

    snprintf(shared_buf, sizeof(shared_buf), "Address %03X current val: %02X. Enter new HEX val: ", addr, user_rom_buffer[addr]);

    Serial.print(shared_buf);

    read_serial_line(shared_buf, sizeof(shared_buf));


    if (shared_buf[0] != '\0') {
        Serial.print(F(ANSI_ECHO));
        Serial.println(shared_buf);
        Serial.print(F(ANSI_RESET));
        uint8_t new_val = (uint8_t) strtol(shared_buf, NULL, 16);
        user_rom_buffer[addr] = new_val;
        snprintf(shared_buf, sizeof(shared_buf), "[+] Memory %03X updated to %02X\n", addr, new_val);
        Serial.println(shared_buf);
    }
    else {
        Serial.println("");
        return;
    }
}

// ============================================================================
// INSPECT REGISTERS
// ============================================================================

void inspect_registers() {
    Serial.println(F("\n--- CPU STATE & REGISTERS ---"));
    snprintf(shared_buf, sizeof(shared_buf), "PC: %03X | SP: %d | ACC: %X | C: %d | HALT: %s",
             sys.cpu.pc, sys.cpu.sp, sys.cpu.acc, sys.cpu.carry, sys.cpu.halted ? "YES" : "NO");
    Serial.println(shared_buf);

    Serial.println(F("Registers (4-bit pairs):"));
    for (uint8_t i = 0; i < 16; i += 2) {
        snprintf(shared_buf, sizeof(shared_buf), "  R%d: %X  |  R%d: %X", i, sys.cpu.regs[i], i + 1, sys.cpu.regs[i + 1]);
        Serial.println(shared_buf);
    }
}

// ============================================================================
// CLI AND EXECUTION LOOP
// ============================================================================

void print_menu() {
    Serial.println();
    Serial.print(F(ANSI_HEADER "\r========================================\r\n"));
    Serial.print(F("       I4Emu - Intellec 4 MONITOR       \r\n"));
    Serial.print(F("========================================" ANSI_RESET "\r\n"));
    Serial.print(F(" [R] Read Paper Tape / Intel HEX File\r\n"));
    Serial.print(F(" [S] Substitute / Display Memory Bytes\r\n"));
    Serial.print(F(" [W] Write / Export Memory to Intel HEX\r\n"));
    Serial.print(F(" [X] eXamine CPU State & Registers\r\n"));
    Serial.print(F(" [G] Go / Run Loaded Program\r\n"));
    Serial.print(F(" [A] About I4Emu\r\n"));
    Serial.print(F(ANSI_HEADER "========================================" ANSI_RESET "\r\n"));
    Serial.flush();
}

void print_prompt() {
    Serial.print(F(ANSI_PROMPT "INTELLEC-4> " ANSI_RESET));
    Serial.flush();
}

void print_about() {
    Serial.println();
    Serial.print(F(ANSI_INFO "\r****************************************\r\n"));
    Serial.print(F(" I4Emu - Intellec 4 emulator by RafaGS \r\n"));
    Serial.print(F(" under GPL 2.0 license \r\n"));
    Serial.print(F("\r\n More info at Minibots: \r\n"));  
    Serial.print(F(" <https://minibots.wordpress.com> \r\n"));   
    Serial.print(F("****************************************" ANSI_RESET "\r\n"));
    Serial.flush();
}

void run_program(const uint8_t* rom_ptr, uint16_t size) {
    init_system(&sys);
    sys.active_rom_ptr = rom_ptr;
    sys.active_rom_size = size;

    Serial.println(F("\n[+] Executing program..."));

    while (!sys.cpu.halted && sys.cpu.pc < size) {
        snprintf(shared_buf, sizeof(shared_buf), "PC:%03X | ACC:%X | C:%d | R0:%X R1:%X",
                 sys.cpu.pc, sys.cpu.acc, sys.cpu.carry, sys.cpu.regs[0], sys.cpu.regs[1]);
        Serial.println(shared_buf);

        execute_instruction(&sys);
        delay(250);
    }

    Serial.println(F("[!] Program Execution Halted. Returning to Monitor.\n"));
    delay(1000);
}

void setup() {
    Serial.begin(9600);
    delay(500);

    while (Serial.available() > 0) Serial.read();

    Serial.print(F(ANSI_CLEAR));
    Serial.flush();

    init_system(&sys);
    print_menu();
    print_prompt();
}

void loop() {
    if (Serial.available() > 0) {
        char opt = Serial.read();
        if (opt == '\n' || opt == '\r') return;

        Serial.print(F(ANSI_ECHO));
        Serial.println(opt);
        Serial.print(F(ANSI_RESET));

        if (opt == 'r' || opt == 'R') read_intel_hex_papertape();
        else if (opt == 's' || opt == 'S') substitute_memory();
        else if (opt == 'w' || opt == 'W') export_intel_hex_papertape();
        else if (opt == 'x' || opt == 'X') inspect_registers();
        else if (opt == 'g' || opt == 'G') run_program(user_rom_buffer, sizeof(user_rom_buffer));
        else if (opt == 'a' || opt == 'A') print_about();
        else if (opt == 'l' || opt == 'L') print_menu();       
        else Serial.println(F(ANSI_ERROR "Invalid option. Use [L] for list options." ANSI_RESET));

        print_prompt();
    }
}