# Registro de passos de configuração — Sessão 3

**Data:** _______ · **Bloco:** 4 (deployment) — **concluído**
**Resultado:** três artefatos de implantação exportados, todos do mesmo impulse

---

## Passos que contam — Bloco 4, deployment

| # | Ação |
|---|---|
| 12 | *Deployment* → alvo **C++ library** (MCU) |
| 13 | Selecionar **EON Compiler** como motor de inferência |
| 14 | Selecionar otimização **quantizada int8** |
| 15 | Rodar o estimador de desempenho do Studio |
| 16 | *Build* e download do pacote MCU |
| 17 | Repetir o deployment para **C++ library (Linux)** |
| 18 | Repetir o deployment para **Linux (AARCH64)** → `.eim` |

**Bloco 4 fechado: 18 passos** (11 da sessão 2 + 7 desta).

Esse é o número que vai ser comparado, mais adiante, com a contagem do fluxo do Qualcomm AI Hub para o domínio MPU da UNO Q.

---

## Artefatos e destino de cada um

| Arquivo | Versão | Destino |
|---|---|---|
| `tcc_hello_bye-cpp-mcu-v2-impulse.zip` | v2 | **Bloco 6** — BitDogLab. EON compilado, int8 |
| `tcc_hello_bye-linux-aarch64-v4-impulse.eim` | v4 | **Bloco 7** — Raspberry Pi 4. ELF aarch64, 13,4 MB, TFLite completo |
| `tcc_hello_bye-cpp-linux-v3-impulse.zip` | v3 | Reserva. Exige build na própria Pi com `EI_CLASSIFIER_USE_FULL_TFLITE=1` |

**Conferido:** os pacotes v2 e v3 carregam o mesmo projeto (1123849), o mesmo bloco de aprendizado (`1123849_7`) e o mesmo bloco de DSP, com parâmetros idênticos. A diferença de versão é só o contador de deploy. Paridade preservada.

**A conferir no Bloco 7:** o `.eim` é v4 e não foi comparado byte a byte. O `runner.init()` imprime nome e versão do projeto — confirme ali que bate com `TCC_hello_bye` / 1123849 antes de medir.

---

## ⚠️ Os números do Studio são estimativas, não medições

Este é o ponto mais importante da sessão, e ele muda como os valores entram na dissertação.

Você anotou 157 ms de latência, 15,4 kB de RAM de pico e 30,8 kB de flash. Os três vieram idênticos nos três deployments, e agora sabe-se por quê: o seletor de target do Studio estava em **Cortex-M4F a 80 MHz**, que é o padrão da ferramenta. A estimativa é sempre desse núcleo, independentemente do pacote baixado.

Então:

- **Não reporte esses valores como medidos.** São estimativas do simulador do Edge Impulse para um Cortex-M4F de 80 MHz.
- **Nenhum dos seus alvos é esse núcleo.** O RP2040 é Cortex-M0+ a 133 MHz; a Raspberry Pi 4 é Cortex-A72 a 1,5 GHz. A estimativa não descreve nenhum dos dois.
- Os números medidos vêm do Bloco 6 (BitDogLab) e do Bloco 7 (Raspberry Pi 4), com cronômetro e dez execuções.

**Sempre cite o núcleo junto com o número.** "157 ms" sozinho é afirmação falsa sobre o seu hardware; "157 ms estimados para Cortex-M4F 80 MHz" é informação correta e útil.

Isso não torna as estimativas inúteis. Pelo contrário: **que a ferramenta entregue uma previsão de latência, RAM e flash antes de você encostar no hardware é, em si, um dado do critério 3.3.6.** Verifique se o fluxo do Qualcomm AI Hub oferece equivalente. E, depois do Bloco 6, compare a estimativa com o medido — o tamanho do erro é material bom para o Capítulo 5.

---

## Achado técnico relevante: o MFCC domina a latência

Dos 157 ms estimados, **154 ms são do bloco MFCC e 3 ms do classificador**. O pré-processamento consome 98% do tempo.

Isso tem uma consequência direta sobre o Bloco 8, que planeja comparar float32 contra int8 no RP2040. A quantização afeta **a rede neural, não o DSP** — o MFCC continua em ponto flutuante nos dois casos. Então:

- Comparar a **latência total** entre as duas versões mostraria pouca diferença, e levaria à conclusão errada de que a ausência de FPU importa pouco.
- Compare a **latência do classificador isoladamente** (`r.timing.classification`), que é onde o efeito existe.

E há um achado maior escondido aqui, que fortalece o capítulo em vez de enfraquecê-lo: num Cortex-M0+ sem unidade de ponto flutuante, **o gargalo migra para o pré-processamento**. A ausência de FPU não deixa de importar — ela muda de lugar. Essa é uma afirmação mais interessante que a original, e você vai poder sustentá-la com número próprio depois do Bloco 6.

---

## Previsão registrada antes da medição

Escrever a expectativa antes de medir é bom método: transforma o Bloco 6 em teste de hipótese qualitativo, em vez de coleta solta de número. Registre isto hoje, com data, e não mude depois.

O alvo estimado é um Cortex-M4F de 80 MHz; o alvo real da BitDogLab é um Cortex-M0+ de 133 MHz. Três diferenças puxam em direções opostas:

| Fator | Efeito no RP2040 |
|---|---|
| Clock 133 MHz contra 80 MHz | **a favor**, fator ~1,7 |
| M4F tem FPU de ciclo único; M0+ emula float em software | **contra**, e muito — o MFCC é float |
| M4F usa CMSIS-DSP; no M0+ você compila com `EIDSP_USE_CMSIS_DSP=0` | **contra** |

**Hipótese:** a latência medida na BitDogLab deve ficar substancialmente acima dos 157 ms estimados, com a diferença concentrada quase toda no bloco MFCC. O classificador, que é int8 e leva 3 ms na estimativa, deve escalar bem melhor que o DSP.

Se a medição confirmar, você tem evidência própria e direta para a afirmação central do eixo 4.4 — e, de quebra, uma demonstração de que a estimativa do Studio não transfere entre arquiteturas, o que é observação legítima do critério 3.3.6.

Se contrariar, melhor ainda: algo não previsto está acontecendo e vale investigar.

---

## Para a planilha

| Métrica | Valor | Natureza |
|---|---|---|
| Tamanho do modelo (`.tflite` int8) | 6.504 bytes (~6,4 kB) | **medido** no arquivo |
| Tensor arena (pacote MCU, EON) | 6.265 bytes (~6,1 kB) | **medido** nos metadados |
| Latência total | 157 ms (154 MFCC + 3 classificador) | **estimativa** — Cortex-M4F 80 MHz |
| RAM de pico | 15,4 kB (MFCC 15,4 / classificador 3,8) | **estimativa** — Cortex-M4F 80 MHz |
| Flash | 30,8 kB | **estimativa** — Cortex-M4F 80 MHz |
| Acurácia | 88,64% | Model testing, sessão 2 |
| Passos do Bloco 4 | 18 | contagem |

Marque a coluna "natureza" na planilha de verdade, e inclua o núcleo nas três linhas de estimativa. Misturar estimativa com medição é o tipo de erro que a banca encontra; estimativa sem o núcleo a que se refere é pior ainda, porque parece medição.

Note ainda que a arena de 6,1 kB e a RAM de pico de 15,4 kB não se contradizem: a primeira é o grafo da rede, a segunda inclui os buffers de trabalho do MFCC, que dominam.

---

## Bloco 5 — quatro amostras, não uma

Amostras salvas em `C:\Users\timoteo.silva\Desktop\TCC\E4\Audio Samples`, uma por classe, todas verificadas como corretamente reconhecidas pelo modelo:

`sample_hello.wav` · `sample_bye.wav` · `sample_noise.wav` · `sample_unknown.wav`

Isso permite um desenho melhor que o previsto no plano. **Papéis distintos para as quatro:**

| Papel | Amostras | Justificativa |
|---|---|---|
| Latência | `sample_hello`, 10 execuções | a latência não depende do conteúdo — o MFCC e a rede executam o mesmo número de operações qualquer que seja a entrada |
| Paridade entre alvos | as quatro, 1 execução cada | o mesmo modelo deve produzir a mesma saída em hardwares diferentes |

**A paridade com quatro classes substitui com vantagem os 88,64% como controle de sanidade.** Acurácia sobre uma dúzia de amostras é estimativa ruidosa; comparar quatro vetores de probabilidade entre alvos é verificação direta, e falha de implantação aparece imediatamente como classe vencedora trocada.

Critério de aprovação, a registrar antes de medir: **mesma classe vencedora nas quatro amostras, em todos os alvos**, com probabilidades concordando até a terceira ou quarta casa decimal. Não espere igualdade bit a bit — a rede é int8 e determinística, mas o MFCC roda em ponto flutuante, e ordem de operações e biblioteca matemática diferem entre o Cortex-M0+ e o Cortex-A72.

**Ordem das classes:** `bye, hello, noise, unknown` — alfabética, como o modelo as expõe em `ei_classifier_inferencing_categories`. Use essa ordem em tudo para não reinterpretar vetor depois.

### Antes de converter

O caminho tem espaço no nome; use aspas no `cp`. Confira cada arquivo:

```bash
mkdir -p ~/tcc/e4/modelo/amostras && cd ~/tcc/e4/modelo/amostras
cp "/mnt/c/Users/timoteo.silva/Desktop/TCC/E4/Audio Samples/"*.wav .
```

Esperado em cada um: 16000 Hz, 1 canal, 16 bits, 16000 amostras. O script do guia (Bloco 5.2) ajusta o comprimento para exatos 16.000 e **avisa quando trunca ou completa com silêncio** — anote se acontecer, porque alterar a entrada é intervenção, e intervenção não registrada é dado perdido.

O script gera `audio_fixo.h` com as quatro amostras para a BitDogLab e quatro `.bin` para a Raspberry Pi. Cerca de 128 kB de flash no Pico W, folgado nos 2 MB.

## Antes de encerrar o expediente

1. `git add -A && git commit` — inclua os três artefatos de deployment e as quatro amostras `.wav`
2. `history > ~/tcc/e4/logs/historico_sessao3.txt`
3. Dispositivo do seletor de target: **Cortex-M4F 80 MHz** (padrão do Studio) — já registrado acima
4. Três linhas sobre onde parou

Segunda-feira começa no **Bloco 6**, que é o de maior risco. Comece o dia por ele.

---

> Rascunho de bancada. Estimativas e medições estão marcadas separadamente — mantenha essa distinção ao transportar para o Capítulo 5.
