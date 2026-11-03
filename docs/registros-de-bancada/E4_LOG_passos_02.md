# Registro de passos de configuração — Sessão 2

**Data:** _______ · **Bloco(s):** 3 (dataset) e 4 (treino no Edge Impulse)
**Escopo:** etapa comum aos três alvos — ainda não há alvo de hardware envolvido
**Resultado:** modelo treinado, 88,64% de acurácia no conjunto de teste

---

## Passos de configuração — Bloco 4

Pela convenção fixada na sessão 1, o Bloco 3 não conta (baixar dataset público é custo do problema, idêntico nos três alvos). O Bloco 4 conta inteiro, e será comparado depois com o fluxo do Qualcomm AI Hub.

| # | Ação |
|---|---|
| 1 | Criar conta no Edge Impulse |
| 2 | Criar projeto do tipo Audio (*Get Started → Audio detection → Build my first model*) |
| 3 | Criar segundo projeto do zero, após descartar o primeiro |
| 4 | Coletar amostras da keyword "hello" (23) |
| 5 | Coletar amostras da keyword "bye" (24) |
| 6 | Dividir o dataset entre treino e teste (74/26) |
| 7 | Acrescentar amostras de "unknown" e "background noise" |
| 8 | Configurar o Impulse: bloco MFCC + bloco Classifier |
| 9 | Gerar as features |
| 10 | Salvar o Impulse e treinar o modelo |
| 11 | Rodar *Model testing* |

**Total parcial: 11 passos.** O Bloco 4 não está fechado — faltam os passos de deployment (ver Pendências).

## Bloco 3 — não contabilizado

| Ação | Situação |
|---|---|
| `wget` do Google Speech Commands v0.02 (2,3 GB) | **baixado, não utilizado** |

Registre isso como está. O caminho previsto era o dataset público; o caminho efetivamente tomado foi a gravação própria pelo assistente do Edge Impulse. Os arquivos continuam em `~/tcc/e4/dataset/` e podem servir depois, se você quiser ampliar a classe "unknown".

---

## Becos sem saída

| Tentativa | O que aconteceu | Onde interessa |
|---|---|---|
| Primeiro projeto, keyword "hello world", 14 amostras | Amostras mal executadas quanto ao timing da gravação; projeto descartado e refeito do zero | **3.3.5** — o assistente guiado leva ao primeiro modelo rápido, mas não alerta sobre qualidade de gravação; o erro só aparece depois do treino |

Esse beco é bom material. Ele mostra concretamente o que a abstração alta do fluxo guiado entrega e o que ela esconde: você chega a um modelo treinado em minutos, sem entender por que ele não presta. Vale um parágrafo no Capítulo 5.

---

## Desvios em relação ao plano (documentar, não corrigir)

Três desvios. Nenhum invalida nada, mas os três precisam estar declarados na dissertação.

**1. Dataset próprio em vez do Google Speech Commands.** O modelo foi treinado com ~47 gravações da sua própria voz, não com um corpus público multi-locutor. Consequência: o modelo é dependente de locutor. Isso **não** compromete o experimento, porque o E4 mede o pipeline de implantação, não a qualidade do classificador — mas torna a acurácia um número interno, não comparável com literatura.

**2. Keywords "hello" e "bye" em vez de "yes" e "no".** Sem problema; são foneticamente distantes, que era o critério.

**3. MFCC em vez de MFE.** Também sem problema — MFCC é o extrator clássico para fala. O que importa é que **o mesmo bloco de DSP vá para todos os alvos**, e isso está garantido: tanto a biblioteca C++ quanto o `.eim` embutem o Impulse inteiro, DSP incluído. Registre o extrator escolhido, porque ele afeta o tempo de `timing.dsp` que você vai reportar.

---

## Observação comparativa a guardar para o eixo 4.4

Gravar amostras direto no navegador, sem instalar nada e sem escrever uma linha de código, é recurso **da ferramenta**, não do problema. Quando você percorrer o caminho do Qualcomm AI Hub para o domínio MPU da UNO Q, verifique se existe equivalente — e, se não existir, quantos passos são necessários para chegar de áudio bruto a dataset rotulado.

Essa diferença de esforço na etapa de aquisição é dado do critério 3.3.6 e não aparece em nenhuma comparação de latência ou de tamanho de modelo. É exatamente o tipo de evidência que justifica o experimento existir.

---

## Sobre os 88,64%

Use esse número pelo que ele é: **verificação de sanidade entre alvos, não medida de qualidade do modelo.**

Com um conjunto de teste de cerca de uma dúzia de amostras, a incerteza em torno desse valor é larga — uma amostra a mais ou a menos move vários pontos percentuais. Não reporte 88,64% como se fossem quatro algarismos significativos confiáveis.

O que o número precisa fazer é **se repetir nos três alvos**. Se a BitDogLab der 88,64% e a Raspberry Pi der 70%, há erro de implantação em algum lugar, não diferença de plataforma. É para isso que ele serve, e para isso ele basta.

---

## Ação urgente: preservar o dataset

Suas 47 gravações existem hoje em um único lugar — a nuvem do Edge Impulse. Se a conta, o projeto ou o plano gratuito mudarem, o experimento se torna irreproduzível, e reprodutibilidade é critério declarado do seu trabalho (seção 2.1.4).

Antes da próxima sessão:

- [ ] *Dashboard → Export* — baixe o projeto inteiro (dados + Impulse)
- [ ] Guarde em `~/tcc/e4/dataset/projeto_ei/` e faça commit
- [ ] Anote o **ID do projeto** e a **versão do Impulse** em `logs/versoes.txt`

Dez minutos agora contra um experimento perdido depois.

---

## Pendências do Bloco 4

Antes de ir para o Bloco 5:

- [ ] *Deployment → C++ library*, quantização **int8**, EON Compiler se disponível
- [ ] **Anotar "peak RAM usage" e "flash usage"** na tela de deployment, antes de sair dela
- [ ] *Dashboard* → baixar o `.tflite` int8 (o tamanho do arquivo é a métrica "tamanho do modelo")
- [ ] Baixar também o deployment **Linux (AARCH64)** para o Bloco 7
- [ ] Anotar quantos passos o deployment exigiu, somando aos 11 acima

---

## O que anotar na planilha

| Campo | Valor |
|---|---|
| Keywords | hello, bye |
| Amostras por keyword | 23 / 24 |
| Classes | 4 (hello, bye, unknown, noise) |
| Split treino/teste | 74/26 |
| Extrator | MFCC |
| Acurácia (conjunto de teste) | 88,64% |
| Origem do dataset | gravação própria, locutor único |
| Passos do Bloco 4 | 11 + deployment |

---

## Próxima sessão começa em

**Deployment do Bloco 4** (quatro itens acima), depois **Bloco 5** (buffer fixo) e **Bloco 6** (implantar na BitDogLab).

Para o Bloco 5, escolha como buffer fixo **uma gravação do conjunto de teste que o modelo classifica corretamente** — a tela de *Model testing* mostra o resultado por amostra. Você vai precisar do `.wav`; baixe pela própria interface do Edge Impulse.

Um cuidado específico das gravações próprias: o `wav_para_c.py` tem um `assert` exigindo 16 kHz e um canal. Gravação feita pelo navegador pode vir em outra taxa, e o Edge Impulse reamostra internamente sem necessariamente reamostrar o arquivo que você baixa. Confira antes:

```bash
python3 -c "import wave; w=wave.open('palavra.wav'); print(w.getframerate(), w.getnchannels(), w.getsampwidth())"
```

Se não vier `16000 1 2`, converta: `ffmpeg -i palavra.wav -ar 16000 -ac 1 -sample_fmt s16 palavra_16k.wav`.

O *live testing* que você adiou é o Bloco 9, e ele vem por último mesmo. Não é medição — é demonstração, e depende de tudo o mais estar funcionando.

---

> Rascunho de bancada. Confira contra `history` e contra o log do projeto no Edge Impulse antes de transportar números para o Capítulo 5.
