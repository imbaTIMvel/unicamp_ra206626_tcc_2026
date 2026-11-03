#!/usr/bin/env python3
"""E4 - paridade e latencia de inferencia sobre entrada fixa.
Alvo: Raspberry Pi 4, runtime Edge Impulse para Linux (.eim AARCH64).
As quatro amostras .bin sao as mesmas usadas nos demais alvos.
"""
import time, numpy as np
from edge_impulse_linux.runner import ImpulseRunner

CLASSES = ['bye', 'hello', 'noise', 'unknown']
BASE = '/home/imbatimvel'

runner = ImpulseRunner(f'{BASE}/modelo.eim')
info = runner.init()
p = info['project']
print('projeto:', p['name'], '| id:', p['id'], '| deploy:', p.get('deploy_version'))

def carregar(c):
    return np.fromfile(f'{BASE}/sample_{c}.bin', dtype=np.int16).astype(float).tolist()

print('=== PARIDADE ===')
for c in CLASSES:
    r = runner.classify(carregar(c))
    cls = r['result']['classification']
    print(c, ','.join(f'{k}={v:.5f}' for k, v in cls.items()),
          f"dsp_ms={r['timing']['dsp']}", f"classif_ms={r['timing']['classification']}")

print('=== LATENCIA: 11 execucoes, descartando a primeira ===')
f = carregar('hello')
ts = []
for i in range(11):
    t0 = time.perf_counter_ns()
    r = runner.classify(f)
    dt = (time.perf_counter_ns() - t0) / 1000
    if i == 0:
        print(f'descartada: {dt:.1f} us'); continue
    ts.append(dt)
    print(f'{i},parede_us={dt:.1f},dsp_ms={r["timing"]["dsp"]},'
          f'classif_ms={r["timing"]["classification"]}')

print(f'parede: mediana={np.median(ts):.1f} us  min={min(ts):.1f}  max={max(ts):.1f}')
runner.stop()
