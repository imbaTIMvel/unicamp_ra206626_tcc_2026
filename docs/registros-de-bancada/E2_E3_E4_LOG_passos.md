# Registro de passos — sessões finais: E2, E3 e E4

**Data:** 28/10/2026
**E2** — concluído nos três alvos · **E3** — concluído nos três alvos · **E4** — quarto ponto obtido
**Estado:** fase experimental encerrada.

---

## Identificação de componentes por registrador

Dois módulos de acelerômetro foram usados, e **nenhum dos dois corresponde ao rótulo comercial de forma confiável**. A identificação foi feita por leitura do registrador `WHO_AM_I` (0x75):

| Alvo | Retorno | Peça efetiva |
|---|---|---|
| BitDogLab | `0x70` | **MPU-6500** |
| UNO Q | `0x68` | **MPU-6050** |
| Raspberry Pi 4 | `0x68` | **MPU-6050** |

O módulo emprestado da FEEC foi adquirido como MPU-6500. Dois dos três alvos leram 0x68.

**Declare na dissertação que os componentes foram identificados por leitura de registrador, não pelo rótulo.** Para a medição não há consequência — o mapa de registradores relevante é idêntico e a transação é a mesma —, mas a divergência precisa estar registrada.

---

## E2 — Comunicação I²C

### Resultados, todos a 400 kHz

| Alvo | Mediana | Faixa | Tempo de setup |
|---|---|---|---|
| **UNO Q** (Cortex-M33, Wire) | **210.413 ns** | 397 ns | 14 min 28 s |
| **BitDogLab** (M0+, Pico SDK) | **241.651 ns** | 25 ns | 8 min 24 s |
| **Raspberry Pi 4** (A72, smbus2) | **242.093 ns** | 936 ns | ver sessão anterior |

Os três cabem em **13%**. Os dois primeiros alvos microcontrolados e o alvo Linux diferem entre si em 0,18%.

A BitDogLab apresentou faixa de 25 ns em dez blocos — **0,01%**, a menor dispersão registrada em qualquer eixo deste trabalho.

### Correção de uma afirmação anterior

Na sessão com a Raspberry Pi, decompus os dados das duas frequências em "86 bits de transação mais 27 µs de custo fixo de driver e interpretador". **Essa atribuição estava errada.**

Se os 27 µs fossem software, a BitDogLab — sem kernel, sem interpretador, bare metal — deveria medir perto de 215 µs. Mediu 241,7.

A leitura correta é que essa parcela é **tempo real de barramento** não capturado pela contagem simples de bits: intervalos entre bytes, tempos de setup e hold, condições de start, restart e stop. O protocolo responde por essencialmente todo o tempo em qualquer alvo.

Isso **fortalece** a conclusão do eixo, em vez de enfraquecê-la.

### O achado que só o par E1–E2 produz

| | E1 (acesso a periférico) | E2 (barramento) |
|---|---|---|
| Operação subjacente | ~2,7 µs | ~215 µs |
| Penalidade da UNO Q | **55×** | **nenhuma** |

A mesma biblioteca de alto nível que tornou a UNO Q 55 vezes mais lenta no conversor analógico-digital não impõe penalidade alguma aqui.

**A abstração não tem custo fixo — tem custo relativo à granularidade da operação abstraída.** Quando o gargalo é um protocolo de centenas de microssegundos, dezenas de microssegundos de biblioteca desaparecem; quando a operação custa poucos microssegundos, o mesmo acréscimo a domina.

Orientação concreta para progressão didática: introduzir biblioteca de alto nível em operação de barramento é inócuo; fazê-lo em acesso direto a periférico esconde do estudante uma penalidade de duas ordens de grandeza.

### Beco sem saída — condição de reinício

A leitura canônica de registrador I²C usa *repeated start* entre a escrita do endereço e a leitura dos dados.

- **Pico SDK:** `i2c_write_blocking(..., true)` — funcionou de primeira
- **Núcleo Arduino:** `Wire.endTransmission(false)` — retornou `-1` em todas as leituras

A substituição por `endTransmission()` com stop convencional resolveu. Operação descrita de modo idêntico na documentação das duas plataformas; portabilidade não assegurada nem para operação elementar. **Critério 3.3.3 e 2.4.2.**

Lição de método incorporada ao script: a coluna `OK`/`FALHA` no bloco funcional. Sem ela, a primeira medição inválida passaria.

---

## E3 — Multitarefa e determinismo

### BitDogLab — três condições

| Condição | Mediana | p95 | Máximo |
|---|---|---|---|
| A — ocioso | 0 µs | 0 µs | 0 µs |
| B — carga no **segundo núcleo** | 0 µs | 0 µs | 0 µs |
| C — carga no **mesmo núcleo** | 125 µs | 238 µs | 250 µs |

Em A e B o desvio ficou **abaixo da resolução do instrumento** (1 µs do `time_us_32()`). Não é zero — é menor que o que se consegue medir. Declare assim.

**A condição B é o que distingue este alvo.** Carga intensa ocupando integralmente o core1 não degradou a tarefa periódica do core0. No alvo Linux essa condição não é sequer formulável: o escalonador decide onde cada tarefa executa.

### O padrão da condição C contém o conceito do eixo

Desvios uniformemente distribuídos entre 0 e 250 µs, com mediana em 125 — exatamente metade do máximo.

Isso porque o bloco de trabalho concorrente **não é interrompível** na ausência de escalonador. A tarefa periódica só retoma ao término do bloco corrente, e o atraso é uniforme no intervalo correspondente à sua duração.

**O pior caso é calculável a partir do código, não estimável a partir de observação.** Determinismo não significa ausência de desvio; significa desvio previsível.

### UNO Q — e a descoberta do Zephyr

| Condição | Mediana | p95 | Máximo |
|---|---|---|---|
| A — ocioso | 2 µs | 26 µs | 26 µs |
| C — carga no mesmo núcleo | 40 µs | 98 µs | 137 µs |
| B — núcleo separado | **não aplicável** | | |

O registro de compilação resolveu uma pendência aberta desde a primeira versão do roteiro:

```
Board platform: arduino:zephyr (1.0.0)
/tmp/remoteocd/zephyr-arduino_uno_q_stm32u585xx.elf
```

**O RTOS é o Zephyr.** E a resposta é mais interessante do que a pergunta previa: o Zephyr **tem** escalonador preemptivo e API de threads. A camada Arduino as esconde. Mesmo padrão dos dois bits de resolução do E1 — o recurso existe no sistema e a abstração o oculta.

**Os números denunciam o escalonador.** Na condição ociosa, a BitDogLab deu 0 µs e a UNO Q deu máximo de 26 µs. Algo interrompe periodicamente, e o candidato é o tick do sistema. Bare metal não tem nada ticando.

Sob carga, a UNO Q saiu melhor (137 µs contra 250 µs) — **não é vantagem de arquitetura de concorrência, é clock**: o Cortex-M33 a 160 MHz executa o mesmo bloco mais rápido.

### Comparação final do eixo

| | BitDogLab | UNO Q | Raspberry Pi 4 |
|---|---|---|---|
| Ocioso, máximo | < 1 µs | 26 µs | 1,3 µs |
| Carga mesmo núcleo, máximo | 250 µs | 137 µs | 19.950 µs |
| Carga em núcleo separado | **< 1 µs** | não aplicável | não formulável |
| Pior caso / período de 10 ms | 2,5% | 1,4% | **199%** |

---

## E4 — Quarto ponto de medição, domínio MPU da UNO Q

### Decisão de escopo

O caminho do Qualcomm AI Hub foi substituído pelo runtime genérico Linux ARM64 — o **mesmo `.eim`** usado na Raspberry Pi. Caminho mais curto para número comparável; não substitui a evidência sobre aprisionamento, que permanece documental conforme o plano de riscos.

### Ambiente

| Item | Valor |
|---|---|
| Arquitetura | aarch64 |
| Sistema | **Debian 13 trixie** — idêntico ao da Raspberry Pi |
| Núcleos | 4 |
| RAM | 3,6 GiB (851 MiB já em uso no repouso) |
| Python | 3.13 |

Mesma distribuição da Pi: a comparação entre os dois domínios Linux é quase perfeitamente controlada, variando só o hardware.

### Resultados

```
projeto: TCC_hello_bye | id: 1123849 | deploy: 4
bye     → bye 0.99609  hello 0.00000  noise 0.00000  unknown 0.00000
hello   → bye 0.00000  hello 0.71094  noise 0.08203  unknown 0.20703
noise   → bye 0.00000  hello 0.00000  noise 0.99609  unknown 0.00000
unknown → bye 0.00000  hello 0.02734  noise 0.00391  unknown 0.96875
```

| Métrica | Valor |
|---|---|
| DSP | 3 ms |
| Classificador | < 1 ms (abaixo da resolução do runtime) |
| Tempo de parede, mediana | 9.570,4 µs |
| Mínimo / máximo | 9.439,6 / 9.906,0 µs |
| Dispersão | 4,9% |

### Paridade em três famílias de silício

| Alvo | DSP | Parede | Paridade |
|---|---|---|---|
| BitDogLab (Cortex-M0+) | 920 ms | 931 ms | 4/4 |
| UNO Q (domínio MPU) | 3 ms | 9.570 µs | 4/4 |
| Raspberry Pi 4 (Cortex-A72) | 2 ms | 12.925 µs | 4/4 |

**Coincidência integral até a quinta casa decimal nas três.** Confirmação mais ampla possível, no escopo do trabalho, da reprodutibilidade conferida pela quantização int8.

Note que a UNO Q é 50% mais lenta no DSP mas 26% mais rápida no tempo de parede. São grandezas diferentes: o DSP mede o processador; o tempo de parede inclui IPC e serialização de 16.000 floats por socket.

### Atrito de instalação — comparação controlada

Mesma distribuição, mesmo pacote, resultado diferente:

| | Raspberry Pi 4 | UNO Q |
|---|---|---|
| `portaudio19-dev` | instalou normalmente | **conflito insolúvel de versões** |
| Causa | — | imagem do fabricante mistura backports com a base estável |
| Solução | — | forçar `libasound2-dev=1.2.16.1-1~bpo13+1` |

O erro completo do `apt` está nas anotações de bancada e deve ser citado: é a evidência de que **o atrito não vem do software, vem da imagem que o fabricante entrega**.

---

## O achado estrutural da UNO Q: interdependência entre domínios

Os registros de compilação revelaram o que a análise documental não antecipara. **Os dois domínios não são apenas integrados — são interdependentes.**

```
Info : Linux GPIOD JTAG/SWD bitbang driver (libgpiod v2)
Info : [stm32u5.cpu] Cortex-M33 r0p4 processor detected
Info : TZEN = 0 : TrustZone disabled by option bytes
Info : RDP level 0 (0xAA)
Info : flash size = 2048 KiB, dual-bank
Container e3_tarefas-main-1  Started
```

**Quatro constatações:**

**1. A gravação do microcontrolador é feita pelo domínio Linux**, emulando SWD por software em pinos de uso geral. Não há programador dedicado.

**2. O canal serial atravessa o domínio Linux.** Por isso o `while (!Serial)` nunca sinaliza e foi preciso `delay(5000)`. Explica também o tempo de setup maior.

**3. Cada aplicação roda em contêiner Docker**, com rede virtual própria. O `hostname -I` devolveu duas interfaces, incluindo a bridge `172.17.0.1`.

**4. O custo é mensurável.** O sketch de ler acelerômetro ocupou **91 kB de flash e 38 kB de RAM**, por inclusão automática de seis bibliotecas de ponte — `Arduino_RouterBridge`, `Arduino_RPClite`, `MsgPack`, `DebugLog`, `ArxTypeTraits`, `ArxContainer`. **Nenhuma relacionada a I²C.**

**Sem o domínio Linux em operação, não se grava nem se comunica com o microcontrolador.**

Contraste: o arranjo Pico W mais Raspberry Pi mantém os domínios fisicamente separáveis, cada um programável de forma independente; e a placa-portadora fala direto com o RP2040 por interface padronizada de armazenamento em massa, sem camada intermediária.

É prós e contras da mesma decisão de projeto: a integração entrega uma plataforma única e entrega junto uma dependência que o estudante não escolheu e cujo custo não lhe é apresentado.

### Dados de datasheet resolvidos pelos registros

- STM32U575/U585, revisão U, **2 MB de flash em dual-bank**
- **TrustZone desabilitado** por option bytes — recurso do silício removido por decisão de projeto
- **RDP level 0** — proteção de leitura desabilitada, flash livremente inspecionável

---

## Incorporado ao TCC

Treze parágrafos novos em `EA019_TCC_Timoteo_Altoe_v5_experimentos_completos.docx`, com alterações rastreadas:

**4.1.1** (2) — interdependência entre domínios e seu custo mensurável
**4.2.3** (4) — resultados nos três alvos, correção da decomposição, custo relativo da abstração, descontinuidade do *repeated start*
**4.3.3** (4) — três condições no RP2040, o que a condição B isola, o padrão uniforme da condição C, Zephyr e threads ocultas
**4.4.3** (3) — quarto ponto de medição, paridade em três famílias de silício, delimitação do que não foi demonstrado

---

## Estado final dos experimentos

| | BitDogLab | UNO Q | Raspberry Pi 4 |
|---|---|---|---|
| **E1** | 2.702 ns | 150.010 ns | 7.357 ns |
| **E2** | 241.651 ns | 210.413 ns | 242.093 ns |
| **E3** | < 1 / 250 µs | 26 / 137 µs | 1,3 / 19.950 µs |
| **E4** | 931 ms | 9.570 µs (MPU) | 12.925 µs |

**Doze medições em três plataformas. Fase experimental encerrada.**

Permanece documental: o caminho do Qualcomm AI Hub, conforme o plano de riscos.

---

> Rascunho de bancada. Medições próprias, sem tratamento estatístico, coerentes com o princípio do Capítulo 4.
