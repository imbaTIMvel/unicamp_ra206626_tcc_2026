#!/usr/bin/env python3
"""E2 - latencia de uma transacao I2C de seis bytes. Alvo: Raspberry Pi 4.
A frequencia do barramento vem do device tree (config.txt), nao do script.
"""
from smbus2 import SMBus
import time, statistics as st

ADDR = 0x68
bus = SMBus(1)
print("WHO_AM_I:", hex(bus.read_byte_data(ADDR, 0x75)))
bus.write_byte_data(ADDR, 0x6B, 0x00)
time.sleep(0.1)

def ler():
    d = bus.read_i2c_block_data(ADDR, 0x3B, 6)
    return [((d[i] << 8 | d[i+1]) - 65536 if (d[i] << 8 | d[i+1]) > 32767
             else (d[i] << 8 | d[i+1])) for i in (0, 2, 4)]

print("=== FUNCIONAL ===")
for i in range(5):
    print(i, ler()); time.sleep(0.2)

print("=== LATENCIA: 10 blocos de 1000 iteracoes ===")
med = []
for b in range(10):
    t0 = time.perf_counter_ns()
    for _ in range(1000):
        ler()
    ns = (time.perf_counter_ns() - t0) / 1000
    med.append(ns)
    print(f"{b},por_iter_ns={ns:.0f}")

print(f"mediana={st.median(med):.0f} ns  min={min(med):.0f}  max={max(med):.0f}")
print("=== FIM ===")
