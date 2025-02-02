# Projeto de Contagem com Matriz de LEDs e Botões

## Descrição

Este projeto utiliza a placa **BitDogLab** para controlar uma **matriz de LEDs 5x5 WS2812** e um **LED RGB**. Ele implementa um contador visual, onde o número exibido na matriz pode ser incrementado ou decrementado usando dois botões. O LED RGB também pisca com um efeito visual.

## Componentes

- **Matriz de LEDs 5x5 WS2812** conectada à **GPIO 7**.
- **LED RGB (vermelho)** conectado à **GPIO 13**.
- **Botão A** conectado à **GPIO 5**.
- **Botão B** conectado à **GPIO 6**.

## Funcionalidades

1. **LED Vermelho**: Pisca continuamente 5 vezes por segundo.
2. **Botão A (GPIO 5)**: Incrementa o número exibido na matriz de LEDs.
3. **Botão B (GPIO 6)**: Decrementa o número exibido na matriz de LEDs.
4. **Matriz de LEDs**: Exibe números de 0 a 9, utilizando um formato fixo (ex: estilo digital).

## Requisitos

1. **Uso de interrupções (IRQ)** para tratar as ações dos botões.
2. **Debouncing**: Implementação do tratamento de bouncing dos botões via software.
3. **Controle de LEDs**: Utilização de LEDs comuns e LEDs WS2812, demonstrando o controle de diferentes tipos de LEDs.
4. **Código Organizado**: Código estruturado e comentado para fácil compreensão.

## Como Funciona

- O **LED vermelho RGB** pisca a uma taxa de 5 vezes por segundo, alternando entre ligado e desligado.
- O número exibido na **matriz de LEDs 5x5** é alterado a cada pressionamento dos botões A e B.
- O formato de exibição dos números é fixo, com os LEDs iluminando-se para formar os números de 0 a 9.


## Vídeo Demonstrativo:

[![Assista ao vídeo demonstrativo](https://i.ytimg.com/vi/m4CvZlHNlw0/hq720_2.jpg?sqp=-oaymwEoCJUDENAFSFryq4qpAxoIARUAAIhC0AEB2AEB4gEKCBgQAhgGOAFAAQ==&rs=AOn4CLDEAiMRf0h5bnH_93G-STIvsmkGAQ)](https://www.youtube.com/shorts/m4CvZlHNlw0)
