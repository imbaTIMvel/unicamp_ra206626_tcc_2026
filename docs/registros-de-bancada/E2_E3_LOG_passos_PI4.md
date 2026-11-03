# Registro de passos — E2 e E3, Raspberry Pi 4

**Data:** _______ · **E2** — concluído, duas frequências · **E3** — concluído
**Estado:** os três experimentos estão fechados no alvo Linux. Faltam E2 e E3 na BitDogLab e na UNO Q.

---

## Montagem

| Módulo | Sinal | Pino da Pi |
|---|---|---|
| MPU-6050 | VCC | 1 (3V3, compartilhado com o RST do AD5592R) |
| MPU-6050 | GND | 9 |
| MPU-6050 | SCL | 5 (GPIO 3) |
| MPU-6050 | SDA | 3 (GPIO 2) |
| LED + 220 Ω | anodo | 12 (GPIO 18) |

O AD5592R do E1 permaneceu montado; os pinos de I²C e SPI não conflitam.

---

## Correção de identificação do componente

O módulo foi adquirido como **MPU-6500**. A leitura do registrador `WHO_AM_I` (0x75) retornou **0x68**, valor correspondente ao **MPU-6050**.

```
imbatimvel@rasppi4:~ $ i2cget -y 1 0x68 0x75
0x68
```

Módulos de baixo custo frequentemente trazem peça diferente da anunciada. **Declare na dissertação que o componente foi identificado por leitura de registrador, e não pelo rótulo comercial.** É rigor que a banca valoriza, e resolve a favor do plano original, que previa o MPU-6050.

Consequência prática: o beco sem saída antecipado — bibliotecas para MPU-6050 abortando na verificação de identidade de um 6500 — não ocorreu.

---

## Atrito de bancada durante a preparação do I²C

| Sintoma | Causa | Resolução |
|---|---|---|
| `i2cdetect` com grade vazia | módulo sem responder | verificação de alimentação e fiação |
| `i2cdetect` com **todos** os endereços respondendo | barramento preso em nível baixo, SDA em curto | refazer a ligação |
| `/dev/i2c-1` ausente após o boot | linhas do `config.txt` comentadas | `sudo dtparam i2c_arm=on` em tempo de execução |

O segundo item é diagnóstico útil de registrar: **grade cheia e grade vazia são sintomas opostos com a mesma aparência de falha.** Vazia significa ausência de resposta; cheia significa linha travada, que o utilitário interpreta como confirmação em todos os endereços.

---

## E2 — resultados

### Teste de vida

Em repouso, a magnitude do vetor de aceleração ficou em ≈15.970 contra os 16.384 correspondentes a 1 g na escala de ±2 g — coerente. Movendo o módulo, os três eixos variaram por toda a faixa.

### Latência, duas frequências

| Clock do barramento | Mediana | Mínimo / máximo |
|---|---|---|
| 100 kHz (padrão) | **886.677 ns** | 886.212 / 887.016 |
| 400 kHz | **242.093 ns** | 241.482 / 242.418 |

`throttled=0x0`, governor em `performance`, clock da CPU em 1,8 GHz, nas duas séries.

### A decomposição que as duas frequências permitem

A razão medida é **3,66**, não 4. Com dois pontos, resolve-se o sistema e separa-se a parcela proporcional à frequência daquela que independe dela:

| Componente | Valor |
|---|---|
| Transação | ≈ 86 bits |
| Custo fixo (driver + interpretador) | ≈ 27 µs |
| Fração do barramento a 100 kHz | 96,9% |
| Fração do barramento a 400 kHz | 88,8% |

**Isto é o melhor resultado metodológico do E2**, e nenhum outro eixo permitiu obtê-lo: medir em duas condições separa configuração de característica de forma aritmética, não argumentativa.

### Dois achados

**1. O gargalo é o protocolo, não a plataforma.** Confirma a previsão documental. Qualquer das três placas, no mesmo barramento e frequência, estaria limitada pelos mesmos 86 bits. A diferença entre elas só pode aparecer nos 27 µs de custo fixo.

**2. O custo fixo do I²C é cinco vezes o do SPI.** No mesmo alvo e mesmo interpretador, o E1 mediu ≈6 µs de overhead no caminho SPI contra os ≈27 µs aqui. Hipótese plausível, **não verificada**: tratamento por interrupção no controlador I²C, ausente no caminho SPI.

### Atrito silencioso — material do critério 3.3.3

A frequência do barramento **não é configurável em tempo de execução**: depende de parâmetro lido pelo firmware no boot. A primeira tentativa falhou porque a linha no `config.txt` permanecia precedida de `#`, condição padrão da distribuição:

```
6:#dtparam=i2c_arm=on,i2c_arm_baudrate=400000
8:#dtparam=spi=on
```

O arquivo aparentava configurado, a sintaxe estava correta, nenhuma mensagem de erro foi emitida. A ausência de efeito só apareceu na inspeção direta do registrador de divisão de relógio:

```bash
sudo cat /sys/kernel/debug/clk/clk_summary | grep i2c_div
```

**Falha silenciosa é pior que falha visível**, porque não há o que depurar. Natureza distinta das ocorrências dos outros eixos.

Também se tentou aplicar a frequência sem reinício, via `rmmod`/`modprobe i2c_bcm2835 baudrate=400000`. **Não funcionou** — o clock vem do device tree, não do parâmetro de módulo. Registre: só reinício aplica.

---

## E3 — resultados

### Condições

Tarefa periódica de 10 ms, 500 repetições, LED alternado a cada ciclo. Carga gerada por `stress-ng --cpu 16` em quatro núcleos. Governor em `performance`. `throttled=0x0` ao final.

| Condição | Mediana | p95 | Máximo |
|---|---|---|---|
| Ocioso | **0,4 µs** | 0,5 µs | 1,3 µs |
| Sob carga | **6.106,9 µs** | 15.956,7 µs | **19.950,3 µs** |

### O número decisivo não é o fator de 15.000

É a relação entre o pior desvio e o período: **19,95 ms de atraso sobre tarefa de 10 ms significam dois períodos inteiros perdidos.** Não é degradação de precisão — é perda de prazo. Numa aplicação de controle, duas aquisições não teriam ocorrido.

O p95 em 15.957 µs indica que **mais de 5% das repetições perderam ao menos um período**. Comportamento recorrente, não evento excepcional.

### A leitura correta, que não é a óbvia

Ocioso, o desvio mediano de 0,4 µs sobre período de 10 ms é erro relativo de 0,004% — desempenho que a maioria das aplicações didáticas consideraria excelente.

**A propriedade ausente não é a precisão, é a garantia.** O escalonador de propósito geral não oferece compromisso quanto ao pior caso sob concorrência, e é esse compromisso que um RTOS, ou a ausência de sistema operacional, fornece.

Formulação para a dissertação: o domínio Linux ensina bem a estruturar aplicações e mal a raciocinar sobre prazos. A diferença entre as duas condições medidas é o argumento mais eloquente disponível para demonstrar isso a um estudante.

### Beco sem saída — primeira tentativa inválida

A primeira execução do E3 mediu "sob carga" **sem carga**. O `stress-ng` foi iniciado manualmente em outro terminal e completou os 60 segundos **antes** de o operador pressionar Enter. Resultado: mediana idêntica nas duas condições (0,4 µs) e pior caso até menor sob suposta carga, de 1,3 para 0,9 µs.

Dois erros de desenho, ambos corrigidos:

- **Sincronia manual entre dois terminais.** Resolvido disparando a carga pelo próprio script, com `subprocess.Popen`.
- **`--cpu 4` em máquina de quatro núcleos** não cria contenção suficiente; o escalonador ainda aloca um núcleo quase inteiro ao processo medido. Resolvido com `--cpu 16`.

**O sinal que denunciou o erro:** não existe cenário em que carga de CPU *reduza* jitter. Resultado fisicamente implausível é o melhor detector de erro experimental, e vale registrar o episódio como tal.

---

## Padrão transversal: três ocorrências do mesmo fenômeno

| Eixo | Parâmetro | Efeito |
|---|---|---|
| 4.4 (E4) | `EIDSP_QUANTIZE_FILTERBANK` | **inerte** — nenhuma diferença em latência, heap ou binário |
| 4.1 (E1) | clock SPI, 1 → 10 MHz | **dominante** — 68% do valor original |
| 4.2 (E2) | clock I²C, 100 → 400 kHz | **dominante** — 97% do valor original |

Em três de três casos, um parâmetro arbitrado sem justificativa precisou ser variado deliberadamente para que se soubesse o que ele valia. A advertência metodológica já está no Capítulo 4; esta terceira ocorrência a consolida.

---

## Incorporado ao TCC

Em `EA019_TCC_Timoteo_Altoe_v4_E2E3pi.docx`, com alterações rastreadas:

**4.2.3** — três parágrafos: a decomposição entre barramento e custo fixo, as duas consequências (protocolo como gargalo, custo fixo do I²C contra o do SPI), e o atrito silencioso do `config.txt`.

**4.3.3** — três parágrafos: as duas condições medidas, a perda de dois períodos como número decisivo, e a leitura de que falta garantia e não precisão.

---

## Estado dos experimentos

| | BitDogLab | UNO Q | Raspberry Pi 4 |
|---|---|---|---|
| E1 | ✔ 2.702 ns | ✔ 150.010 ns | ✔ 7.357 ns |
| E2 | pendente | pendente | ✔ 242.093 ns a 400 kHz |
| E3 | pendente | pendente | ✔ 0,4 µs / 6.107 µs |
| E4 | ✔ | não realizado | ✔ |

---

## Próxima sessão — E2 e E3 nos alvos microcontrolados

**BitDogLab, E2.** O MPU-6050 entra no conector de 4 pinos rotulado "I2C 0", lado direito: GP0 como SDA, GP1 como SCL, mais 3V3 e GND no mesmo conector. Sem protoboard. **Não use o conector "I2C 1"** (GP2/GP3): ele compartilha o bloco i2c1 com o display de bordo.

Endereço 0x68, `WHO_AM_I` em 0x75 retornando 0x68, acordar o chip escrevendo 0x00 em 0x6B, ler seis bytes a partir de 0x3B. **Fixe o clock do barramento em 400 kHz no código** para que a comparação com a Pi seja direta.

**BitDogLab, E3.** É o alvo mais rico deste eixo: dá para demonstrar as três camadas de concorrência num programa só — tarefa periódica em software no core0, animação da matriz WS2812B acionada por PIO, e segundo núcleo por `multicore_launch_core1()`. A carga concorrente substitui o `stress-ng`.

**UNO Q.** E2 pelo `Wire` do núcleo Arduino; E3 conforme o RTOS que roda sob o núcleo — item ainda pendente na lista de dados a confirmar.

---

> Rascunho de bancada. Medições próprias, sem tratamento estatístico, coerentes com o princípio do Capítulo 4.
