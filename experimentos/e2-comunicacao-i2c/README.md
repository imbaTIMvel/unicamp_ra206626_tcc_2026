# E2 — Comunicação por barramento I²C

Leitura dos três eixos de um acelerômetro em uma única transação de seis bytes, nas três plataformas. O experimento verifica a previsão de que, neste eixo, o desempenho é determinado pelo protocolo e não pela plataforma.

## Resultados

Todos a 400 kHz, transação em bloco de seis bytes a partir do registrador 0x3B.

| Alvo | Latência mediana | Faixa |
|---|---|---|
| Arduino UNO Q (`Wire`) | **210 413 ns** | 397 ns |
| BitDogLab (Pico SDK) | **241 651 ns** | 25 ns (0,01%) |
| Raspberry Pi 4 (`smbus2`) | **242 093 ns** | 936 ns |

Na Raspberry Pi, a 100 kHz — frequência padrão do adaptador — a mediana foi de 886.677 ns.

**Os três cabem em uma faixa de 13%**, e os dois últimos diferem entre si em 0,18%. Um Cortex-M0+ de 133 MHz sem sistema operacional e um Cortex-A72 de 1,8 GHz com driver de núcleo e interpretador produzem praticamente o mesmo número.

**O contraste com o E1 é o achado.** A mesma biblioteca de alto nível que tornou a UNO Q 55 vezes mais lenta no acesso direto a periférico não impõe penalidade alguma aqui. A abstração não tem custo fixo: tem custo relativo à granularidade da operação abstraída.

As duas frequências medidas na Raspberry Pi permitem decompor a latência. Resolvendo o sistema, a transação equivale a aproximadamente 86 bits e o custo independente da frequência a cerca de 27 µs — parcela que, pelos resultados dos alvos microcontrolados, corresponde a tempo real de barramento não capturado pela contagem simples de bits, e não a custo de software.

## Montagem

Acelerômetro de seis eixos, endereço 0x68.

| Alvo | SDA | SCL |
|---|---|---|
| BitDogLab | conector "I2C 0", GP0 | GP1 |
| UNO Q | pino SDA da barra superior | SCL |
| Raspberry Pi 4 | pino 3 (GPIO 2) | pino 5 (GPIO 3) |

Alimentação em **3,3 V** em todos os casos.

Na BitDogLab, **não use o conector rotulado "I2C 1"** (GP2/GP3): ele compartilha o bloco de hardware i2c1 com o display de bordo.

## Identificação dos componentes

Os módulos foram adquiridos como MPU-6500, mas a leitura do registrador `WHO_AM_I` (0x75) revelou peças distintas:

| Alvo | Retorno | Peça |
|---|---|---|
| BitDogLab | `0x70` | MPU-6500 |
| UNO Q | `0x68` | MPU-6050 |
| Raspberry Pi 4 | `0x68` | MPU-6050 |

Para a medição não há consequência: o mapa de registradores relevante é idêntico e a transação é a mesma. **Os componentes foram identificados por leitura de registrador, não pelo rótulo comercial.**

## Arquivos

```
bitdoglab/e2_sensores.c       acesso por i2c_write_blocking / i2c_read_blocking
raspberry-pi/e2_teste.py      teste de vida
raspberry-pi/e2_medir.py      medição
uno-q/e2_sensores/            aplicação exportada do Arduino App Lab
```

## Como reproduzir

**BitDogLab.** Mesmo procedimento do E1. O barramento é fixado em 400 kHz no próprio código, por `i2c_init(i2c0, 400 * 1000)`.

**UNO Q.** `Wire.setClock(400000)` no `setup()`. Sem isso o barramento fica em 100 kHz e a comparação não vale.

**Raspberry Pi 4.** A frequência **não é configurável em tempo de execução**: depende de parâmetro lido pelo firmware na inicialização. Edite `/boot/firmware/config.txt`, garantindo que a linha esteja **sem o caractere de comentário**:

```
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

Reinicie e confirme antes de medir:

```bash
sudo cat /sys/kernel/debug/clk/clk_summary | grep i2c_div    # deve mostrar 400000
i2cdetect -y 1                                                # deve mostrar 68
```

## Ressalvas

A primeira tentativa de elevar a frequência falhou silenciosamente: a linha estava correta no arquivo, mas precedida do caractere de comentário que a distribuição traz por padrão. Nenhuma mensagem de erro foi emitida, e a ausência de efeito só apareceu na inspeção direta do registrador de divisão de relógio.

A leitura canônica de registrador I²C emprega condição de reinício entre a escrita do endereço e a leitura dos dados. O acesso de baixo nível do Pico SDK a implementa por parâmetro explícito e funcionou de primeira; a biblioteca de alto nível do núcleo Arduino, embora exponha parâmetro aparentemente equivalente, retornou indicação de ausência de dados em todas as leituras. A versão publicada usa condição de parada convencional.

A vantagem de 13% da UNO Q é compatível com a diferença de frequência entre os núcleos e não deve ser lida como mérito de sua camada de abstração.
