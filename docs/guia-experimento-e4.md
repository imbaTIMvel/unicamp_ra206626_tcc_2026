# E4 — Guia de bancada

**Experimento:** pipeline de keyword spotting em 2–3 alvos
**Alvos:** BitDogLab v6.3 (Pico W) · Raspberry Pi 4 Model B · UNO Q (quando chegar)
**Ambiente:** Windows 11 + WSL2 (Ubuntu) · Pico SDK manual em `~/pico-sdk`

Documento único. Substitui `E4_guia_de_bancada_dia1.md` e `E4_partida_windows11_wsl_rpi4.md`, e incorpora as correções descobertas na sessão 1.

---

## Progresso

- [x] **Bloco 0** — cartão SD gravado, Raspberry Pi 4 ligada
- [x] **Bloco 1** — WSL e toolchain (cronometrado)
- [x] **Bloco 2** — prova de vida na BitDogLab
- [ ] **Bloco 3** — dataset
- [ ] **Bloco 4** — treino no Edge Impulse
- [ ] **Bloco 5** — buffer fixo
- [ ] **Bloco 6** — implantar e medir na BitDogLab
- [ ] **Bloco 7** — Raspberry Pi 4
- [ ] **Bloco 8** — float32 contra int8 *(bônus de alto valor)*
- [ ] **Bloco 9** — microfone ao vivo *(demonstração)*

---

## Critério de sucesso

Ao final do experimento você precisa ter:

1. Um modelo int8 treinado e exportado.
2. O mesmo modelo rodando na BitDogLab, imprimindo latência e classe no terminal serial.
3. Cinco números anotados por alvo: tamanho do modelo, memória de execução, latência de inferência (mediana de 10), tempo até a primeira inferência e contagem de passos de configuração.

O alvo Raspberry Pi é desejável, não obrigatório. **Se tiver que escolher, a BitDogLab vem primeiro** — é o alvo de maior risco técnico e o que sustenta o argumento central do eixo 4.4.

Se o experimento travar num ponto, o ponto em que travou já é resultado.

---

## O ambiente: três decisões fixadas

**1. O SDK da extensão VS Code não é usado — mas é dado.**

A pasta `C:\Users\timoteo.silva\.pico-sdk` é onde a extensão "Raspberry Pi Pico" instala SDK, toolchain, CMake, Ninja e picotool. É o caminho de um clique. O trabalho usa a instalação manual no WSL, e com isso você tem os dois números na mesma máquina:

| Caminho | O que medir | Como |
|---|---|---|
| Assistido (extensão VS Code) | passos de configuração | reconstituído — conte os cliques, não cronometre |
| Manual (WSL) | tempo e passos | medido |

Registre na planilha que o tempo do caminho assistido é reconstituído, não medido.

**2. A toolchain do Windows não serve para o WSL.** Os binários em `.pico-sdk` são executáveis Windows. Não aponte `PICO_SDK_PATH` para `/mnt/c/Users/...`.

**3. Build no WSL, gravação e monitor pelo Windows.** O WSL2 não enxerga USB nem porta serial sem `usbipd-win`, que não vale a dependência.

```
WSL (Ubuntu)                    Windows 11
─────────────                   ──────────
compila o .uf2       ──────►    volume RPI-RP2 (copiar o arquivo)
                                terminal serial em COMx (PuTTY)
```

> **Regra de ouro do WSL:** o projeto mora em `~/tcc`, no sistema de arquivos do Linux. **Nunca em `/mnt/c/`.** Compilar através da camada de tradução leva minutos onde levaria segundos.

---

## Cronômetro e contagem de passos

Duas métricas se perdem se não forem registradas na hora.

**Tempo até a primeira inferência:** começa ao abrir o primeiro terminal para instalar a toolchain daquele alvo; termina na primeira saída correta. **Pausas e leitura de documentação contam** — é isso que o critério mede.

**Passos de configuração:** marque um risco no papel a cada um. Contar depois, de memória, não funciona.

Convenção fixada na sessão 1, a aplicar igual nos três alvos:

1. **Ação física de bancada conta** — pôr em BOOTSEL conta, plugar o cartão na Pi conta.
2. **Tentativa que falhou não conta como passo** — vai para a lista de becos sem saída, que é material qualitativo do critério 3.3.3.
3. **Instrumentação do experimento não conta** — `versoes.txt` e `git commit` existem por causa do TCC, não da plataforma.
4. **Baixar dataset público não conta** — é custo do problema, idêntico nos três alvos. Já o fluxo da ferramenta de treino (Bloco 4) conta inteiro, e o mesmo critério vale depois para o Qualcomm AI Hub.

Anote também os **becos sem saída**: cada tentativa que falhou e por quê. Não entram na contagem, mas rendem os melhores parágrafos do Capítulo 5.

---

## Bloco 0 — Cartão SD e primeiro boot da Pi

A Raspberry Pi é o caminho crítico: espera longa e ociosa. Ponha para rodar antes de tocar na BitDogLab.

1. Instale o **Raspberry Pi Imager** no Windows.

2. Escolha **Raspberry Pi OS Lite (64-bit)**, sob "Raspberry Pi OS (other)". Duas decisões que importam:
   - **64-bit é obrigatório.** O deployment do Edge Impulse é AARCH64; num sistema 32-bit o `.eim` não executa.
   - **Lite, sem desktop.** O ambiente gráfico adiciona carga de fundo que contamina a medição de dispersão de latência do Capítulo 4.

3. Na engrenagem de configuração: nome do host, usuário e senha, Wi-Fi com país correto, **SSH habilitado**.

4. Grave, insira o cartão, ligue a placa e deixe-a quieta até o Bloco 7.

O Imager tem três campos e eles confundem: **Dispositivo** só filtra a lista de sistemas; **Armazenamento** é o leitor de cartão, não a placa. A Raspberry Pi nunca aparece ali.

**Anote:** variante de RAM da Pi 4 (serigrafada perto do USB) e marca, modelo e classe do cartão (A1/A2). Cartão ruim vira outlier de latência que você atribuiria ao Linux.

Um cartão de 32 GB é folgado: o Lite ocupa ~2,5 GB e o `.eim` fica nas dezenas de MB.

---

## Bloco 1 — WSL e toolchain ⏱️

```powershell
wsl -l -v
# se não houver Ubuntu:
wsl --install -d Ubuntu
```

**Inicie o cronômetro.** Dentro do WSL:

```bash
sudo apt update
sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
                    build-essential git python3 unzip
```

> **Correção:** a documentação do Pico SDK lista `libstdc++-arm-none-eabi-newlib`, que **não existe mais** nos repositórios recentes do Ubuntu. O conteúdo foi absorvido pelo `libnewlib-arm-none-eabi`. Se incluí-lo, o `apt` aborta a linha inteira. Isto é achado do critério 3.3.3 — registre.

```bash
cd ~
git clone -b master --recurse-submodules https://github.com/raspberrypi/pico-sdk.git
echo 'export PICO_SDK_PATH=$HOME/pico-sdk' >> ~/.bashrc
source ~/.bashrc

mkdir -p ~/tcc/e4/{dataset,modelo,picow,rpi,logs}
cd ~/tcc/e4 && git init
```

Confirme a toolchain e registre versões:

```bash
arm-none-eabi-gcc --version
{ git -C ~/pico-sdk rev-parse --short HEAD
  arm-none-eabi-gcc --version | head -1
  cmake --version | head -1
  lsb_release -d ; } >> ~/tcc/e4/logs/versoes.txt
```

**O cronômetro só para no fim do Bloco 2.**

---

## Bloco 2 — Prova de vida na BitDogLab

**Não pule.** Este bloco separa "meu modelo não funciona" de "meu caminho de gravação não funciona" — duas falhas indistinguíveis quando você tenta as duas ao mesmo tempo.

### 2.1 Criar os arquivos

Cole o bloco inteiro no WSL. As aspas em `<< 'EOF'` impedem o shell de interpretar o conteúdo:

```bash
mkdir -p ~/tcc/e4/picow/vida && cd ~/tcc/e4/picow/vida

cat > vida.c << 'EOF'
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_R 13   // LED RGB da BitDogLab, canal vermelho

int main(void) {
    stdio_init_all();
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    int n = 0;
    while (true) {
        gpio_put(LED_R, 1); sleep_ms(200);
        gpio_put(LED_R, 0); sleep_ms(800);
        printf("vivo %d\n", n++);
    }
}
EOF

cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(vida C CXX ASM)
pico_sdk_init()
add_executable(vida vida.c)
target_link_libraries(vida pico_stdlib)
pico_enable_stdio_usb(vida 1)
pico_enable_stdio_uart(vida 0)
pico_add_extra_outputs(vida)
EOF

cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
ls -l
```

> Note o `cp`, não `cd`. Sem o `pico_sdk_import.cmake` no projeto, o CMake não conhece `pico_sdk_init` e cai no compilador nativo `/usr/bin/cc`.

### 2.2 Compilar

```bash
mkdir build && cd build
cmake .. -DPICO_BOARD=pico_w
make -j$(nproc)
```

O `cmake` deve identificar `arm-none-eabi-gcc`. Se aparecer `/usr/bin/cc`, há cache sujo: **apague a pasta**, não basta rodar de novo.

```bash
cd .. && rm -rf build && mkdir build && cd build
cmake .. -DPICO_BOARD=pico_w && make -j$(nproc)
```

### 2.3 Gravar

1. Segure **BOOTSEL** no módulo Pico W.
2. Pressione e solte o **RESET** da BitDogLab.
3. Solte o BOOTSEL. O volume `RPI-RP2` aparece no Windows com uma letra — digamos `D:`.

```bash
ls /mnt              # /mnt/d está na lista?
cp vida.uf2 /mnt/d/
```

Se `/mnt/d` não existir, o WSL não montou o removível:

```bash
sudo mkdir -p /mnt/d && sudo mount -t drvfs D: /mnt/d
cp vida.uf2 /mnt/d/
```

A placa reinicia e o volume some — comportamento esperado. Antes da próxima gravação: `sudo umount /mnt/d`.

> O caminho `/media/$USER/RPI-RP2/` da documentação Linux **não existe no WSL**.

### 2.4 Ler o serial (no Windows)

O `minicom` não serve: o WSL2 não expõe porta COM.

1. Gerenciador de Dispositivos → **Portas (COM e LPT)** → anote o `COM<n>`.
2. PuTTY, categoria **Session**: Connection type **Serial**, Serial line `COM<n>`, Speed `115200`.
3. **Connection → Serial:** 8 data bits, 1 stop bit, parity None, **Flow control None**. O padrão XON/XOFF pode travar a saída no meio de uma rajada de `printf`.
4. **Session → Logging:** marque *All session output* e aponte para `C:\Users\timoteo.silva\tcc\logs\serial_&Y&M&D_&T.log`. Os `&` viram data e hora. **Ative isto** — no Bloco 6 você coleta dez latências e não quer transcrever à mão.
5. Volte a **Session**, salve como `BitDogLab`.

Se as linhas saírem em escada, **Terminal → Implicit CR in every LF**.

A janela fecha sozinha quando a placa entra em BOOTSEL, porque a porta some. Reabra depois de gravar.

**Você deve ver** o LED vermelho piscando e `vivo 0, vivo 1, ...`.

**Pare o cronômetro.** Esse número é, ao mesmo tempo, o "tempo até o primeiro programa funcional" do E1 e a linha de base do E4.

---

## Bloco 3 — Dataset

Não cronometre. Palavras em inglês simplificam hoje; gravar em português é refinamento posterior.

```bash
cd ~/tcc/e4/dataset
wget http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz
mkdir sc && tar -xzf speech_commands_v0.02.tar.gz -C sc
```

São 2,3 GB. Enquanto baixa, crie a conta no Edge Impulse e um projeto do tipo **Audio**.

Escolha duas palavras foneticamente distantes — `yes` e `no` funcionam; `go` e `no` não, e você perde tempo com matriz de confusão ruim.

Quatro classes, ~300 arquivos cada. Mais do que isso só aumenta o tempo de treino sem mudar conclusão nenhuma:

```bash
cd ~/tcc/e4/dataset
mkdir -p envio/{yes,no,ruido,desconhecido}
ls sc/yes/*.wav | shuf -n 300 | xargs -I{} cp {} envio/yes/
ls sc/no/*.wav  | shuf -n 300 | xargs -I{} cp {} envio/no/
ls sc/_background_noise_/*.wav | xargs -I{} cp {} envio/ruido/
for p in up down left right stop on off; do
  ls sc/$p/*.wav | shuf -n 45 | xargs -I{} cp {} envio/desconhecido/
done
```

Os arquivos de `_background_noise_` têm um minuto cada; o Edge Impulse fatia na ingestão. Se reclamar, use *Split sample* na interface.

---

## Bloco 4 — Treinar no Edge Impulse

**Conte os passos.** Este bloco é comparável, depois, ao fluxo do Qualcomm AI Hub.

1. **Upload:** use a interface web, *Data acquisition → Upload data*, que aceita seleção múltipla e definição de rótulo no envio. O `edge-impulse-cli` exige Node 20+ e o `apt` do Ubuntu entrega versão mais velha — se quiser mesmo o CLI, instale o Node pelo NodeSource.

2. **Impulse design:** janela 1000 ms, stride 500 ms, 16 kHz. Processamento **Audio (MFE)**. Aprendizado **Classification**.

3. **Generate features.** Olhe o gráfico de separação. Classes embaralhadas aqui = problema de dados, não de modelo.

4. **Treinar.** 50 épocas, learning rate padrão. **Acurácia acima de 85% basta** — você mede o pipeline, não publica o modelo.

5. **Model testing** no conjunto separado. **Anote a acurácia**: é o controle de sanidade entre alvos.

6. **Deployment → C++ library**, quantização **int8**, EON Compiler ativo se aparecer. Baixe o `.zip`.

7. **Antes de sair da tela de deployment, anote "peak RAM usage" e "flash usage".** São dois dos seus cinco números, e a tela some.

8. Em *Dashboard*, baixe também o `.tflite` int8 — o tamanho desse arquivo é a métrica "tamanho do modelo".

9. Baixe ainda o deployment **Linux (AARCH64)** para o Bloco 7.

---

## Bloco 5 — Buffer fixo

Escolha **um** arquivo do conjunto de teste que o modelo classifica corretamente (a tela de *Model testing* mostra por amostra).

```bash
cd ~/tcc/e4/modelo
cp ~/tcc/e4/dataset/sc/yes/<arquivo>.wav palavra.wav
```

`wav_para_c.py`:

```python
import wave, struct

with wave.open('palavra.wav', 'rb') as w:
    assert w.getframerate() == 16000 and w.getnchannels() == 1
    n = w.getnframes()
    amostras = struct.unpack(f'<{n}h', w.readframes(n))

with open('audio_fixo.h', 'w') as f:
    f.write(f'#define AUDIO_N {len(amostras)}\n')
    f.write('static const int16_t audio_fixo[AUDIO_N] = {\n')
    for i in range(0, len(amostras), 12):
        f.write('  ' + ', '.join(str(v) for v in amostras[i:i+12]) + ',\n')
    f.write('};\n')

with open('audio_fixo.bin', 'wb') as f:      # versão para o Raspberry Pi
    f.write(struct.pack(f'<{len(amostras)}h', *amostras))
```

**O mesmo áudio em todos os alvos, sem exceção.** É a entrada idêntica que torna as latências comparáveis; se divergir, o experimento perde o sentido. Guarde o `.wav` no repositório junto com o `.h` e o `.bin`.

---

## Bloco 6 — Implantar na BitDogLab e medir ⏱️

O bloco mais arriscado. Reserve tempo mental.

```bash
cd ~/tcc/e4/picow
unzip /mnt/c/Users/timoteo.silva/Downloads/ei-*-cpp-*.zip -d e4_kws
cd e4_kws
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
cp ../../modelo/audio_fixo.h .
```

Ler o zip de `/mnt/c` tudo bem; **compilar lá, não**.

`main.cpp`:

```c
#include "pico/stdlib.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "audio_fixo.h"

static int fornecer_dados(size_t offset, size_t length, float *out) {
    for (size_t i = 0; i < length; i++) out[i] = (float)audio_fixo[offset + i];
    return 0;
}

int main(void) {
    stdio_init_all();
    sleep_ms(3000);                       // tempo de abrir o PuTTY

    signal_t sinal = { .total_length = AUDIO_N, .get_data = &fornecer_dados };
    ei_impulse_result_t r = { 0 };

    while (true) {
        uint32_t t0 = time_us_32();
        run_classifier(&sinal, &r, false);
        uint32_t t1 = time_us_32();

        printf("latencia_total_us=%u dsp_ms=%d classif_ms=%d\n",
               t1 - t0, r.timing.dsp, r.timing.classification);
        for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
            printf("  %s: %.3f\n", ei_classifier_inferencing_categories[i],
                   r.classification[i].value);
        sleep_ms(1000);
    }
}
```

`CMakeLists.txt` — ajuste os globs à estrutura real do zip:

```cmake
cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(e4_kws C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

file(GLOB_RECURSE EI_SRC
  "edge-impulse-sdk/dsp/kissfft/*.cpp"
  "edge-impulse-sdk/dsp/dct/*.cpp"
  "edge-impulse-sdk/tensorflow/lite/*.c"
  "edge-impulse-sdk/tensorflow/lite/**/*.cc"
  "edge-impulse-sdk/porting/posix/*.c*"
  "tflite-model/*.cpp"
)

add_executable(e4_kws main.cpp ${EI_SRC})
target_include_directories(e4_kws PRIVATE . edge-impulse-sdk model-parameters tflite-model)
target_compile_definitions(e4_kws PRIVATE
  EI_CLASSIFIER_USE_FULL_TFLITE=0
  EIDSP_USE_CMSIS_DSP=0        # Cortex-M0+ nao tem as extensoes DSP
  EIDSP_QUANTIZE_FILTERBANK=0
)
target_link_libraries(e4_kws pico_stdlib)
pico_enable_stdio_usb(e4_kws 1)
pico_enable_stdio_uart(e4_kws 0)
pico_add_extra_outputs(e4_kws)
```

> **`EIDSP_USE_CMSIS_DSP=0` não é opcional.** O Cortex-M0+ do RP2040 não tem as extensões DSP que a CMSIS-DSP assume; sem o define, a compilação quebra ou o binário trava. **E isto não é acidente de configuração: é a mesma ausência de hardware que obriga a quantização int8**, evidência central do eixo 4.4. Anote o episódio — vale um parágrafo melhor que qualquer número.

Compile e grave como no Bloco 2. Com o log do PuTTY ativo, deixe rodar dez inferências.

**Medir:**

- Dez inferências sobre o mesmo buffer; mediana depois, a partir do arquivo de log. Não transcreva número à mão.
- O EI reporta `timing.dsp` e `timing.classification` separados — **guarde os dois**. A divisão entre pré-processamento e inferência é informação boa e ninguém publica.
- Tensor arena: no log de build ou em `model-parameters/model_metadata.h`.
- Confira se a classe predita bate com o rótulo do `.wav`. Se não bater, o problema está na conversão do buffer, não no modelo.

---

## Bloco 7 — Raspberry Pi 4 Model B ⏱️

### 7.1 Conectar

```powershell
ssh <usuario>@<hostname>.local
```

Se o `.local` não resolver, pegue o IP na lista de clientes DHCP do roteador.

### 7.2 Três verificações antes de medir

A parte que as pessoas pulam e depois publicam número errado.

```bash
uname -m              # precisa retornar aarch64
vcgencmd get_throttled
vcgencmd measure_temp
```

- `uname -m` diferente de `aarch64` → imagem 32-bit. Regrave; o `.eim` não roda.
- `get_throttled` diferente de `throttled=0x0` → subtensão ou superaquecimento. **Latência medida sob throttling é lixo.** A Pi 4 exige fonte USB-C de 5 V / 3 A de verdade.
- Sem dissipador, a Pi 4 reduz clock a 80 °C sob inferência contínua. Rode `get_throttled` **de novo depois** das dez inferências. Se mudou no meio, descarte e refaça com refrigeração.

### 7.3 Preparar

```bash
sudo apt update && sudo apt full-upgrade -y
sudo reboot
# depois do boot
pip3 install edge_impulse_linux --break-system-packages

{ cat /etc/os-release | head -2
  uname -a
  vcgencmd measure_clock arm ; } >> ~/versoes_rpi.txt
```

### 7.4 Levar modelo e áudio

```bash
scp /mnt/c/Users/timoteo.silva/Downloads/modelo.eim <usuario>@<host>.local:~/
scp ~/tcc/e4/modelo/audio_fixo.bin <usuario>@<host>.local:~/
```

Na Pi: `chmod +x modelo.eim`.

### 7.5 Medir

> **Por que não `tflite-runtime` puro:** você rodaria o mesmo modelo, mas teria que reimplementar o MFE para alimentar a entrada, e meio decibel de diferença no banco de filtros invalida a comparação de acurácia entre alvos. O `.eim` empacota o mesmo DSP e o mesmo modelo.

```python
import time, numpy as np
from edge_impulse_linux.runner import ImpulseRunner

runner = ImpulseRunner('./modelo.eim')
info = runner.init()
print(info['project']['name'])

features = np.fromfile('audio_fixo.bin', dtype=np.int16).astype(float).tolist()

ts = []
for _ in range(10):
    t0 = time.perf_counter_ns()
    r = runner.classify(features)
    ts.append((time.perf_counter_ns() - t0) / 1000)

print('mediana_us', np.median(ts), 'pior_us', max(ts))
print(r['result']['classification'])
print('dsp_ms', r['timing']['dsp'], 'classif_ms', r['timing']['classification'])
runner.stop()
```

**Reporte mediana e pior caso.** Sob Linux sem `PREEMPT_RT` devem aparecer outliers ausentes no alvo MCU — evidência direta para a discussão de determinismo do Capítulo 4.

> A Pi 4 é mais modesta que a Pi 5 citada no Capítulo 3. Registre modelo exato e variante de RAM, e deixe a diferença de geração explícita antes que vire erro de interpretação.

---

## Bloco 8 — float32 contra int8 *(bônus de alto valor)*

Exporte o **mesmo** modelo em float32, compile para a BitDogLab e meça.

O Cortex-M0+ emula ponto flutuante em software; a diferença deve ser grande. É a evidência mais direta possível para a afirmação central do eixo 4.4, ao custo de uma recompilação.

**Se tiver que escolher entre este bloco e o alvo Raspberry Pi, faça este.** Ele responde a uma pergunta que a documentação não responde; o Raspberry Pi responde uma que ela quase responde.

---

## Bloco 9 — Microfone ao vivo *(demonstração, não medição)*

Só depois de tudo acima. Troque o buffer fixo por amostragem do GP28 a 16 kHz:

```c
adc_gpio_init(28);
adc_select_input(2);              // GP28 = ADC2 = microfone
adc_set_clkdiv(48000000.0f / 16000.0f - 1.0f);   // 16 kHz
adc_fifo_setup(true, true, 1, false, false);     // FIFO + pedido de DMA
adc_run(true);
```

**Subtraia o offset.** O microfone da BitDogLab repousa em 1,65 V — cerca de 2048 em 12 bits — e o extrator de MFE espera áudio centrado em zero:

```c
int16_t audio = (int16_t)(((int32_t)amostra - 2048) << 4);
```

Meça o repouso real da sua placa e use no lugar de 2048. **Se funcionar com buffer fixo e falhar ao vivo, suspeite deste offset antes do modelo.**

Sinalize a classe na matriz WS2812B (GP7) ou no OLED (I2C1, GP14/GP15).

Ressalva a registrar: microfone analógico amostrado por ADC não equivale a microfone digital em barramento dedicado. Não é medida de acurácia comparável entre plataformas — é verificação de que o pipeline funciona fora de condições controladas.

---

## Planilha de coleta

```
experimento | plataforma | alvo | metrica              | valor | unidade | execucao | observacao
E4          | BitDogLab  | MCU  | modelo_tamanho       |       | kB      | 1        | int8
E4          | BitDogLab  | MCU  | tensor_arena         |       | kB      | 1        |
E4          | BitDogLab  | MCU  | latencia_dsp         |       | ms      | 1..10    | mediana
E4          | BitDogLab  | MCU  | latencia_inferencia  |       | ms      | 1..10    | mediana
E4          | BitDogLab  | MCU  | tempo_1a_inferencia  |       | min     | 1        | do zero
E4          | BitDogLab  | MCU  | passos_config        |       | passos  | 1        |
E4          | BitDogLab  | MCU  | acuracia             |       | %       | 1        | conj. de teste
E4          | RPi 4      | CPU  | ...
```

Guarde junto: `logs/versoes.txt`, `versoes_rpi.txt`, o histórico do shell, os logs do PuTTY, a versão da placa (BitDogLab v6.3), o commit do pico-sdk e a versão da biblioteca do Edge Impulse.

---

## Quando der errado

| Sintoma | Causa |
|---|---|
| `unable to locate package libstdc++-arm-none-eabi-newlib` | Pacote extinto; remova da linha do `apt` |
| Build lentíssimo no WSL | Projeto em `/mnt/c/`. Mova para `~` |
| CMake usa `/usr/bin/cc` | Falta `pico_sdk_import.cmake`, ou cache sujo — `rm -rf build` |
| `Unknown CMake command "pico_sdk_init"` | Mesma causa acima |
| `PICO_SDK_PATH` vazio | `source ~/.bashrc` só vale no terminal em que rodou; reexporte |
| `/media/$USER/RPI-RP2/` não existe | Caminho Linux nativo; no WSL é `/mnt/<letra>` |
| `/mnt/d` não existe | Removível não automontado: `sudo mount -t drvfs D: /mnt/d` |
| `RPI-RP2` não aparece | Cabo só de carga; ou BOOTSEL solto antes do RESET |
| `minicom` não encontrado | Não instale — WSL2 não expõe COM. Use PuTTY no Windows |
| Nenhuma COM no Gerenciador | Cabo só de carga, ou `pico_enable_stdio_usb` em 0 |
| Saída serial em escada | PuTTY → Terminal → *Implicit CR in every LF* |
| Serial trava no meio da rajada | Flow control em XON/XOFF; mude para None |
| Primeiras linhas se perdem | Normal — o `sleep_ms(3000)` existe para isso |
| Compilação quebra em `arm_math.h` | Falta `EIDSP_USE_CMSIS_DSP=0` |
| Trava após `run_classifier` | Tensor arena não cabe na RAM; confira o "peak RAM" do EI |
| Classe errada com buffer fixo | Conversão do WAV: taxa 16 kHz, 1 canal, endianness |
| Funciona fixo, falha ao vivo | Offset de 1,65 V não subtraído |
| Latência absurdamente alta | Exportou float32 sem querer — o que é, ironicamente, o Bloco 8 |
| `.eim` não executa na Pi | Sistema 32-bit; `uname -m` tem que dizer `aarch64` |
| Dispersão absurda na Pi | Throttling; cheque `get_throttled` antes e depois |
| Dataset não sobe pelo CLI | Node antigo; use a interface web |

---

## Ponto de parada seguro

Antes de fechar o notebook, dez minutos que evitam uma manhã perdida:

1. `git add -A && git commit` de tudo, inclusive `.uf2` e `.eim`.
2. Transcreva os números para a planilha e para o Capítulo 5 **agora**, enquanto o contexto está fresco.
3. `history > ~/tcc/e4/logs/historico_sessao<N>.txt` — é a fonte primária da contagem de passos.
4. Três linhas sobre onde parou e qual era a próxima dúvida. Você não vai lembrar.
