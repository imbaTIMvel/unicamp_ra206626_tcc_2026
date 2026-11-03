#!/usr/bin/env python3
"""E2 - teste de vida do acelerometro por I2C. Alvo: Raspberry Pi 4.
Endereco 0x68. Incline o modulo durante a execucao.
"""
from smbus2 import SMBus
import time

ADDR = 0x68
bus = SMBus(1)

print("WHO_AM_I:", hex(bus.read_byte_data(ADDR, 0x75)))
bus.write_byte_data(ADDR, 0x6B, 0x00)      # acorda o chip
time.sleep(0.1)

def eixo(reg):
    hi = bus.read_byte_data(ADDR, reg)
    lo = bus.read_byte_data(ADDR, reg + 1)
    v = (hi << 8) | lo
    return v - 65536 if v > 32767 else v

for i in range(20):
    ax, ay, az = eixo(0x3B), eixo(0x3D), eixo(0x3F)
    print(f"{i} ax={ax:6d} ay={ay:6d} az={az:6d}")
    time.sleep(0.25)
