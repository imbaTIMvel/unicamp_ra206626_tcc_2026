# Registro de passos de configuração — Sessão 5

**Data:** terça-feira, 06/10/2026
**Bloco:** 6 — **concluído**. Primeiras medições em hardware real.

---

## Passos que contam — Bloco 6, alvo BitDogLab

| # | Ação |
|---|---|
| 21 | Criar `ei_porting_pico.cpp` |
| 22 | Criar `main.cpp` |
| 23 | Substituir o `CMakeLists.txt` |
| 24 | `cmake .. -DPICO_BOARD=pico_w` — **162 fontes confirmados** |
| 25 | `make -j$(nproc)` |
| 26 | Gravar o firmware (BOOTSEL + RESET, cópia do `.uf2`) |
| 27 | Abrir a sessão serial com log em arquivo |

**Total do alvo BitDogLab até aqui: 27 passos.**

Não contam: a conferência dos `.bin`, o `arm-none-eabi-size` e as duas rodadas de instrumentação de memória — são medição, não configuração.

---

## Resultados medidos — alvo BitDogLab (Pico W em BitDogLab v6.3)

| Métrica | Valor | Como foi obtido |
|---|---|---|
| Tamanho do modelo (`.tflite` int8) | 6.504 B (~6,4 kB) | arquivo do deployment |
| Tensor arena | 6.265 B (~6,1 kB) | `model_metadata.h` |
| Heap em uso, pico | 18.956 B | `mallinfo().uordblks` amostrado a cada alocação |
| RAM estática (`bss`) | 4.128 B | `arm-none-eabi-size` |
| **Memória de execução total** | **~23,1 kB** | heap de pico + `bss` |
| Flash, firmware completo | 235.644 B | `arm-none-eabi-size`, campo `text` |
| Flash, descontadas as amostras | ~107,6 kB | 235.644 − 128.000 B de áudio embutido |
| Latência total, mediana | **930,77 ms** | 10 execuções, rodada sem instrumentação |
| Latência, mínimo / máximo | 930,74 / 930,86 ms | idem |
| Dispersão | 116 µs — **0,012%** | idem |
| MFCC | 920 ms (98,8%) | `r.timing.dsp` |
| Classificador | 11 ms (1,2%) | `r.timing.classification` |
| Paridade | **4 de 4 classes corretas** | uma execução por classe |

Vetores de probabilidade da passada de paridade, a guardar para comparação com a Raspberry Pi:

```
bye     → bye 0.99609  hello 0.00000  noise 0.00000  unknown 0.00000
hello   → bye 0.00000  hello 0.71094  noise 0.08203  unknown 0.20703
noise   → bye 0.00000  hello 0.00000  noise 0.99609  unknown 0.00000
unknown → bye 0.00000  hello 0.02734  noise 0.00391  unknown 0.96875
```

A amostra `hello` é a de menor margem (0,711 contra 0,207). É nela que eventuais diferenças numéricas entre arquiteturas vão aparecer primeiro — use-a como sentinela no Bloco 7.

---

## Estimado contra medido

| | Estimativa (M4F 80 MHz) | Medido (M0+ 133 MHz) | Razão |
|---|---|---|---|
| Total | 157 ms | 931 ms | 5,9× |
| MFCC | 154 ms | 920 ms | 6,0× |
| Classificador | 3 ms | 11 ms | 3,7× |
| RAM de pico | 15,4 kB | 19,0 kB (heap) | 1,2× |
| Flash | 30,8 kB | ~107,6 kB | não comparável |

**Descontando a vantagem de clock** (133 contra 80 MHz, fator 1,66), a penalidade por ciclo fica em cerca de **10× no MFCC** e **6× no classificador**. Essa diferença entre os dois fatores é o resultado mais importante da sessão: o pré-processamento em ponto flutuante paga o preço integral da ausência de FPU; a rede quantizada em int8 degrada bem menos.

**Sobre a flash:** os 30,8 kB estimados cobrem apenas o impulse. Os ~107,6 kB incluem o Pico SDK, a pilha USB CDC e o `printf` com suporte a ponto flutuante. Não são a mesma grandeza — reporte os dois com a ressalva explícita do que cada um abrange, nunca a razão entre eles.

---

## Achados

**1. O gargalo é o DSP, não a rede.** 920 dos 931 ms. A quantização integral é necessária mas não suficiente: ela resolve 1,2% do tempo. Para o Capítulo 4 isso desloca o foco — o esforço de otimização, nesse patamar, rende mais em processamento de sinais do que em arquitetura de rede.

**2. Detecção contínua é inviável neste alvo.** Janela de 1 s, inferência de 0,93 s: 93% de ocupação. Com o stride de 500 ms do impulse, a operação contínua exigiria 1,86 s de processamento por segundo de áudio. O Bloco 9 terá de operar em janelas esparsas. Ajuste a expectativa antes de chegar lá.

**3. Determinismo quase perfeito.** 116 µs de dispersão em dez execuções, e consumo de memória idêntico byte a byte entre execuções e entre classes. O caminho de alocação não depende do conteúdo da entrada. É o contraponto direto ao que a Raspberry Pi deve mostrar.

**4. A estimativa do Studio não transfere entre arquiteturas.** Erro de seis vezes. Isso não a torna inútil — é recurso real da ferramenta, pertinente ao critério 3.3.6 —, mas delimita para que serve: comparação entre modelos no mesmo alvo, não previsão de desempenho em alvo diferente.

---

## Becos sem saída — material do critério 3.3.3

| Tentativa | O que houve |
|---|---|
| `signal_t` com `.total_length` antes de `.get_data` | Em C++ os inicializadores designados seguem a ordem de declaração da struct, e em `ei_signal_t` o `get_data` vem primeiro. Erro de compilação. |
| `sleep_ms(3000)` para abrir o terminal | Corrida contra o relógio num programa que roda uma vez só. Substituído por `while (!stdio_usb_connected())`. |
| Contador manual de alocações | Resultou em 315.553 B — acima dos 264 kB de SRAM do chip. O MFCC aloca e libera por quadro, e o contador somou 50 vezes a mesma memória reaproveitada. |
| `mallinfo().uordblks` | Funcionou: 18.956 B. |

O terceiro item merece registro na dissertação. **Nada no SDK orienta como medir memória de execução**, e o caminho intuitivo produz um número fisicamente impossível que passaria despercebido se o chip tivesse mais RAM. É exatamente o tipo de lacuna de documentação que o critério 3.3.3 existe para capturar.

---

## Limitações a declarar

- **Latência e memória vieram de rodadas distintas.** A instrumentação custou cerca de 0,45% de latência (930,77 contra 934,98 ms). Reporte os 930,77 ms e declare que a memória foi medida em execução separada.
- **`mallinfo` mede o heap em uso, não toda a memória de execução.** Buffers em pilha não entram. O valor somado aos 4.128 B de `bss` é a melhor aproximação disponível, não o pico absoluto.
- **O banco de filtros foi compilado em ponto flutuante** (`EIDSP_QUANTIZE_FILTERBANK=0`), configuração adotada por compatibilidade com o Cortex-M0+. Parte dos 920 ms pode ser atribuível a essa escolha, e não apenas à arquitetura. **Recompilar com o banco quantizado é teste de dez minutos e fecharia essa lacuna** — recomendo fazer antes do Bloco 8.

---

## Incorporado ao TCC

Três parágrafos novos no `.docx`, com alterações rastreadas:

- **4.4.1** — dimensionamento da penalidade arquitetural (6× bruto, ~10× contra ~6× por ciclo), com a ressalva do banco de filtros.
- **4.4.1** — ocupação de 93% da janela, inviabilidade da detecção contínua, e os números de determinismo.
- **4.4.2** — qualificação da síntese: a quantização é necessária mas não suficiente; o gargalo está no pré-processamento.

A tabela completa de resultados pertence ao Capítulo 5, ainda não redigido.

---

## Próxima sessão — Bloco 7

1. Verificar `uname -m` (precisa ser `aarch64`) e `vcgencmd get_throttled`
2. `scp` do `.eim` e dos quatro `.bin`
3. Rodar o script de paridade e latência
4. `vcgencmd get_throttled` **de novo**, depois das dez execuções
5. Comparar os quatro vetores de probabilidade com os da tabela acima
6. Reportar mediana **e pior caso** — a dispersão é o contraponto aos 116 µs

Opcional, dez minutos, antes do Bloco 8: recompilar o firmware com `EIDSP_QUANTIZE_FILTERBANK=1` e medir de novo.

---

> Rascunho de bancada. Os números são medições próprias, de execução única por configuração, sem tratamento estatístico — coerente com o princípio do Capítulo 4, e a ser declarado como tal.
