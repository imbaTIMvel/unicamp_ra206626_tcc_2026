# Registro de passos de configuração — Sessão 4

**Data:** segunda-feira, 05/10/2026 · encerramento às 17h08
**Blocos:** 5 — **concluído** · 6 — **preparado, não compilado**

---

## Resumo

As quatro amostras de áudio foram convertidas em buffers fixos, e o terreno do Bloco 6 está montado: pacote MCU descompactado, dependências do Pico SDK no lugar, estrutura real do SDK inspecionada. A compilação fica para amanhã, com três problemas já identificados e resolvidos no papel antes de aparecerem como erro.

---

## Bloco 5 — concluído

### Verificação das amostras

As quatro saíram do Edge Impulse já no formato exato que o impulse espera:

| Arquivo | Taxa | Canais | Profundidade | Amostras |
|---|---|---|---|---|
| `sample_bye.wav` | 16000 Hz | 1 | 16 bits | 16000 |
| `sample_hello.wav` | 16000 Hz | 1 | 16 bits | 16000 |
| `sample_noise.wav` | 16000 Hz | 1 | 16 bits | 16000 |
| `sample_unknown.wav` | 16000 Hz | 1 | 16 bits | 16000 |

**Nenhuma amostra precisou de truncamento ou de preenchimento com silêncio.** Registre isso de forma explícita na dissertação: a entrada dos três alvos é o áudio original, sem nenhuma intervenção. É uma afirmação forte de reprodutibilidade, e só se pode fazê-la porque a verificação foi feita antes da conversão.

### Artefatos gerados

```
~/tcc/e4/modelo/
├── wav_para_c.py
├── audio_fixo.h              → BitDogLab (4 arrays + tabela de amostras)
└── amostras/
    ├── sample_{bye,hello,noise,unknown}.wav
    └── sample_{bye,hello,noise,unknown}.bin   → Raspberry Pi 4
```

### Contagem

Pela convenção fixada na sessão 1, item 4, **o Bloco 5 não entra na contagem de passos de nenhum alvo**. Produzir a entrada comum é custo do problema, idêntico nas três plataformas, e contabilizá-lo inflaria igualmente os três números sem informar nada.

---

## Passos que contam — Bloco 6, preparação

Alvo: **BitDogLab (Pico W)**. Continuação da numeração do alvo.

| # | Ação |
|---|---|
| 17 | Instalar o `unzip` no WSL |
| 18 | Descompactar `tcc_hello_bye-cpp-mcu-v2-impulse-#1.zip` em `~/tcc/e4/picow/e4_kws` |
| 19 | Copiar `pico_sdk_import.cmake` para o projeto |
| 20 | Copiar `audio_fixo.h` para o projeto |

**Subtotal da sessão: 4 passos.**

Não contam, mas ficam registrados: a inspeção dos globs, a listagem de `porting/` e a cópia de resguardo do `CMakeLists.txt` original. São instrumentação e leitura, não configuração.

> Nota menor sobre o `unzip`: o guia já o lista na linha do `apt` do Bloco 1, mas ele não entrou na instalação da sessão 1 — provavelmente ficou de fora quando a linha foi reescrita sem o pacote extinto. Quem repetir o procedimento pelo guia atual não terá esse passo.

---

## Achados do dia

Três, todos de material para o Capítulo 5 e todos descobertos antes de causarem erro.

**1. Nenhuma das 26 camadas de porting do SDK atende o RP2040.** A listagem de `edge-impulse-sdk/porting/` traz 26 diretórios — Zephyr, Arduino, Espressif, Mbed, Raspberry, STM32, e assim por diante. Nenhum para Pico ou RP2040. Pior do que ausência simples: incluir `porting/posix` nos globs compila dois arquivos que, por causa da guarda `#if EI_PORTING_POSIX == 1`, viram zero código — o glob "acha" os arquivos e o linker falha depois. A camada `clib` quase serve, mas seu `ei_read_timer_us()` retorna 0 e não é símbolo fraco, o que zeraria exatamente as duas métricas de tempo do experimento. Solução adotada: camada de porting própria (`ei_porting_pico.cpp`, seção 6.1 do guia).

**2. O `CMakeLists.txt` do deployment "C++ library" é específico para Zephyr.** Exige um alvo chamado `app` criado previamente e adiciona `edge-impulse-sdk/cmake/zephyr` como subdiretório. O deployment é anunciado como genérico e a documentação afirma que roda "em qualquer dispositivo", mas o arquivo de build que acompanha está amarrado a um RTOS. Original preservado como `CMakeLists.ei-original.txt`, para citação.

**3. São 661 arquivos de CMSIS excluídos do build.** O `EIDSP_USE_CMSIS_DSP=0` não é ajuste de conveniência: é a consequência direta de o Cortex-M0+ não ter as extensões DSP que a biblioteca assume. **A afirmação do eixo 4.4 sobre ausência de unidade de ponto flutuante e de aceleração DSP tem, aqui, uma expressão numérica: 661 arquivos de código otimizado que a plataforma não pode usar.** Vale a frase na dissertação.

### Lição de método, que também é achado

O teste de globs por contagem de arquivos deu quatro acertos e **dois falsos positivos**: o padrão `**` do `find` atravessa barras e o do CMake não, e `porting/posix` contou dois arquivos que compilam para nada. Contar arquivos verifica que o caminho existe, não que o conteúdo serve. A verificação boa é a contagem final de fontes no `cmake` — 162 esperados — somada ao próprio build.

---

## Estado do diretório ao encerrar

```
~/tcc/e4/picow/e4_kws/
├── CMakeLists.txt              ← do EI, para Zephyr; será substituído
├── CMakeLists.ei-original.txt  ← cópia de resguardo
├── README.txt
├── audio_fixo.h                ✔
├── pico_sdk_import.cmake       ✔
├── edge-impulse-sdk/
├── model-parameters/
└── tflite-model/
```

Falta criar: `ei_porting_pico.cpp`, `main.cpp` e o `CMakeLists.txt` novo. Os três estão prontos no guia, seções 6.1 e 6.2.

---

## Amanhã — ordem exata

1. Criar `ei_porting_pico.cpp` (guia, 6.1)
2. Criar `main.cpp` (guia, Bloco 6)
3. Substituir o `CMakeLists.txt` (guia, 6.2)
4. `mkdir build && cd build && cmake .. -DPICO_BOARD=pico_w` — **conferir que a mensagem diz 162 fontes**
5. `make -j$(nproc)`
6. Gravar: BOOTSEL + RESET, `cp e4_kws.uf2 /mnt/d/`
7. PuTTY na COM, com log em arquivo ativo
8. Ler a passada de paridade: as quatro amostras devem acertar a classe
9. Ler as dez execuções de latência sobre `sample_hello`
10. Comparar com a previsão registrada: 157 ms estimados para Cortex-M4F 80 MHz, com 154 ms de MFCC

Verificação de trinta segundos antes de começar, que vale a pena:

```bash
ls -l ~/tcc/e4/modelo/amostras/*.bin    # quatro arquivos, 32000 bytes cada
```

Se algum divergir, o `wav_para_c.py` não escreveu o que se espera, e é melhor descobrir agora do que depois de uma hora de build.

---

## Pendências administrativas

- [ ] `git add -A && git commit` desta sessão
- [ ] `history > ~/tcc/e4/logs/historico_sessao4.txt`
- [ ] Transportar para a planilha: nenhuma métrica nova hoje; só a contagem de passos

---

> Rascunho de bancada. A contagem de passos é reconstituída do log do operador e deve ser conferida contra o histórico do shell.
