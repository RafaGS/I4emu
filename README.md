# I4Emu - Intel 4004 / Intellec 4 AVR Emulator

**[English Version](#english-version) | [Versión en Español](#versi%25C3%25B3n-en-espa%25C3%25B1ol)**

## English Version

More information at [Minibots](https://minibots.wordpress.com).

This project is a lightweight, resource-constrained emulator of the classic Intel MCS-4 / Intellec 4 development system (based on the Intel 4004 CPU), specifically engineered to run on ATmega328P microcontrollers (Arduino UNO/Nano). It features an ANSI/VT100 interactive serial monitor compliant with Intel MON-4 standard commands and includes a Python-based assembly toolchain.

### Key Features

- **Optimized Intel 4004 Core:** Complete emulation of the 4-bit CPU architecture, including the Accumulator, Carry flag, 12-bit Program Counter (PC), 3-level call stack, and 16 4-bit index registers ($R_0$–$R_{15}$) configurable as register pairs ($P_0$ – $P_7$).

- **Zero-Heap Memory Management:** Tailored for the 2 KB SRAM limit of the ATmega328P. Avoids dynamic memory allocation (`String` objects) by using C-strings and a reusable global buffer (`shared_buf`), maintaining a static 512-byte ROM space.

- **Intel MON-4 Compliant Monitor:** Interactive CLI via UART serial interface supporting standard Intel monitor commands: **`R`** (Read Paper Tape), **`S`** (Substitute/Display Memory), **`W`** (Write Paper Tape), **`X`** (Examine/Modify Registers), **`G`** (Go/Run), and **`A`** (About).

- **ANSI/VT100 Terminal Interface:** Formatted terminal control featuring colorized prompt echoes (`ANSI_ECHO`), immediate input cancelation on empty returns, and out-of-range protection.

- **Paper Tape (Intel HEX) I/O:** Stream-based reader and writer for loading and dumping code using standard Intel HEX records (`:LLAAAATT[DATA]CC`) with real-time checksum validation.

### System Architecture

- **AVR Host Layer:** Runs directly on ATmega328P using standard `HardwareSerial` configured at 9600 baud.

- **Memory Bus Mapping:** 512-byte static array (`user_rom_buffer`) representing two Intel 4001 ROM chips ($256 \times 8$ bits each).

- **Fetch-Decode-Execute Loop:** Instruction decoder supporting 1-byte (8-bit) and 2-byte (16-bit) MCS-4 opcodes with execution state reporting.

### Toolchain & Supporting Utilities

- **`code2ihex.py`:** A Python assembler that translates Intel 4004 assembly source code directly into Intel HEX format. Features support for registers ($R_0$ – $R_{15}$), register pairs ($P_0$ – $P_7$), and automatic comment stripping (`;`, `//`).

### Quick Start Guide

1. **Compilation & Upload:** Compile and upload the firmware to your ATmega328P / Arduino UNO using Arduino IDE or CLI.

2. **Terminal Connection:** Open a serial terminal client configured to 9600 baud:
   
   Bash
   
   ```
   picocom -b 9600 /dev/ttyUSB0
   ```

3. **Assemble Code:** Generate Intel HEX from assembly source using the Python script:
   
   Bash
   
   ```
   python code2ihex.py program.asm
   ```

4. **Load & Run:**
   
   - In the `INTELLEC-4>` prompt, send **`R`** to start paper tape reading.
   
   - Paste the Intel HEX output into the terminal.
   
   - Send **`G`** to launch execution.

### Requirements

- **Hardware:** ATmega328P development board (Arduino UNO, Nano, or standalone MCU). No extra external circuitry required.

- **Software/Environment:** Arduino IDE or CLI (C++11 or higher).

- **Toolchain:** Python 3.x for `asm2ihex.py` and `code2ihex.py`.

### License

This project is licensed under the **GNU General Public License v2.0**. See the `LICENSE` file for more details.

## Versión en Español

Más información en [Minibots](https://minibots.wordpress.com).

Este proyecto es un emulador ligero del sistema de desarrollo clásico Intel MCS-4 / Intellec 4 (basado en el procesador Intel 4004), diseñado específicamente para ejecutarse en microcontroladores ATmega328P (Arduino UNO/Nano). Incluye un monitor serie interactivo ANSI/VT100 compatible con los comandos del estándar Intel MON-4 y una cadena de herramientas en Python.

### Características Principales

- **Núcleo Intel 4004 Optimizado:** Emulación de la arquitectura de 4 bits, incluyendo Acumulador, flag de Acarreo (Carry), Contador de Programa (PC) de 12 bits, pila de llamadas de 3 niveles y 16 registros de índice de 4 bits ($R_0$–$R_{15}$) configurables por pares ($P_0$ – $P_7$).

- **Gestión de Memoria Sin Heap:** Diseñado para ajustarse a los 2 KB de SRAM del ATmega328P. Elimina la asignación dinámica de memoria (`String`) empleando cadenas tipo C y un búfer global reutilizable (`shared_buf`), manteniendo un espacio estático de ROM de 512 bytes.

- **Monitor Compatible con Intel MON-4:** Interfaz CLI por puerto serie UART que implementa los comandos estándar de Intel: **`R`** (Read Paper Tape), **`S`** (Substitute/Display Memory), **`W`** (Write Paper Tape), **`X`** (Examine/Modify Registers), **`G`** (Go/Run) y **`A`** (About).

- **Interfaz de Terminal ANSI/VT100:** Formateo de terminal con eco de entrada en color (`ANSI_ECHO`), cancelación por línea vacía (ENTER) y control de límites de memoria.

- **E/S de Cinta de Papel (Intel HEX):** Lector y emisor en flujo para carga y volcado de código utilizando tramas estándar Intel HEX (`:LLAAAATT[DATOS]CC`) con verificación de *checksum* en tiempo real.

### Arquitectura del Sistema

- **Capa Host AVR:** Ejecución directa sobre ATmega328P mediante `HardwareSerial` configurado a 9600 baudios.

- **Bus de Memoria:** Arreglo estático de 512 bytes (`user_rom_buffer`) que simula dos chips de ROM Intel 4001 ($256 \times 8$ bits cada uno).

- **Ciclo Fetch-Decode-Execute:** Decodificador de instrucciones de 1 byte (8 bits) y 2 bytes (16 bits) con volcado del estado de registros.

### Cadena de Herramientas (Toolchain)

- **`code2ihex.py`:** Ensamblador en Python que traduce código fuente ensamblador del Intel 4004 directamente a formato Intel HEX. Soporta registros ($R_0$ – $R_{15}$), pares ($P_0$ – $P_7$) y eliminación automática de comentarios (`;`, `//`).

### Guía Rápida de Uso

1. **Compilación y Subida:** Compila y graba el firmware en tu ATmega328P / Arduino UNO desde Arduino IDE o CLI.

2. **Conexión por Terminal:** Abre una terminal serie configurada a 9600 baudios:
   
   Bash
   
   ```
   picocom -b 115200 /dev/ttyUSB0
   ```

3. **Ensamblar Código:** Genera el archivo Intel HEX desde código ensamblador usando el script de Python:
   
   Bash
   
   ```
   python asm2ihex.py programa.asm
   ```

4. **Carga y Ejecución:**
   
   - En el menú `INTELLEC-4>`, pulsa **`R`** para activar la lectura de cinta.
   
   - Pega el bloque Intel HEX generado en la terminal.
   
   - Pulsa **`G`** para arrancar la ejecución.

### Requisitos

- **Hardware:** Placa de desarrollo basada en ATmega328P (Arduino UNO, Nano o microcontrolador independiente).

- **Entorno de desarrollo:** Arduino IDE o CLI (estándar C++11 o superior).

- **Herramientas:** Python 3.x para ejecutar `code2ihex.py`.

### Licencia

Este proyecto está licenciado bajo la **GNU General Public License v2.0**. Puedes consultar el archivo `LICENSE` para más detalles.
