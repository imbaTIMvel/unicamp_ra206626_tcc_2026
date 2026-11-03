#!/usr/bin/env python3
"""E1 - latencia de leitura analogica via AD5592R (SPI a 10 MHz).
Alvo: Raspberry Pi 4. Realiza apenas a leitura, sem acionamento de atuador.
"""
import spidev, time, statistics as st

spi = spidev.SpiDev(); spi.open(0, 0)
spi.mode = 1
spi.max_speed_hz = 10_000_000

def w(word):
    spi.xfer2([word >> 8, word & 0xFF])

def ler():
    r = spi.xfer2([0x00, 0x00])
    return ((r[0] << 8) | r[1]) & 0x0FFF

for p in (0x7DAC, 0x5A00, 0x1820, 0x2001, 0x1201):
    w(p); time.sleep(0.001)
ler(); ler()

print("=== FUNCIONAL: 10 leituras, mexa no LDR ===")
for i in range(10):
    print(i, ler()); time.sleep(0.2)

print("=== LATENCIA: 10 blocos de 1000 iteracoes ===")
med = []
for b in range(10):
    t0 = time.perf_counter_ns()
    for _ in range(1000):
        ler()
    t1 = time.perf_counter_ns()
    ns = (t1 - t0) / 1000
    med.append(ns)
    print(f"{b},por_iter_ns={ns:.0f}")

print(f"mediana={st.median(med):.0f} ns  min={min(med):.0f}  max={max(med):.0f}")
print("=== FIM ===")
