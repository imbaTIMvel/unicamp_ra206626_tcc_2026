import wave, struct, os

N_ALVO = 16000                      # EI_CLASSIFIER_RAW_SAMPLE_COUNT
CLASSES = ['bye', 'hello', 'noise', 'unknown']   # ordem do modelo

def ler(caminho):
    with wave.open(caminho, 'rb') as w:
        assert w.getframerate() == 16000, f'{caminho}: taxa {w.getframerate()}'
        assert w.getnchannels() == 1,     f'{caminho}: {w.getnchannels()} canais'
        assert w.getsampwidth() == 2,     f'{caminho}: {w.getsampwidth()*8} bits'
        n = w.getnframes()
        d = list(struct.unpack(f'<{n}h', w.readframes(n)))
    if len(d) > N_ALVO:
        corte = len(d) - N_ALVO
        d = d[:N_ALVO]
        print(f'  {caminho}: truncado em {corte} amostras')
    elif len(d) < N_ALVO:
        faltam = N_ALVO - len(d)
        d += [0] * faltam
        print(f'  {caminho}: completado com {faltam} amostras de silencio')
    return d

dados = {}
for c in CLASSES:
    dados[c] = ler(f'amostras/sample_{c}.wav')
    with open(f'amostras/sample_{c}.bin', 'wb') as f:      # para o Raspberry Pi
        f.write(struct.pack(f'<{N_ALVO}h', *dados[c]))

with open('audio_fixo.h', 'w') as f:                        # para a BitDogLab
    f.write('// gerado por wav_para_c.py - nao editar a mao\n')
    f.write(f'#define AUDIO_N {N_ALVO}\n')
    f.write(f'#define AUDIO_QTD {len(CLASSES)}\n\n')
    for c in CLASSES:
        f.write(f'static const int16_t audio_{c}[AUDIO_N] = {{\n')
        v = dados[c]
        for i in range(0, N_ALVO, 12):
            f.write('  ' + ', '.join(str(x) for x in v[i:i+12]) + ',\n')
        f.write('};\n\n')
    f.write('typedef struct { const char *nome; const int16_t *dados; } amostra_t;\n')
    f.write('static const amostra_t amostras[AUDIO_QTD] = {\n')
    for c in CLASSES:
        f.write(f'  {{ "{c}", audio_{c} }},\n')
    f.write('};\n')

print('ok:', os.path.getsize('audio_fixo.h') // 1024, 'kB de header')