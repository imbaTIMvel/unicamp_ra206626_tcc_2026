#!/usr/bin/env python3
"""E3 - desvio de tarefa periodica de 10 ms, em repouso e sob contencao.
Alvo: Raspberry Pi 4, LED no GPIO18.
A carga e disparada pelo proprio script, eliminando erro de sincronia entre
terminais; sao 16 processos em 4 nucleos, para garantir disputa efetiva.
"""
import time, statistics as st, subprocess
from gpiozero import LED

led = LED(18)
PERIODO_NS = 10_000_000      # 10 ms
N = 500

def medir(rotulo):
    desvios = []
    prox = time.perf_counter_ns() + PERIODO_NS
    for _ in range(N):
        while time.perf_counter_ns() < prox:
            pass
        real = time.perf_counter_ns()
        led.toggle()
        desvios.append(abs(real - prox) / 1000)
        prox += PERIODO_NS
    s = sorted(desvios)
    print(f"{rotulo}: mediana={st.median(s):.1f} us  "
          f"p95={s[int(N*0.95)]:.1f} us  max={max(s):.1f} us")

print("=== SEM CARGA ===")
medir("ocioso")

print("iniciando carga...")
carga = subprocess.Popen(["stress-ng", "--cpu", "16", "--timeout", "30s"],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(3)
print("=== SOB CARGA ===")
medir("carregado")
carga.wait()
led.off()
