# Registro de passos — E1, Sessão 1

**Data:** segunda-feira, 20/10/2026
**Alvos cobertos:** BitDogLab v6.3 — **concluído** · Arduino UNO Q (domínio MCU) — **concluído**
**Pendente:** Raspberry Pi 4 com ADC externo AD5592R

---

## Teste preliminar: a entrada analógica da BitDogLab

Montado o divisor LDR + 10 kΩ sobre o GP28, a tensão medida **diretamente no pino do microcontrolador** permaneceu fixa em ~1,65 V, sem acompanhar a variação de iluminação. A medida no nó isolado da protoboard variava normalmente entre 1,28 V e 3,01 V, o que a princípio sugeriu falta de contato; a medição no pino desfez a dúvida.

**Veredito:** a saída do amplificador do microfone, de baixa impedância, domina o nó. Um divisor externo de 10 kΩ não tem como impor tensão ali.

**Consequência:** a variante externa do E1 é inviável na BitDogLab. Fonte analógica adotada: eixo do joystick (GP26, ADC0).

Isso fecha a pendência aberta desde a leitura do BIH e é **resultado, não obstáculo**: integrar periféricos consome pinos, e os três canais analógicos do RP2040 estão ocupados por joystick e microfone.

---

## Passos — BitDogLab

| # | Ação |
|---|---|
| 1 | Criar `~/tcc/e1/teste_adc`, copiar o SDK, compilar e gravar o firmware de teste |
| 2 | Acompanhar pelo PuTTY e diagnosticar o GP28 |
| 3 | Criar `~/tcc/e1/picow`, copiar o SDK |
| 4 | Escrever `e1_sensores.c` e o `CMakeLists.txt` |
| 5 | `cmake` e `make` |
| 6 | Gravar o `.uf2` |
| 7 | Abrir a sessão serial |

**Tempo até a primeira leitura correta: 9 min 24,7 s.**

Nota de bancada: o volume de gravação mudou de `D:` para `E:` após a criação de outra partição na máquina. Vale fixar no registro, porque o caminho aparece nos comandos do guia.

---

## Resultados — BitDogLab (Pico W em BitDogLab v6.3)

| Métrica | Valor |
|---|---|
| Linhas de código, versão mínima funcional | 16 com `#include`, 13 sem |
| `CMakeLists.txt` | 9 linhas |
| Tempo de setup | 9 min 24,7 s |
| **Latência, mediana** | **2.702 ns** |
| Mínimo / máximo | 2.697 / 2.713 ns |
| Faixa | 16 ns (0,59% da mediana) |
| Desvio padrão | 2,84 ns |
| Amostras | 100 blocos de 1.000 iterações, em 10 execuções |
| Abstração — leitura ADC | nível 1 (configuração explícita de periférico) |
| Abstração — saída PWM | nível 1 |

**Validação experimental de uma decisão de método.** As dez execuções foram feitas com o joystick em estados muito diferentes — parado em 2000, cravado em 14, cravado em 4079, varrendo toda a faixa. A mediana permaneceu em 2.702 ns em todas. O tempo de conversão é independente do valor convertido, o que sustenta com cem amostras a justificativa de substituir o LDR pelo joystick sem prejuízo à comparabilidade. Era argumento de datasheet; agora é medição.

Observação: a excursão do ADC foi de 14 a 4.079, não 0 a 4.095. Comportamento normal de potenciômetro de joystick, que não alcança os trilhos de alimentação.

---

## Passos — Arduino UNO Q (domínio MCU)

Placa: UNO Q com 4 GB LPDDR4 e 32 GB eMMC.

| # | Ação |
|---|---|
| 1 | Conectar a placa |
| 2 | Baixar o instalador do Arduino App Lab |
| 3 | Executar o instalador |
| 4 | Abrir o App Lab |
| 5 | Montar o circuito na protoboard |
| 6 | Configurar a placa: nome, Wi-Fi, senha do domínio Linux |
| 7 | **Atualização obrigatória de firmware** |
| 8 | Reiniciar o App Lab |
| 9 | Criar o app `e1_sensores` e adicionar o `sketch.ino` |
| 10 | Executar |

**Tempo até a primeira leitura correta: 36 min 27,8 s**, dos quais **15 min 19 s** foram a atualização obrigatória.

---

## Resultados — Arduino UNO Q, núcleo Arduino

| Métrica | Valor |
|---|---|
| Tempo de setup | 36 min 27,8 s (21 min 09 s sem a atualização) |
| **Latência, mediana** | **150.010 ns** |
| Mínimo / máximo | 149.964 / 150.064 ns |
| Faixa | 100 ns (0,067% da mediana) |
| Amostras | 50 blocos de 1.000 iterações |
| Resolução do ADC | **12 bits**, com 2 bits descartados por padrão |
| Abstração — leitura ADC | nível 3 (função de alto nível que oculta o periférico) |
| Abstração — saída PWM | nível 3 |

---

## Comparação entre os dois alvos microcontrolados

| | BitDogLab (M0+ 133 MHz) | UNO Q (Cortex-M33) | Razão |
|---|---|---|---|
| Latência do laço | 2.702 ns | 150.010 ns | **55,5×** |
| Faixa de dispersão | 0,59% | 0,067% | — |
| Tempo de setup | 9 min 25 s | 36 min 28 s | 3,9× |
| Setup sem atualização obrigatória | 9 min 25 s | 21 min 09 s | 2,2× |
| Nível de abstração | 1 | 3 | — |

---

## Três achados

**1. O hardware mais capaz foi 55 vezes mais lento.** O STM32U585 é Cortex-M33 com FPU, arquiteturalmente superior ao Cortex-M0+. A diferença não está no silício: está na camada de biblioteca. O `adc_read()` do Pico SDK consulta um registrador de periférico já configurado; o `analogRead()` do núcleo Arduino reconfigura o conversor a cada chamada, com parâmetros conservadores, para funcionar igual em qualquer pino de qualquer placa do formato.

**Este é o resultado mais eloquente do eixo.** A abstração didática costuma ser avaliada pelo efeito na curva de aprendizagem; aqui ela aparece com preço mensurável em desempenho, numa tarefa elementar. E o custo é invisível ao estudante, que não tem como perceber, sem instrumentar, que uma linha executa dezenas de operações que ele não pediu.

**2. Dois bits escondidos.** Com a configuração padrão, as leituras cobriam faixa de dez bits (453 a 972). Após `analogReadResolution(12)`, passaram a 1.700–4.009. O conversor é de 12 bits e o núcleo descartava dois, presumivelmente por compatibilidade histórica com o formato de oito bits. **Resolve a pendência da seção 7 do roteiro.** O problema não é a perda — uma chamada a recupera — mas o fato de a API não sinalizar que há resolução sendo descartada.

**3. A abstração reduz linhas, não tempo de preparação.** 36 minutos contra 9. Mesmo descontando a atualização obrigatória, continua duas vezes maior. Contraria a expectativa de que a plataforma mais abstrata seja a de partida mais rápida.

---

## Correção pendente no sketch

O sketch utilizado tem um defeito que **não afeta a latência medida**, mas precisa ser corrigido antes de a listagem ir para o Capítulo 5:

```cpp
analogWrite(PIN_LED, map(leitura, 0, 1023, 0, 255));   // errado após 12 bits
```

Com `analogReadResolution(12)`, leituras acima de 1023 produzem valores acima de 255 e o `analogWrite` satura. O trecho de demonstração funcional, portanto, satura o LED acima de um quarto da escala. O laço de latência usa `>> 4`, que está correto, e por isso a medição permanece válida.

Correção: `map(leitura, 0, 4095, 0, 255)` ou, mais direto, `leitura >> 4`.

Vale registrar o episódio como beco sem saída — é consequência direta do achado 2, e ilustra que corrigir a resolução obriga a revisar todo o código que assumia a escala antiga.

---

## Raspberry Pi 4 — bloqueado

### Montagem realizada

Módulo AD5592RBRUZ, ligações para a Pi:

| Módulo | Pino físico da Pi |
|---|---|
| SCK | 23 (GPIO 11) |
| MOSI | 19 (GPIO 10) |
| MISO | 21 (GPIO 9) |
| CS | 24 (GPIO 8) |
| +3V3 | 17 |
| GND | 20 |
| **RST** | **não conectado** |

Divisor na protoboard: +3V3 → LDR → nó em IO0 → 10 kΩ → GND.

### Estado do ambiente

SPI ativo (`/dev/spidev0.0` e `0.1`), governor em `performance`, clock em 1,8 GHz, `throttled=0x0`. Endereço atual: 192.168.2.229.

### O que travou

O módulo não respondeu, e o **LED azul do módulo permaneceu apagado**.

### Hipótese principal para a próxima sessão

**O RST não foi conectado ao 3V3.** O datasheet é explícito: o pino de reset deve ser mantido em nível alto para operação normal, e uma borda de descida reinicia todos os registradores. Com o pino flutuante, qualquer ruído mantém o chip em reset e nenhuma configuração persiste.

É o primeiro item a testar, e custa um jumper.

### Segunda hipótese

Se o LED azul for indicador de alimentação, ele apagado aponta para 3V3 não chegando ao chip — jumper rompido, mau contato ou barra de pinos trocada. Meça com o multímetro entre +3V3 e GND do módulo, com a Pi ligada: tem que dar 3,3 V.

### Pergunta em aberto

O módulo tem duas barras: uma 1x8 (GND, IO7…IO0, +3V3) e uma 2x7 feita para o conector IDC da BitDogLab. **Qual delas foi usada para os sinais SPI?** Sendo a 2x7, convém reconferir a ordem física dos pinos contra o silk, porque o mapeamento assumido pode não corresponder.

---

## Falhas de infraestrutura — não contam como passos

A sessão teve uma cadeia de problemas no alvo Linux, nenhum atribuível à plataforma como objeto de estudo, mas todos registráveis como custo operacional:

| Ocorrência | Resolução |
|---|---|
| Cartão SD não bootava (verde apagado, vermelho aceso) | Regravação do sistema |
| Acesso negado na gravação | Imager executado como administrador |
| IP mudou duas vezes (.10 → .134 → .229) | Localização pela interface do roteador |
| `.134` ocupado por outro dispositivo | Diagnóstico por `arp -a` |
| Chave de host SSH invalidada pela regravação | `ssh-keygen -R` |
| Ambiente perdido na regravação | Reinstalação — **retrabalho, já contado na sessão 6 do E4** |

**O tempo de setup da Raspberry Pi não deve ser cronometrado nesta sessão.** A medição seria contaminada por falhas de bancada e inflaria injustamente a comparação. Use a contagem da sessão 6 do E4, que correu limpa.

**Mas vale um parágrafo no Capítulo 4.** O alvo Linux tem três pontos de falha que o alvo microcontrolado não tem: cartão de memória, alimentação externa e identidade de rede. Nenhum é defeito de projeto, e todos são custo real de operação num contexto didático, em que o estudante não tem como distinguir falha de infraestrutura de erro próprio.

---

## Incorporado ao TCC

Quatro parágrafos em 4.1.1, como alterações rastreadas: a medição dos 55×, a leitura pedagógica do custo invisível da abstração, o achado da resolução e os tempos de setup. A seção 4.1.3 recebeu o resultado do GP28 e a validação da independência entre latência e valor convertido.

---

## Próxima sessão — fechar o E1

1. **Conectar o RST ao 3V3** e repetir o teste de vida
2. LED azul ainda apagado → medir 3,3 V entre +3V3 e GND do módulo
3. Confirmar qual barra de pinos foi usada, conferindo contra o silk
4. Tentar `spi.mode = 2` se o modo 1 não responder
5. Com a leitura acompanhando o LDR, cronometrar **apenas** a escrita do acesso por `spidev` — custo legítimo da plataforma, por não haver ADC interno
6. Medir dez blocos de mil iterações
7. Refazer o governor após cada boot — não sobrevive a reinício

**Ressalva de comparabilidade:** no Pico e na UNO Q o laço era leitura mais escrita de PWM; na Pi, como está, é só leitura. São grandezas diferentes. Considere acrescentar um `PWMLED` do `gpiozero` para fechar o laço equivalente, ou reporte as duas medidas em separado e declare a diferença.

---

> Rascunho de bancada. Medições próprias, sem tratamento estatístico, coerentes com o princípio do Capítulo 4.
