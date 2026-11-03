# Registro de passos de configuração — Sessão 6

**Data:** _______ · **Bloco:** 7 — **preparado, não medido**
**Alvo:** Raspberry Pi 4 Model B Rev 1.5, 8 GB

---

## Resumo

Placa acessível por SSH, sistema atualizado, runtime de inferência importando. A medição não foi feita. Quatro dependências não declaradas consumiram a sessão, e isso é resultado, não contratempo.

---

## Ambiente registrado

| Item | Valor |
|---|---|
| Placa | Raspberry Pi 4 Model B Rev 1.5 |
| RAM | 8 GB (7,6 GiB disponíveis ao sistema) |
| Sistema | Debian 13 (trixie) — Raspberry Pi OS Lite 64-bit |
| Arquitetura | `aarch64` ✔ bloqueante superado |
| Throttling | `throttled=0x0` ✔ bloqueante superado |
| Temperatura em repouso | 38,9 °C |
| Governor | **`ondemand`** ⚠ ver abaixo |
| Clock em repouso | 600 MHz |
| Disco | 29 GB, 25% em uso |
| Endereço | 192.168.2.10 |
| Python | 3.13 |
| `edge_impulse_linux` | 1.2.2 |
| `numpy` | 2.2.4 |
| `opencv-python` | 4.14.0.94 |
| `PyAudio` | 0.2.14 |
| `six` | 1.17.0 |

⚠ **O governor está em `ondemand` e o clock em repouso é 600 MHz, não 1,5 GHz.** Como as dez execuções levam frações de segundo, as primeiras podem rodar a 600 MHz antes de o governor reagir — contaminando exatamente a dispersão que o experimento quer medir. Fixe em `performance` antes de medir e registre a decisão.

---

## Passos que contam — alvo Raspberry Pi 4

| # | Ação | Sessão |
|---|---|---|
| 1 | Instalar o Raspberry Pi Imager no Windows | anterior |
| 2 | Selecionar dispositivo, sistema (Lite 64-bit) e armazenamento | anterior |
| 3 | Configurar host, usuário, senha, Wi-Fi e SSH na engrenagem | anterior |
| 4 | Gravar o cartão | anterior |
| 5 | Inserir o cartão e energizar a placa | esta |
| 6 | Primeiro acesso SSH: aceitar a chave do host e autenticar | esta |
| 7 | `sudo apt update && sudo apt full-upgrade -y` | esta |
| 8 | `pip3 install edge_impulse_linux --break-system-packages` | esta |
| 9 | `sudo apt install -y portaudio19-dev` | esta |
| 10 | `pip3 install pyaudio --break-system-packages` | esta |
| 11 | `pip3 install six --break-system-packages` | esta |
| 12 | `pip3 install opencv-python --break-system-packages` | esta |

**Subtotal: 12 passos.** Faltam o `scp` dos arquivos, o ajuste de governor e a execução — algo entre 3 e 5 a mais.

**Dos 12, quatro (do 9 ao 12) são dependências que o pacote oficial não declara.** Um terço da configuração deste alvo existe só para contornar o que a instalação recomendada deixou faltando.

Não contam: as oito verificações de ambiente e o registro de versões — são instrumentação.

---

## Becos sem saída e atritos

| Tentativa | O que houve | Critério |
|---|---|---|
| `ssh ...@rasppi4.local` a partir do WSL | O WSL2 roda em rede virtual própria e não resolve mDNS. Resolvido movendo a sessão para o PowerShell. | ambiente, não plataforma |
| `import edge_impulse_linux` | `ModuleNotFoundError: pyaudio`. O `__init__` do pacote importa o módulo de áudio incondicionalmente. | **3.3.3** |
| `pip3 install pyaudio` | Falha sem `portaudio19-dev`; exigiu biblioteca de sistema via `apt` e compilação local do wheel. | **3.3.3** |
| Novo import | `ModuleNotFoundError: six`. | **3.3.3** |
| Novo import | Aviso de OpenCV ausente — **biblioteca de visão computacional exigida por um runtime usado para classificar áudio a partir de arquivo**. | **3.3.3** |

Nota: o OpenCV instalou de wheel pré-compilado pelo piwheels, em segundos. O risco de compilação de horas que eu havia sinalizado não se concretizou — registre isso também, porque a advertência estava errada.

**Hipótese a verificar, não afirmação:** Debian 13 e Python 3.13 são recentes, e é plausível que a matriz de testes do pacote 1.2.2 não os cubra. Se for esse o caso, o atrito é de defasagem e não de negligência — o que muda a leitura. Vale conferir a data de publicação do pacote e as versões que ele declara suportar antes de escrever qualquer coisa a respeito.

---

## O achado da sessão: dois tipos distintos de atrito

O contraste entre os dois alvos é mais interessante que a contagem.

**Na BitDogLab**, o atrito foi **escrever código que faltava** — a camada de porting para o RP2040. Trabalhoso, mas **previsível**: bastava listar o diretório `porting/` para ver que nenhuma das 26 camadas servia. A documentação não avisava, mas a estrutura do SDK permitia antecipar.

**Na Raspberry Pi**, o atrito foi **descobrir dependências que o pacote não declara**. Cada uma só apareceu quando a anterior foi resolvida, em quatro rodadas de tentativa e erro. Não havia o que ler para antecipar.

Para um trabalho que avalia plataformas segundo o uso em programa de capacitação, a segunda é pior. Um estudante diante de `ModuleNotFoundError: six` não tem como saber se errou, se o pacote está quebrado ou se falta alguma coisa no sistema dele. O primeiro tipo de atrito ensina arquitetura; o segundo só consome tempo.

Vale um parágrafo em 4.4.2, e sugiro escrevê-lo depois da medição, quando houver o contraste completo entre os dois alvos.

---

## Próxima sessão — fechar o Bloco 7

1. Fixar o governor e **registrar**:
   ```bash
   sudo apt install -y cpufrequtils
   sudo cpufreq-set -g performance
   cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
   vcgencmd measure_clock arm
   ```
2. `scp` do `.eim` e dos quatro `.bin` a partir do WSL, para `192.168.2.10`
3. Conferir: 13,4 MB no `.eim`, 32.000 bytes em cada `.bin`; `chmod +x modelo.eim`
4. Rodar o script de paridade e latência — **onze execuções, descartando a primeira**
5. `vcgencmd get_throttled` e `measure_temp` **depois** das execuções
6. Comparar os quatro vetores com os da BitDogLab; a amostra `hello` é a sentinela
7. Reportar mediana **e pior caso**

Lembrete de método: o `.eim` roda como processo separado, com comunicação por socket. O tempo de parede do Python inclui IPC e serialização de 16.000 floats, que não existem no Pico. Os números comparáveis entre alvos são `timing.dsp` e `timing.classification`, reportados pelo próprio runtime; o tempo de parede entra separado, rotulado como latência fim a fim.

---

## Pendências

- [ ] Confirmar se o cartão foi regravado nesta sessão ou se a configuração original já servia — se houve regravação, é retrabalho a registrar
- [ ] `git commit` e histórico do shell
- [ ] Verificar a hipótese sobre a matriz de testes do pacote 1.2.2

---

> Rascunho de bancada. A contagem é reconstituída do log do operador e deve ser conferida contra o histórico do shell.
