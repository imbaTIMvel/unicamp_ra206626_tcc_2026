# Prós e contras de soluções comerciais para Sistemas Embarcados com Edge AI

Material de apoio ao Trabalho de Conclusão de Curso em Engenharia Elétrica,
Faculdade de Engenharia Elétrica e de Computação, Universidade Estadual de Campinas, 2026.

Compara, segundo sete critérios técnicos e pedagógicos, a Arduino UNO Q e a combinação
Raspberry Pi Pico W com Raspberry Pi 4, ao longo de quatro eixos: sensores e atuadores,
comunicação, multitarefa e inteligência artificial embarcada.

Este repositório reúne os códigos, os modelos, os dados e as condições de ambiente
necessários para reproduzir os experimentos descritos no trabalho.

## Resultados em uma tabela

| Eixo | BitDogLab (RP2040) | Arduino UNO Q | Raspberry Pi 4 |
|---|---|---|---|
| E1 — leitura analógica e PWM | 2.702 ns | 150.010 ns | 7.357 ns |
| E2 — transação I²C a 400 kHz | 241.651 ns | 210.413 ns | 242.093 ns |
| E3 — desvio sob contenção | 250 µs | 137 µs | 19.950 µs |
| E4 — inferência | 931 ms | 9,6 ms (domínio Linux) | 12,9 ms |

Condições de medição, ressalvas de comparabilidade e limitações constam do trabalho
e dos registros em `docs/`.

## Organização

- `experimentos/` — um diretório por eixo, subdividido por plataforma
- `ambiente/` — versões de toolchain, sistema e condições de contorno
- `dados/` — saídas brutas e planilha consolidada
- `docs/` — registros de bancada, proveniência dos scripts e percursos malsucedidos
- `figuras/` — diagramas do trabalho

## Reproduzir

Cada diretório de experimento traz um `README.md` com montagem, compilação e execução.
Os projetos do RP2040 exigem o Pico SDK; os do domínio Linux, o runtime indicado em
`ambiente/`. O conjunto de treinamento não é versionado — veja
`experimentos/e4-edge-ai/dataset/README.md`.

## Declaração de vínculo

O Raspberry Pi Pico W empregado está soldado em uma BitDogLab v6.3, placa-portadora
desenvolvida no projeto Escola 4.0 da FEEC/Unicamp, coordenado pelo orientador deste
trabalho. A placa é tratada como instrumentação, não como solução avaliada. As medidas
adotadas para neutralizar esse vínculo constam da seção 1.7 da dissertação.

## Licença

Código sob MIT; dados, figuras e documentação sob CC BY 4.0.
