# E3 — Multitarefa e determinismo

Desvio entre o instante previsto e o instante efetivo de execução de uma tarefa periódica de 10 ms, em repouso e sob contenção deliberada de processador. O experimento demonstra o que distingue os domínios quanto a previsibilidade temporal.

## Resultados

Quinhentas repetições por condição.

| Alvo | Ocioso (mediana / pior) | Carga no mesmo núcleo | Carga em núcleo separado |
|---|---|---|---|
| BitDogLab (bare metal) | < 1 µs / < 1 µs | 125 µs / **250 µs** | **< 1 µs / < 1 µs** |
| Arduino UNO Q (Zephyr) | 2 µs / 26 µs | 40 µs / 137 µs | não aplicável |
| Raspberry Pi 4 (Linux) | 0,4 µs / 1,3 µs | 6.107 µs / **19.950 µs** | não formulável |

**O número decisivo não é o fator entre as condições, e sim a relação com o período.** No alvo Linux, 19,95 ms de atraso sobre uma tarefa de 10 ms significam **dois períodos inteiros perdidos**; o percentil 95, em 15.957 µs, indica que mais de 5% das repetições perderam ao menos um. No alvo microcontrolado, o pior caso é 2,5% do período.

**A condição de carga em núcleo separado é o que distingue a BitDogLab.** Com o segundo núcleo integralmente ocupado, a tarefa periódica do primeiro não sofreu degradação mensurável. No alvo Linux essa condição não é sequer formulável: o escalonador decide onde cada tarefa executa.

## O padrão da terceira condição contém o conceito

Na BitDogLab, os desvios distribuíram-se de modo aproximadamente uniforme entre zero e 250 µs, com mediana em 125 — exatamente metade do máximo. Isso ocorre porque o bloco de trabalho concorrente **não é interrompível** na ausência de escalonador: a tarefa periódica só retoma o controle ao término do bloco corrente.

O pior caso é, portanto, **calculável a partir do código**, e não estimável a partir de observação. Determinismo, nesse sentido, não significa ausência de desvio, mas desvio previsível.

Convém não atribuir garantia automática ao alvo sem sistema operacional: também nele o cumprimento de prazos depende de interrupções, bloqueios, compartilhamento de recursos e análise do tempo de execução no pior caso. O que ele oferece é comportamento temporal analisável.

## O que os números da UNO Q revelam

O registro de compilação mostra que o núcleo Arduino dessa plataforma é construído sobre o **Zephyr**. O sistema dispõe de escalonador preemptivo e interface de criação de threads; a camada Arduino não as expõe, razão pela qual a condição de carga em núcleo separado não pôde ser reproduzida.

Os próprios desvios denunciam o escalonador: em repouso, enquanto o alvo bare metal ficou abaixo da resolução de medida, a UNO Q registrou máximo de 26 µs — compatível com a interrupção periódica de temporização do sistema.

Sob carga, a UNO Q saiu melhor que a BitDogLab (137 µs contra 250 µs). Não é vantagem de arquitetura de concorrência: o Cortex-M33 a 160 MHz executa o mesmo bloco de trabalho mais rápido.

## Arquivos

```
bitdoglab/e3_tarefas.c        três condições, com carga no segundo núcleo via multicore_launch_core1
raspberry-pi/e3_medir.py      duas condições, com carga disparada pelo próprio script
uno-q/e3_tarefas/             aplicação exportada do Arduino App Lab
```

Montagem: apenas um LED em série com resistor de 220 Ω, alternado a cada período para tornar a tarefa observável. Na BitDogLab usa-se o LED RGB de bordo (GP13); na Raspberry Pi, o GPIO 18; na UNO Q, o D9.

## Como reproduzir

**BitDogLab e UNO Q.** Procedimento de compilação idêntico ao dos demais experimentos. O programa percorre as três condições sozinho e imprime mediana, percentil 95 e máximo de cada uma.

**Raspberry Pi 4.** Requer `gpiozero` e `stress-ng`. Fixe o governador antes de medir:

```bash
sudo apt install -y stress-ng
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
python3 e3_medir.py
vcgencmd get_throttled      # deve retornar 0x0 ao final
```

## Ressalvas

Os valores inferiores a um microssegundo situam-se **abaixo da resolução do instrumento** — `time_us_32()` no alvo microcontrolado — e não devem ser lidos como ausência de desvio.

A carga é disparada pelo próprio script, com dezesseis processos concorrentes em quatro núcleos. As duas escolhas corrigem erros da primeira tentativa, documentados em `docs/becos-sem-saida.md`: a carga era iniciada manualmente em terminal separado e terminava antes do início da medição, e o número de processos igualava o de núcleos, insuficiente para produzir disputa efetiva. O resultado daquela execução foi idêntico nas duas condições, com o pior caso inclusive menor sob suposta carga — implausibilidade física que denunciou o erro.

A maior variabilidade observada no domínio Linux neste experimento, comparada à de outros eixos, não autoriza isolar a duração da tarefa como causa: as operações e as condições de execução diferem entre experimentos.
