# Conjunto de dados

As quatro amostras em `amostras/` são a **entrada comum** submetida aos três alvos,
em `.wav` (original) e `.bin` (int16 bruto, 16 kHz, 16.000 amostras). Os hashes
estão em `CHECKSUMS.md5`. Qualquer divergência nesses arquivos invalida a
comparação de latência entre plataformas.

O conjunto de treinamento **não é versionado aqui**, por tamanho e por licença
própria. Foram utilizadas gravações do autor para as duas palavras-chave, além de
amostras de ruído de fundo e de palavras não pertencentes ao vocabulário.

- Projeto no Edge Impulse: [`TCC_hello_bye`](https://studio.edgeimpulse.com/studio/1123849), id 1123849
- Classes: `bye`, `hello`, `noise`, `unknown`
- Bloco de processamento: MFCC — janela de 1000 ms, passo de 500 ms, 16 kHz
- Exportação: quantização inteira de 8 bits, com compilador de grafo ativo
