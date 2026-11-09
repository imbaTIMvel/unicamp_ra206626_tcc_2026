# E1 — Sensores e atuadores

Leitura de uma fonte analógica e acionamento de um LED por modulação de largura de pulso, nas três plataformas. O experimento mede o custo da camada de abstração em uma operação de granularidade fina, e produz as listagens que sustentam a comparação de concisão de código.

## Resultados

| Alvo | Latência mediana | Faixa | Linhas de código | Abstração |
|---|---|---|---|---|
| BitDogLab (RP2040, 133 MHz) | **2 702 ns** | 16 ns (0,59%) | 16 + 9 de compilação | nível 1 |
| Raspberry Pi 4 (A72, 1,8 GHz) | **7 357 ns** | 92 ns (1,25%) | 11 (somente leitura) | nível 1 |
| Arduino UNO Q (Cortex-M33) | **150 010 ns** | 100 ns (0,067%) | 8 | nível 3 |

Tempo até o primeiro programa funcional: 9 min 25 s na BitDogLab; 36 min 28 s na UNO Q, dos quais 15 min 19 s de atualização obrigatória de firmware.

**O hardware mais capaz foi 55 vezes mais lento.** O Cortex-M33 tem unidade de ponto flutuante e é arquiteturalmente superior ao Cortex-M0+; a diferença está na camada de biblioteca, não no silício.

## Montagem

Divisor resistivo com fotorresistor, alimentado em 3,3 V, e LED em série com resistor de 220 Ω.

```
3V3 ──── LDR ────┬──── 10 kΩ ──── GND
                 │
                 └──► entrada analógica
```

| Alvo | Entrada analógica | Saída PWM |
|---|---|---|
| BitDogLab | **eixo do joystick de bordo (GP26, ADC0)** | LED RGB de bordo (GP13) |
| UNO Q | A0 | D9 |
| Raspberry Pi 4 | canal IO0 do AD5592R (SPI) | não aplicável |

**Por que o joystick na BitDogLab.** O RP2040 expõe três entradas analógicas de uso geral, e a placa-portadora ocupa as três: dois eixos de joystick e o microfone. Apenas a do microfone chega aos conectores externos, compartilhada com a saída do amplificador, e verificou-se em bancada que um divisor externo não consegue impor tensão nesse nó — a tensão permanece fixa em ~1,65 V. O programa `teste-adc/teste.c` reproduz essa verificação.

A substituição não compromete a medição, e isso foi confirmado experimentalmente: dez execuções com o eixo em posições muito distintas deram mediana idêntica, porque o tempo de conversão independe do valor convertido.

Ligação do AD5592R à Raspberry Pi: SCK no pino 23, MOSI no 19, MISO no 21, CS no 24, alimentação no 17, terra no 20 e **RST no 3V3** — sem isso o componente permanece em reinicialização.

## Arquivos

```
bitdoglab/
  e1_sensores_min_func.c    versão mínima funcional (a das listagens do trabalho)
  e1_sensores.c             versão instrumentada (a que produziu as medições)
  CMakeLists.txt
  teste-adc/                verificação da entrada analógica compartilhada
raspberry-pi/
  e1_teste.py               teste de vida do conversor externo
  e1_medir.py               medição com barramento a 1 MHz
  e1_medir_10m.py           medição com barramento a 10 MHz
uno-q/e1_sensores/          aplicação exportada do Arduino App Lab
resultados/
```

## Como reproduzir

**BitDogLab.** Requer o Pico SDK com `PICO_SDK_PATH` definido.

```bash
cd bitdoglab && cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
mkdir build && cd build && cmake .. -DPICO_BOARD=pico_w && make -j$(nproc)
```

Grave o `.uf2` com a placa em BOOTSEL e acompanhe a porta serial a 115200 baud.

**UNO Q.** Importe `uno-q/e1_sensores` no App Lab e execute. O monitor serial precisa estar aberto antes do início da impressão.

**Raspberry Pi 4.** Habilite o SPI, instale `python3-spidev` e fixe o governador em `performance` antes de medir — ele não sobrevive a reinício.

```bash
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
python3 e1_medir_10m.py
```

## Ressalvas

A implementação do Raspberry Pi realiza **apenas a leitura**, sem acionamento de atuador; seu valor subestima o laço completo e não é diretamente comparável aos demais.

A fonte de sinal difere entre alvos, pelo motivo exposto acima.

Com o conversor externo em faixa de 0 a 2×V_REF e o divisor alimentado em 3,3 V, utiliza-se pouco mais de um quarto dos doze bits disponíveis.

O barramento serial do conversor operou a 10 MHz, contra teto de 20 MHz especificado na Tabela 3 do datasheet do AD5592R, com alimentação em 3,3 V. A 1 MHz, frequência arbitrada na primeira tentativa, 68% da latência medida era o próprio barramento.

Na UNO Q, o conversor opera em doze bits, mas a biblioteca descarta dois por padrão: `analogReadResolution(12)` é necessário, e sem ele as leituras cobrem faixa de dez bits sem qualquer sinalização.
