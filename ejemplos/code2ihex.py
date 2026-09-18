#!/usr/bin/bash

import sys

def parse_val(v: str) -> int:
    v = v.strip()
    return int(v, 16) if v.lower().startswith("0x") else int(v)

def parse_reg(r: str) -> int:
    r = r.strip().upper()
    return int(r[1:]) if r.startswith("R") else int(r)

def parse_rp(rp: str) -> int:
    rp = rp.strip().upper()
    pairs = {
        "R0R1":0, "R2R3":1, "R4R5":2, "R6R7":3, 
        "R8R9":4, "R10R11":5, "R12R13":6, "R14R15":7,
        "P0":0, "P1":1, "P2":2, "P3":3, "P4":4, "P5":5, "P6":6, "P7":7
    }
    return pairs[rp] if rp in pairs else int(rp)

def assemble_line(line: str) -> list[int]:
    line = line.split(";")[0].split("//")[0].strip() # Limpia comentarios
    if not line: return []
    
    parts = line.replace(",", " ").split()
    mnem, args = parts[0].upper(), parts[1:]

    if mnem == "NOP": return [0x00]
    elif mnem == "FIM": return [0x20 | (parse_rp(args[0]) << 1), parse_val(args[1]) & 0xFF]
    elif mnem == "SRC": return [0x21 | (parse_rp(args[0]) << 1)]
    elif mnem == "FIN": return [0x30 | (parse_rp(args[0]) << 1)]
    elif mnem == "JIN": return [0x31 | (parse_rp(args[0]) << 1)]
    elif mnem == "JUN": 
        a = parse_val(args[0])
        return [0x40 | ((a >> 8) & 0x0F), a & 0xFF]
    elif mnem == "JMS": 
        a = parse_val(args[0])
        return [0x50 | ((a >> 8) & 0x0F), a & 0xFF]
    elif mnem == "INC": return [0x60 | parse_reg(args[0])]
    elif mnem == "ISZ": return [0x70 | parse_reg(args[0]), parse_val(args[1]) & 0xFF]
    elif mnem == "ADD": return [0x80 | parse_reg(args[0])]
    elif mnem == "SUB": return [0x90 | parse_reg(args[0])]
    elif mnem == "LD":  return [0xA0 | parse_reg(args[0])]
    elif mnem == "XCH": return [0xB0 | parse_reg(args[0])]
    elif mnem == "BBL": return [0xC0 | (parse_val(args[0]) & 0x0F)]
    elif mnem == "LDM": return [0xD0 | (parse_val(args[0]) & 0x0F)]
    elif mnem == "CLC": return [0xF1]
    elif mnem == "STC": return [0xFA]
    elif mnem == "RAL": return [0xF5]
    elif mnem == "RAR": return [0xF6]
    elif mnem == "CMC": return [0xF3]
    elif mnem == "TCC": return [0xF7]
    elif mnem == "DAC": return [0xF8]
    elif mnem == "DAA": return [0xFB]
    elif mnem == "DCL": return [0xFD]
    else: raise ValueError(f"Instrucción no soportada: {mnem}")

def to_intel_hex(bytes_list: list[int]) -> str:
    out, chunk_size = [], 16
    for i in range(0, len(bytes_list), chunk_size):
        chunk = bytes_list[i:i + chunk_size]
        hdr = [len(chunk), (i >> 8) & 0xFF, i & 0xFF, 0x00]
        chk = (-sum(hdr + chunk)) & 0xFF
        out.append(f":{len(chunk):02X}{i:04X}00" + "".join(f"{b:02X}" for b in chunk) + f"{chk:02X}")
    out.append(":00000001FF")
    return "\n".join(out)

# ============================================================================
# CÓDIGO DE PRUEBA EN ENSAMBLADOR (Escribe tu programa aquí)
# ============================================================================
codigo_ensamblador = """
    LDM 0x09      ; Carga 9 en el acumulador
    CLC           ; Limpia el acarreo
    RAL           ; Rotación a la izquierda
    XCH R0        ; Guarda el resultado en R0
    NOP
"""

if __name__ == "__main__":
    # Si le pasas un archivo .asm como argumento lo lee, si no usa el string de ejemplo
    if len(sys.argv) > 1:
        with open(sys.argv[1], "r") as f:
            lines = f.readlines()
    else:
        lines = codigo_ensamblador.strip().split("\n")

    opcodes = []
    for line in lines:
        opcodes.extend(assemble_line(line))

    print(to_intel_hex(opcodes))
