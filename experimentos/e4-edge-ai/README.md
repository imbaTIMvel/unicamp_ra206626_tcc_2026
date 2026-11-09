# E4 — Edge AI e TinyML

Implantação do mesmo modelo quantizado de detecção de palavra-chave em três alvos, sobre entrada idêntica, comparando latência, memória, tamanho do artefato e esforço de configuração. É o eixo prioritário do trabalho: aquele em que a documentação menos permite antecipar resultado.

### Modelo

Valores estimados pela ferramenta (Edge Impulse), para um núcleo Cortex-M4F @ 80 MHz.

| Modelo | Extração de características (MFCC) | Inferência | Tempo total | Memória de execução (MFCC + Infer.) | Memória de programa (inferência) | Precisão |
| --- | --- | --- | --- | --- | --- | --- |
| Quantizado (int8) | 154 ms | 3 ms | 157 ms | 15,4 kB + 3,8 kB | 30,8 kB | 88,64% |
| Não otimizado (float32) | 154 ms | 37 ms | 191 ms | 15,4 kB + 7 kB | 27,9 kB | 88,64% |

## Resultados

| Alvo | Extração de características | Inferência | Tempo total | Memória de execução |
|---|---|---|---|---|
| BitDogLab (Cortex-M0+) | **920 ms** | 11 ms | 931 ms | ≈ 23 kB |
| UNO Q, domínio de aplicação | 3 ms | < 1 ms | 9.570 µs | não medida |
| Raspberry Pi 4 (Cortex-A72) | 2 ms | < 1 ms | 12.925 µs | ≈ 61 MB |

O tempo total do alvo microcontrolado é integralmente interno ao dispositivo; nos alvos Linux inclui comunicação entre processos e serialização da entrada, e por isso não é diretamente comparável.

### Verificação de paridade

| Entrada | bye | hello | noise | unknown |
|---|---|---|---|---|
| `sample_bye` | 0,99609 | 0,00000 | 0,00000 | 0,00000 |
| `sample_hello` | 0,00000 | 0,71094 | 0,08203 | 0,20703 |
| `sample_noise` | 0,00000 | 0,00000 | 0,99609 | 0,00000 |
| `sample_unknown` | 0,00000 | 0,02734 | 0,00391 | 0,96875 |

**Valores idênticos nos três alvos, até a quinta casa decimal.** Todas as probabilidades são múltiplos exatos de 1/256, resolução da desquantização de oito bits — o que explica a coincidência apesar das diferenças de arquitetura no pré-processamento em ponto flutuante. A quantização, usualmente apresentada como concessão de acurácia, oferece também reprodutibilidade exata entre arquiteturas.

### Efeito da quantização no alvo microcontrolado

| Versão | Extração | Inferência | Memória de programa |
|---|---|---|---|
| Inteiros de 8 bits | 924 ms | **11 ms** | 246.328 bytes |
| Ponto flutuante de 32 bits | 926 ms | **41 ms** | 241.784 bytes |

Fator de 3,7 na inferência; o tempo total variou apenas 3,5%, o que ocultaria o fenômeno caso fosse a grandeza reportada. Ambas as medições com instrumentação de memória ativa.

**O gargalo é o pré-processamento, não a rede.** Com o modelo já convertido para aritmética inteira, a extração de características — que permanece em ponto flutuante — responde por cerca de 99% do tempo no alvo mais restrito.

## O modelo

Projeto `TCC_hello_bye`, identificador 1123849, exportação versão 4.

- Classes: `bye`, `hello`, `noise`, `unknown`
- Processamento: MFCC — janela de 1000 ms, passo de 500 ms, 16 kHz
- Quantização inteira de 8 bits, com compilador de grafo ativo
- Arquivo `.tflite`: 6.504 bytes; arena do grafo compilado: 2.176 bytes

Treinado com gravações do próprio autor para as duas palavras-chave, mais ruído de fundo e palavras fora do vocabulário. Acurácia de 88,64% no conjunto de teste — número interno de verificação, não comparável com a literatura.

## Entrada fixa, não microfone

As medições usam **quatro trechos de áudio pré-gravados, embutidos no firmware**, e não captura ao vivo. A razão é dupla: escrever caminho de captura para cada alvo consumiria a maior parte do esforço sem medir nenhum critério, e apenas entrada idêntica torna as latências comparáveis.

Os quatro arquivos vieram já em 16 kHz, mono, com 16.000 amostras — **nenhum exigiu truncamento ou preenchimento**. A entrada submetida aos três alvos é o áudio original, sem intervenção. Hashes em `dataset/amostras/CHECKSUMS.md5`; qualquer divergência invalida a comparação.

A latência é medida sobre uma única amostra, repetida dez vezes, porque o tempo de processamento não depende do conteúdo da entrada. As quatro servem à verificação de paridade.

## Arquivos

```
bitdoglab/int8/           projeto que produziu as medições reportadas
bitdoglab/float32/        variante não quantizada
bitdoglab/prova-de-vida/  verificação da toolchain antes do experimento
raspberry-pi/e4_medir.py
uno-q/e4_medir.py
modelo/                   pacotes de implantação, .eim e conversor de áudio
dataset/amostras/         entrada comum, em .wav e .bin
```

Os arquivos terminados em `-original` preservam o que o pacote de implantação entrega, ao lado das versões modificadas. A comparação entre eles documenta o que foi necessário escrever.

## Como reproduzir

**BitDogLab.** Descompacte `modelo/tcc_hello_bye-cpp-mcu-int8-impulse-#1.zip` dentro de `bitdoglab/int8/`, copie o `pico_sdk_import.cmake` do SDK e compile. Confirme que o CMake relata **162 fontes** antes de prosseguir.

Três definições são obrigatórias e constam do `CMakeLists.txt`:

```cmake
EIDSP_USE_CMSIS_DSP=0
EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=0
EI_CLASSIFIER_USE_FULL_TFLITE=0
```

Sem a primeira, a compilação quebra: o Cortex-M0+ não possui as extensões que a biblioteca de rotinas otimizadas pressupõe. **São 661 arquivos excluídos do build** por esse motivo — a ausência de unidade de ponto flutuante expressa em contagem de arquivos.

O arquivo `ei_porting_pico.cpp` precisou ser escrito: a biblioteca distribui vinte e seis camadas de adaptação ao hardware e **nenhuma contempla o RP2040**. A camada genérica disponível declara os temporizadores como funções que retornam zero, o que zeraria as duas métricas de tempo do experimento.

O `CMakeLists.txt` distribuído com o pacote não é utilizável: pressupõe o Zephyr e exige um alvo de compilação criado previamente. Está preservado como `CMakeLists.ei-original.txt`.

**Alvos Linux.** Requerem `edge_impulse_linux` e o `.eim` AARCH64.

```bash
pip3 install edge_impulse_linux --break-system-packages
chmod +x modelo.eim
python3 e4_medir.py
```

Espere dependências não declaradas — `pyaudio`, `portaudio19-dev`, `six` e OpenCV foram necessárias, nenhuma relacionada a inferência sobre arquivo. No domínio de aplicação da UNO Q, a instalação do `portaudio19-dev` encontra conflito de versões na imagem do fabricante, contornável forçando `libasound2-dev=1.2.16.1-1~bpo13+1`.

## Ressalvas

O caminho de otimização proprietário do processador de aplicação **não foi percorrido**. A medição na UNO Q usou o mesmo runtime genérico empregado na Raspberry Pi, o que informa sobre viabilidade por via portável, mas nada afirma sobre o desempenho do ferramental do fabricante nem sobre o grau de aprisionamento que seu uso implicaria.

O **domínio de tempo real da UNO Q ficou fora das medições**. É a limitação mais específica da comparação: seria o único caso em que dois microcontroladores de arquiteturas distintas poderiam ser confrontados sob o mesmo pipeline, já que o modelo quantizado é em princípio compatível com ambos.

As estimativas da ferramenta de desenvolvimento referem-se a um Cortex-M4F de 80 MHz, que **não corresponde a nenhum dos alvos**. O erro observado foi de cerca de seis vezes ao comparar com a medição real. Na planilha de dados, essas linhas estão marcadas como `estimado`.

Com janelas de um segundo e inferência de 0,93 s, a ocupação do processador chega a 93% no alvo microcontrolado. A implementação empregada não oferece desempenho suficiente para detecção contínua com janelas sobrepostas nesse alvo — limitação da implementação, não do microcontrolador.

As duas versões do modelo produziram classificações equivalentes para as entradas testadas, o que verifica o pipeline e a portabilidade da implantação. **Não é conclusão sobre acurácia**: quatro amostras de um único locutor são insuficientes para tanto.

A medição de memória de execução apoia-se em consulta ao alocador e não captura buffers em pilha. A primeira tentativa, com contador manual de alocações, produziu 315.553 bytes em um dispositivo com 264 kB de SRAM — valor fisicamente impossível, documentado em `docs/becos-sem-saida.md`.
