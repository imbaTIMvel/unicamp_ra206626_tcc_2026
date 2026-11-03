# Registro de passos — E1, Sessão 2

**Data:** _______ · **Alvo:** Raspberry Pi 4 Model B — **concluído**
**Estado do E1:** três alvos medidos. Experimento encerrado.

---

## Contexto da sessão

Sessão realizada em **máquina Windows nova**, após migração do ambiente. Arquivos transferidos por `tar.gz`; toolchain, WSL, PuTTY e App Lab já instalados previamente. Boot da Raspberry Pi restabelecido após regravação do cartão.

Refrigeração ativa presente na placa **desde a primeira medição**, alimentada pelos pinos 4 e 5V e 6 GND, por recomendação do orientador. Condição uniforme em todas as sessões.

---

## Passos — alvo Raspberry Pi 4

| # | Ação |
|---|---|
| 20 | Conectar por SSH e obter o endereço (192.168.2.229) |
| 21 | Habilitar SPI por `raspi-config` |
| 22 | `apt update && upgrade && install python3-spidev` |
| 23 | Fixar o governor em `performance` |
| 24 | **Conectar o RST do módulo ao 3V3** |
| 25 | Escrever o acesso ao AD5592R por `spidev` |
| 26 | Teste de vida e medição |

**Total do alvo Raspberry Pi 4 nos dois experimentos: 26 passos.**

Reinstalações decorrentes da regravação do cartão são **retrabalho** e já foram contadas na sessão 6 do E4.

---

## Diagnóstico que destravou o alvo

O módulo não respondia e o LED indicador de alimentação permanecia apagado. Duas causas, resolvidas em sequência:

**RST flutuante.** O datasheet exige o pino de reset em nível alto para operação normal; uma borda de descida reinicia todos os registradores. Com o pino solto, nenhuma configuração persistia.

**Alimentação.** Conferida por medição direta entre +3V3 e GND do módulo, após identificação dos pinos por continuidade e não por contagem no conector.

---

## Teste de vida — resposta ao estímulo

| Condição | Leitura (12 bits) |
|---|---|
| Lanterna sobre o LDR | ~2.590 |
| Luz ambiente | ~2.230 |
| LDR tapado com a mão | ~1.285 |

Canal reportado 0 em todas as leituras, como esperado.

**Verificação de coerência:** com faixa 0 a 2×V_REF, o fundo de escala é 5 V e cada contagem vale 1,22 mV. Assim, 2.232 equivale a ~2,72 V e 1.211 a ~1,48 V — próximo dos 2,787 V e 1,4653 V medidos com multímetro sobre o mesmo divisor durante o teste da BitDogLab. A escala está correta.

---

## Medição de latência

### Primeira série — SPI a 1 MHz

| | |
|---|---|
| Mediana | 23.245 ns |
| Mínimo / máximo | 23.171 / 23.366 ns |
| Faixa | 195 ns (0,84%) |

Segunda execução do mesmo script: mediana 23.203 ns, diferença de 0,18% entre séries.

### Segunda série — SPI a 10 MHz

| | |
|---|---|
| **Mediana** | **7.357 ns** |
| Mínimo / máximo | 7.339 / 7.431 ns |
| Faixa | 92 ns (1,25%) |

`throttled=0x0` em todas as execuções. Clock da CPU em 1,8 GHz, governor em `performance`.

---

## Achado metodológico: 68% do primeiro número era configuração

A queda de 15,9 µs entre as duas séries corresponde quase exatamente à teoria: uma transação de 16 bits custa 16 µs a 1 MHz e 1,6 µs a 10 MHz, diferença de 14,4 µs.

Ou seja, **o valor de 1 MHz que eu arbitrei sem justificativa respondia por mais de dois terços da latência medida.** Não era característica da plataforma.

O que resta dos 7.357 ns é o dado honesto: cerca de 1,6 µs de barramento e **aproximadamente 5,8 µs de travessia de driver de kernel mais interpretador Python**. Esse é o custo real de fazer leitura analógica no domínio Linux.

**Segunda ocorrência do mesmo padrão.** No E4, o `EIDSP_QUANTIZE_FILTERBANK` foi testado nas duas configurações e provou-se inerte; aqui, o clock do barramento provou-se dominante. Os dois episódios juntos sustentam uma advertência metodológica explícita no Capítulo 4: parâmetros arbitrados sem justificativa podem dominar o resultado, e só variá-los deliberadamente separa a configuração da característica sob estudo.

### Validação do clock empregado

Dúvida levantada e resolvida: o datasheet traz duas tabelas de temporização, com ciclo mínimo de SCLK em leitura de 50 ns e 100 ns. **São peças diferentes, não condições diferentes.** A Tabela 3 é do AD5592R; a Tabela 4 é do AD5592R-1, variante com pino VLOGIC separado. O módulo empregado é AD5592R**BRUZ**, sem o sufixo, portanto Tabela 3 — onde o mínimo é 50 ns em qualquer faixa de alimentação, equivalente a teto de 20 MHz.

Operando a 10 MHz, com o módulo alimentado em 3,3 V, há folga de duas vezes. **Número válido.**

Frase para o trabalho: *clock do barramento serial em 10 MHz, contra teto de 20 MHz especificado na Tabela 3 do datasheet do AD5592R, com alimentação em 3,3 V.*

---

## Quadro final do E1

| Alvo | Mediana | Razão vs. BitDogLab | Dispersão | Abstração |
|---|---|---|---|---|
| **BitDogLab** (M0+ 133 MHz) | **2.702 ns** | 1× | 0,59% | nível 1 |
| **RPi 4 + AD5592R** (A72 1,8 GHz) | **7.357 ns** | 2,7× | 1,25% | nível 1 |
| **UNO Q** (Cortex-M33) | **150.010 ns** | 55,5× | 0,067% | nível 3 |

| | BitDogLab | UNO Q | RPi 4 |
|---|---|---|---|
| Tempo de setup | 9 min 25 s | 36 min 28 s | ver sessão 6 do E4 |
| Sem atualização obrigatória | — | 21 min 09 s | — |
| Fonte analógica | joystick de bordo | LDR + 10 kΩ | LDR + 10 kΩ |
| Conversor | interno, 12 bits | interno, 12 bits (2 ocultos) | **externo**, 12 bits |

---

## Os três achados do E1

**1. O hardware mais capaz foi 55 vezes mais lento.** O Cortex-M33 com FPU da UNO Q contra o Cortex-M0+ do RP2040. A diferença está na biblioteca, não no silício.

**2. O conversor externo bateu o interno por vinte vezes.** O Raspberry Pi, com silício externo, SPI, driver de kernel e Python, foi vinte vezes mais rápido que a UNO Q acessando o conversor do próprio chip. A camada de abstração do núcleo Arduino custa mais que uma chamada de sistema. **Este é o resultado mais contraintuitivo do experimento.**

**3. Dois bits escondidos.** O conversor do STM32U585 é de 12 bits e o núcleo Arduino descartava dois por padrão, sem sinalizar. Resolve pendência da seção 7 do roteiro.

### Achado transversal entre os eixos

A dispersão do alvo Linux foi de 1,25% neste experimento, contra 9,2% na primeira execução do E4. A explicação plausível é a duração: um laço de microssegundos cabe entre dois eventos de escalonamento; uma inferência de dezenas de milissegundos atravessa vários. **O determinismo do domínio Linux degrada com a duração da tarefa** — observação que só a comparação entre os dois eixos permite fazer.

---

## Limitações a declarar

- **Fontes analógicas diferentes.** Joystick na BitDogLab, por impossibilidade física de usar entrada externa; LDR nos outros dois. A substituição foi validada experimentalmente: cem amostras com o joystick em estados distintos deram mediana idêntica, confirmando que o tempo de conversão independe do valor convertido.
- **Laço não equivalente no Raspberry Pi.** Nos alvos microcontrolados o laço é leitura mais escrita de PWM; no Raspberry Pi, apenas leitura. São grandezas diferentes e precisam ser reportadas como tais.
- **Faixa útil reduzida no Raspberry Pi.** Com fundo de escala em 5 V e divisor alimentado em 3,3 V, usa-se pouco mais de um quarto dos 12 bits. Consequência direta do conversor externo ter referência própria.
- **Boot pelo slot nativo do MicroSD**, confirmado. Condição padrão, sem ressalva a declarar.
- **Logs seriais** transcritos manualmente, sem arquivo primário preservado em parte das sessões.

---

## Incorporado ao TCC

Quatro parágrafos novos em 4.1.1, como alterações rastreadas, em `EA019_TCC_Timoteo_Altoe_v3_E1completo.docx`:

- o resultado dos vinte vezes e a leitura de que a camada de software pesa mais que a topologia do hardware;
- a advertência metodológica sobre parâmetros arbitrados, com os dois episódios;
- a dispersão comparada entre os três alvos e a degradação do determinismo com a duração da tarefa;
- a contagem de linhas de código, a relação inversa entre concisão e desempenho, e as três ressalvas da contagem.

---

## Contagem de linhas de código

**Regra aplicada**, idêntica nas três plataformas: apenas linhas executáveis necessárias à tarefa; excluídos comentários, linhas em branco, instrumentação de medição e linhas contendo somente delimitadores de bloco. Diretivas de inclusão de cabeçalho **contam**. Arquivos de build contados em separado.

| Alvo | Linhas | Build | Observação |
|---|---|---|---|
| **UNO Q** (`sketch.ino`) | **8** | — | 7 sem `analogReadResolution(12)` |
| **BitDogLab** (`e1_sensores_min_func.c`) | **16** | 9 (`CMakeLists.txt`) | 3 são `#include`; 9 são configuração de periférico |
| **Raspberry Pi 4** (`e1_medir_10m.py`) | **11** | — | **somente leitura**, sem atuador; 5 são configuração do conversor externo |

### A relação inversa

**A plataforma que exige metade das linhas é cinquenta e cinco vezes mais lenta.**

| Alvo | Linhas | Latência | Abstração |
|---|---|---|---|
| UNO Q | 8 | 150.010 ns | nível 3 |
| Raspberry Pi 4 | 11 (só leitura) | 7.357 ns | nível 1 |
| BitDogLab | 16 + 9 | 2.702 ns | nível 1 |

A abstração cobra exatamente aquilo que promete poupar: a configuração explícita do periférico ocupa 9 das 16 linhas no Pico SDK e, no núcleo Arduino, é executada internamente a cada chamada, com parâmetros que o programador não escolheu e não vê.

E entre essas nove linhas está a definição da resolução — cuja omissão no caminho abstrato produziu a perda silenciosa de dois bits. **O que a biblioteca poupa em digitação, cobra em opacidade.**

### Ressalvas da contagem

- A implementação do Raspberry Pi **não é equivalente**: realiza só a leitura. Acrescentar o atuador elevaria linhas e latência, sem alterar a ordenação.
- Cinco das onze linhas do Raspberry Pi são palavras de configuração do conversor externo — reset, referência, faixa, pino e sequência. Existem apenas porque o periférico não está no processador. É o ponto arquitetural do eixo expresso em contagem de código.
- Incluir os `#include` favorece as plataformas abstratas só na aparência: a ausência deles no Arduino não é economia de digitação, é a biblioteca escolhendo de antemão quais periféricos existem.

---

## Códigos de referência para o Capítulo 5

**UNO Q** — versão mínima funcional, sem instrumentação:

```cpp
const int PIN_LDR = A0;
const int PIN_LED = 9;
void setup() {
  pinMode(PIN_LED, OUTPUT);
  analogReadResolution(12);
}
void loop() {
  analogWrite(PIN_LED, map(analogRead(PIN_LDR), 0, 4095, 0, 255));
  delay(50);
}
```

O `map` já está correto com 4095 — a correção pendente da sessão 1 **está resolvida**.

**BitDogLab** — `e1_sensores_min_func.c`, 16 linhas mais 9 de `CMakeLists.txt`.

**Raspberry Pi 4** — `e1_medir_10m.py`, trecho de configuração e leitura, descontada a instrumentação.

---

## Pendências do E1

- [x] Contagem de linhas de código — **resolvida**
- [x] Correção do `map` no sketch da UNO Q — **já estava correta**
- [x] Confirmação do boot — **slot nativo do MicroSD**
- [ ] PWM no laço do Raspberry Pi: **decidido reportar as duas medidas em separado**, com a diferença declarada. O achado principal não depende disso — a Pi faz só a leitura em 7.357 ns enquanto a UNO Q faz leitura mais PWM em 150.010 ns, e acrescentar o atuador à Pi reduziria a razão sem mudar a conclusão.

## Próximos experimentos

**E3** é o mais barato: não exige hardware novo, e na BitDogLab a matriz em PIO mais o segundo núcleo dão as três camadas de concorrência num programa só.

**E2** usa o MPU-6500 pelo conector I2C 0 da BitDogLab. Lembrar que o `WHO_AM_I` responde 0x70, não 0x68, e que bibliotecas escritas para o MPU-6050 abortam na verificação de identidade — beco sem saída previsível, e material do critério de portabilidade.

---

> Rascunho de bancada. Medições próprias, sem tratamento estatístico, coerentes com o princípio do Capítulo 4.
