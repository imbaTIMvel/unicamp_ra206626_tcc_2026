#!/usr/bin/env python3
"""E1 - teste de vida do conversor externo AD5592R (SPI).
Alvo: Raspberry Pi 4. Divisor LDR + 10 kOhm no canal IO0.
Cubra e descubra o LDR durante a execucao.
"""
import spidev, time

spi = spidev.SpiDev(); spi.open(0, 0)
spi.mode = 1
spi.max_speed_hz = 1_000_000

def w(word):
    spi.xfer2([word >> 8, word & 0xFF])

def ler():
    r = spi.xfer2([0x00, 0x00])
    v = (r[0] << 8) | r[1]
    return v & 0x0FFF, (v >> 12) & 0x7

# 0x7DAC reset | 0x5A00 referencia interna | 0x1820 faixa 0..2xVref
# 0x2001 IO0 como ADC | 0x1201 sequencia com repeticao
for p in (0x7DAC, 0x5A00, 0x1820, 0x2001, 0x1201):
    w(p); time.sleep(0.001)

ler(); ler()          # descarta a latencia de conversao do componente

for i in range(40):
    valor, canal = ler()
    print(f"{i} canal={canal} valor={valor}")
    time.sleep(0.25)
