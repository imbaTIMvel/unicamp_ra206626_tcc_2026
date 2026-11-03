# Registro de passos de configuração — Sessão 1

**Data:** _______ · **Alvo:** BitDogLab v6.3 (Pico W) · **Blocos cobertos:** 1 e 2
**Máquina:** Windows 11 + WSL2 (Ubuntu), Pico SDK prévio da extensão VS Code em `C:\Users\...\.pico-sdk` (não utilizado)

---

## Aviso sobre este registro

Reconstituí a lista a partir das dúvidas que você levantou durante a sessão. **Isso enviesa a contagem para baixo:** só enxergo os pontos onde algo travou. Comandos que funcionaram de primeira e não geraram pergunta não aparecem aqui.

Portanto: confira linha a linha contra o seu histórico do shell antes de transportar qualquer número para a planilha.

```bash
history | head -100 > ~/tcc/e4/logs/historico_sessao1.txt
```

O `history` é a fonte primária. Esta tabela é apoio.

---

## Convenção de contagem (defina agora, aplique igual nos três alvos)

Vale a regra da §8 do roteiro, com duas precisões que a sessão de hoje obrigou a fixar:

1. **Ação física de bancada conta como passo.** Pôr a placa em BOOTSEL é passo, assim como plugar o cartão SD na Raspberry Pi. Se contar num alvo, conte em todos.
2. **Tentativa que falhou não conta como passo.** Vai para a tabela de becos sem saída, que é material qualitativo. O passo é a ação que efetivamente configurou alguma coisa.
3. **Instrumentação do experimento não conta.** Registrar versões em `logs/versoes.txt` e rodar `git commit` existem por causa do TCC, não por causa da plataforma.

---

## Passos — Bloco 1 (toolchain WSL)

| # | Ação | Confirmar |
|---|---|---|
| 1 | Verificar distro WSL (`wsl -l -v`) — e instalar Ubuntu, se não havia | ☐ havia ou ☐ instalei |
| 2 | `sudo apt update` | ☐ |
| 3 | `sudo apt install` da toolchain (2ª tentativa, sem o pacote inexistente) | ☐ |
| 4 | `git clone --recurse-submodules` do pico-sdk | ☐ |
| 5 | Exportar `PICO_SDK_PATH` no `.bashrc` e recarregar | ☐ |
| 6 | Criar a árvore `~/tcc/e4/...` e `git init` | ☐ |

## Passos — Bloco 2 (prova de vida)

| # | Ação | Confirmar |
|---|---|---|
| 7 | Criar `vida.c` | ☐ |
| 8 | Criar `CMakeLists.txt` | ☐ |
| 9 | Copiar `pico_sdk_import.cmake` para o projeto | ☐ |
| 10 | `mkdir build` + `cmake .. -DPICO_BOARD=pico_w` | ☐ |
| 11 | `make -j$(nproc)` | ☐ |
| 12 | Pôr a placa em BOOTSEL (BOOTSEL + RESET) | ☐ |
| 13 | Montar o volume `RPI-RP2` no WSL (`mount -t drvfs`) | ☐ foi preciso? |
| 14 | `cp vida.uf2` para o volume | ☐ |
| 15 | Instalar o PuTTY no Windows | ☐ |
| 16 | Configurar a sessão serial (COM, 115200, flow control None, logging) | ☐ |

**Total reconstituído: 16 passos.** Trate como piso, não como valor final.

---

## Becos sem saída (não contam como passo — contam como material)

| Tentativa | O que aconteceu | Onde isso interessa |
|---|---|---|
| `sudo apt install ... libstdc++-arm-none-eabi-newlib` | Pacote não existe mais no Ubuntu corrente; o `apt` aborta a linha inteira | **3.3.3** — documentação oficial desatualizada em relação à distribuição corrente |
| `cd $PICO_SDK_PATH/external/pico_sdk_import.cmake .` | Erro de digitação (`cd` por `cp`); o CMake não achou o arquivo | Erro do operador, não da plataforma — registre, mas não use como crítica |
| Recompilar sem apagar `build/` | Cache do CMake preso em `/usr/bin/cc` | **3.3.5** — o build system explícito do Pico SDK expõe um estado que o usuário precisa saber limpar |
| `cp vida.uf2 /media/$USER/RPI-RP2/` | Caminho de automontagem do Linux não existe no WSL | **3.3.3** — documentação assume Linux nativo; o caminho Windows/WSL não é coberto |
| `minicom -D /dev/ttyACM0` | WSL2 não expõe porta serial sem `usbipd-win` | Idem — limitação de ambiente, não da placa |

Os becos são, para o Capítulo 5, mais úteis que a contagem. Um número diz que foram dezesseis passos; a tabela acima diz **por que** e **onde** a documentação falha, que é o que o critério 3.3.3 realmente mede.

---

## Alvo Raspberry Pi 4 — contagem separada

Passos do Imager e da Pi **não entram** na contagem da BitDogLab. Registre à parte:

| # | Ação | Confirmar |
|---|---|---|
| 1 | Instalar o Raspberry Pi Imager no Windows | ☐ |
| 2 | Selecionar dispositivo, sistema (Lite 64-bit) e armazenamento | ☐ |
| 3 | Configurar host, usuário, Wi-Fi e SSH na engrenagem | ☐ |
| 4 | Gravar o cartão | ☐ |
| 5 | Inserir o cartão e ligar a placa | ☐ |
| 6 | Primeiro acesso por SSH | ☐ |
| 7 | `apt update && full-upgrade` + reboot | ☐ |
| 8 | `pip3 install edge_impulse_linux` | ☐ |

Beco registrado: confusão entre o campo "Dispositivo Raspberry Pi" e o campo "Armazenamento" do Imager. Interessa ao critério de abstração didática — a interface é de um clique, mas a nomenclatura induz ao erro em quem usa pela primeira vez.

---

## Decisões tomadas na sessão (não são passos)

Registre no diário do projeto, não na planilha:

- Cartão de 32 GB é suficiente para o RPi OS Lite 64-bit. Anotar marca, modelo e classe (A1/A2).
- A UNO Q não aceita cartão: armazenamento em eMMC de 16 ou 32 GB, conforme a variante. Exige fonte USB-C com Power Delivery, mínimo recomendado 15 W. **Item novo para a lista de materiais.**
- Raspberry Pi Connect: não habilitar. Carga de fundo não controlada contaminaria a medição de dispersão de latência.
- Build no WSL, gravação e monitor serial pelo Windows. `usbipd-win` adiado.

---

## O que falta preencher

- [ ] Data da sessão
- [ ] Tempo total cronometrado, do início do Bloco 1 à primeira linha `vivo` no PuTTY
- [ ] Conferir os 16 passos contra `history`
- [ ] Item 13: o `mount -t drvfs` foi necessário ou o `/mnt/d` já existia?
- [ ] Item 1: o Ubuntu já estava instalado no WSL ou foi instalado hoje?
- [ ] Conteúdo de `logs/versoes.txt` transcrito para a planilha
- [ ] Marca, modelo e classe do cartão SD

---

> Este registro é rascunho de bancada. Antes de virar número no Capítulo 5, precisa ser conferido por você contra o histórico do shell — e, sendo dado que sustenta conclusão do trabalho, revisado com o orientador.
