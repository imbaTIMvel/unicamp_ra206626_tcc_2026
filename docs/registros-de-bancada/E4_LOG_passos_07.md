# Registro de passos de configuração — Sessão 7

**Data:** quinta-feira, 09/10/2026
**Blocos:** 7 — **concluído** · 8a (banco de filtros) — **concluído**

---

## Resumo

Alvo Raspberry Pi 4 medido: paridade perfeita, latência, dispersão e memória. O experimento 8a fechou a última lacuna metodológica aberta do alvo BitDogLab. Falta apenas o 8b (float32 contra int8).

---

## Passos que contam — alvo Raspberry Pi 4

| # | Ação |
|---|---|
| 13 | `sudo apt install -y linux-cpupower` |
| 14 | `sudo cpupower frequency-set -g performance` |
| 15 | `scp` do `.eim` para a placa |
| 16 | `scp` das quatro amostras `.bin` |
| 17 | `chmod +x modelo.eim` |
| 18 | Criar o script de medição na placa |
| 19 | `sudo apt install -y time` |

**Total do alvo Raspberry Pi 4: 19 passos.** Contra **27** do alvo BitDogLab.

A comparação bruta favorece o Linux, mas precisa de ressalva: dos 19, **quatro existem apenas para contornar dependências não declaradas** (sessão 6), e dois tratam de um ajuste de governor que só é necessário porque o sistema operacional gerencia frequência. Dos 27 da BitDogLab, uma parcela é toolchain instalada do zero, reaproveitável em qualquer projeto futuro no mesmo alvo.

---

## Resultados medidos — Raspberry Pi 4 Model B Rev 1.5, 8 GB

Governor em `performance`, clock confirmado em 1,8 GHz. `throttled=0x0` antes e depois. Temperatura final 45,2 °C.

### Paridade

```
bye     → bye 0.99609  hello 0.00000  noise 0.00000  unknown 0.00000
hello   → bye 0.00000  hello 0.71094  noise 0.08203  unknown 0.20703
noise   → bye 0.00000  hello 0.00000  noise 0.99609  unknown 0.00000
unknown → bye 0.00000  hello 0.02734  noise 0.00391  unknown 0.96875
```

**Idênticos aos da BitDogLab, até a quinta casa decimal.** Projeto confirmado no `runner.init()`: `TCC_hello_bye`, id 1123849, deploy 4 — mesmo impulse dos pacotes v2 e v3.

### Latência e memória

| Métrica | Rodada 1 | Rodada 2 |
|---|---|---|
| Tempo de parede, mediana | 12.925,3 µs | 13.090,9 µs |
| Mínimo / máximo | 12.869,1 / 14.100,9 | 12.977,4 / 13.867,0 |
| DSP (runtime) | 2 ms | 2 ms |
| Classificador (runtime) | < 1 ms | < 1 ms |

Diferença de 1,3% entre as duas rodadas do mesmo script — consistência boa para sistema operacional de uso geral. Memória de pico do processo Python: **62.340 kB (~60,9 MB)**. CPU em 115%, ou seja, mais de um núcleo em uso.

---

## Comparação entre alvos

| | BitDogLab (M0+ 133 MHz) | RPi 4 (A72 1,8 GHz) |
|---|---|---|
| DSP | 920 ms | 2 ms |
| Classificador | 11 ms | < 1 ms |
| Tempo de parede | 931 ms | 12,9 ms |
| Dispersão | 116 µs (0,012%) | ~0,7% estabilizada; 9,2% na 1ª |
| Memória de execução | 23,1 kB | ~60,9 MB |
| Artefato implantável | 107 kB (firmware) | 13,4 MB (`.eim`) |
| Paridade | 4/4 | 4/4, idêntica à 5ª casa |
| Passos de configuração | 27 | 19 |

Vantagem por ciclo do A72 no DSP, descontada a razão de clock de 13,5×: **cerca de 34×**. Compatível com FPU em hardware mais extensões vetoriais.

---

## Três achados

**1. A quantização compra reprodutibilidade entre arquiteturas.** Eu havia previsto divergência a partir da terceira ou quarta casa decimal, pelo MFCC em ponto flutuante. **A previsão falhou**, e a razão é instrutiva: as saídas são int8 desquantizadas, com passo de 1/256. Confira — 0,99609 = 255/256, 0,71094 = 182/256, 0,08203 = 21/256, 0,20703 = 53/256. O passo é grosseiro o bastante para absorver qualquer diferença numérica do pré-processamento. A quantização, normalmente apresentada como concessão de acurácia, entrega também coincidência exata de resultado entre placas distintas — propriedade de valor didático direto.

**2. Rápido e imprevisível, contra lento e determinístico.** O Linux é 460× mais rápido no DSP e aproximadamente 60× mais disperso. Mesmo modelo, mesma entrada, mesmos quatro arquivos. É a confirmação empírica do argumento da seção 4.3, e não vinha de datasheet.

**3. Parte da vantagem não é do núcleo, é da quantidade de núcleos.** 115% de CPU: 0,84 s de usuário contra 0,79 s de relógio. Ao migrar de domínio, ganha-se FPU, extensões vetoriais **e** paralelismo com escalonador — vale distinguir as três coisas ao discutir o ganho.

---

## Experimento 8a — banco de filtros quantizado

Recompilado com `EIDSP_QUANTIZE_FILTERBANK=1`, gravado e medido. **Três medidas independentes saíram idênticas** à configuração anterior:

| | `=0` | `=1` |
|---|---|---|
| Latência, mediana | 934,98 ms | 934,98 ms |
| Heap de pico | 18.956 B | 18.956 B |
| Tamanho do binário | 246.328 B | 246.328 B |

**Conclusão: a macro é inerte neste impulse.** O caminho de MFCC que o modelo utiliza não passa pelo código que ela controla. Leitura do fonte confirma que, de todo modo, ela só trocaria o armazenamento da tabela de coeficientes entre `float` e `uint8` — não alteraria a aritmética da FFT, do logaritmo e da DCT, que é onde estão os 920 ms.

**Isso fecha a lacuna que eu havia aberto na sessão 5.** Os 920 ms do MFCC são atribuíveis à arquitetura do núcleo, não à configuração de compilação sugerida. A afirmação central do eixo 4.4 fica mais firme, e agora com controle experimental.

Repositório revertido para `=0` e recompilado; flags conferidas.

### Correção à tabela da sessão 5

Os dois valores de tamanho de binário referem-se a builds diferentes e ambos são válidos — faltava o rótulo:

| Build | `text` | `bss` |
|---|---|---|
| Sem instrumentação de memória | 235.644 B | 4.128 B |
| Com instrumentação (`mallinfo`) | 246.328 B | 4.132 B |

A instrumentação custou 10,7 kB de flash e 0,45% de latência. Reporte a latência da rodada limpa (930,77 ms) e a memória da rodada instrumentada, declarando que são execuções distintas.

---

## Becos e atritos

| Tentativa | O que houve |
|---|---|
| `sudo apt install cpufrequtils` | Pacote extinto no Debian 13; a documentação do Raspberry Pi ainda o recomenda. Substituto: `linux-cpupower`. |
| `scp /mnt/c/...` rodado dentro da Raspberry Pi | O caminho é do WSL. `scp` tem que sair da máquina onde o arquivo está. |
| `/usr/bin/time -v` sem o pacote `time` | O `time` do bash é builtin e não tem `-v`. |

O primeiro item forma padrão com o `libstdc++-arm-none-eabi-newlib` do primeiro dia: **dois casos de documentação oficial apontando pacote que não existe mais na distribuição corrente**. É subtipo próprio dentro do critério 3.3.3 e vale ser nomeado como tal no Capítulo 5.

---

## Incorporado ao TCC

Quatro parágrafos novos e uma correção, com alterações rastreadas:

- **4.4.1** — ressalva do banco de filtros **substituída** pelo resultado do 8a
- **4.4.1** — contraste medido entre alvos: 34× por ciclo, memória, artefato, multinúcleo
- **4.4.1** — dispersão: rápido e imprevisível contra lento e determinístico
- **4.4.1** — paridade exata e o benefício de reprodutibilidade da quantização
- **4.4.2** — os dois tipos de atrito de integração e por que o segundo é pior para capacitação

---

## O que falta no E4

**8b — float32 contra int8 na BitDogLab.** Exportar o modelo não quantizado, recompilar, medir. Meia hora com folga. Risco real: a arena salta de 6,1 kB para perto de 25 kB e pode não caber, com travamento em vez de erro claro.

**9 — microfone ao vivo.** Demonstração, não medição. Os 93% de ocupação da janela limitam o que pode mostrar.

Pendente sem bancada: preços em BRL, confirmações de datasheet, Figuras 2 e 6.

---

> Rascunho de bancada. Medições próprias, execução única por configuração, sem tratamento estatístico.
