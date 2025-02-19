# Joystick Control and LED Display (RP2040)

Este projeto utiliza o microcontrolador RP2040 (Raspberry Pi Pico) para controlar LEDs e um display OLED com base nas leituras de um joystick. Os LEDs são controlados via PWM, e o quadrado desenhado no display OLED é movido conforme o movimento do joystick. Além disso, há botões para controle adicional, utilizando interrupções com debounce.

## Componentes Utilizados

- **Raspberry Pi Pico (RP2040)**
- **Joystick**: Para controle direcional do quadrado no display.
- **LEDs RGB**: Controlados via PWM com base na posição do joystick.
- **Display OLED 128x64**: Exibe um quadrado movido pelo joystick.
- **Botões**:
    - **Botão A**: Liga/desliga a funcionalidade dos LEDs.
    - **Botão Joystick**: Altera a borda do display entre 2 e 4 pixels e ativa o LED verde.
  
## Pinos e Definições

| Componente          | Pino    | Descrição                                                 |
|---------------------|---------|-----------------------------------------------------------|
| **LED Vermelho**    | GPIO 13 | Controla o LED vermelho via PWM                           |
| **LED Verde**       | GPIO 11 | Controla o LED verde (com botão)                          |
| **LED Azul**        | GPIO 12 | Controla o LED azul via PWM                               |
| **Joystick X**      | GPIO 26 | Eixo X do joystick                                        |
| **Joystick Y**      | GPIO 27 | Eixo Y do joystick                                        |
| **Botão A**         | GPIO 5  | Ativa/desativa os LEDs                                    |
| **Botão Joystick**  | GPIO 22 | Altera o tamanho da borda do display e ativa o LED verde  |

## Descrição do Funcionamento

### Controle dos LEDs:
- O eixo X do joystick controla a intensidade do LED **azul**.
- O eixo Y do joystick controla a intensidade do LED **vermelho**.
- A **zona morta** do joystick é configurada entre 1850 e 2000, onde o valor do joystick não afeta o PWM dos LEDs.

### Movimento do Quadrado:
- O quadrado desenhado no display OLED é controlado pelo movimento do joystick.
- O eixo X do joystick move o quadrado para a **esquerda** e **direita**.
- O eixo Y do joystick move o quadrado para **cima** e **baixo**.

### Controle da Borda:
- O botão **Joystick** altera entre bordas de 2 e 4 pixels no display e ativa o LED verde.
- O botão **A** liga e desliga a funcionalidade dos LEDs.

## Uso de Interrupções e Debounce

O controle dos botões é feito por meio de **interrupções**, permitindo que o sistema responda rapidamente aos pressionamentos de botão. O **debounce** é aplicado para garantir que o sistema não detecte múltiplos acionamentos devido ao "ruído" gerado pelo botão.

- **Botão Joystick (GPIO 22)**: Ao ser pressionado, altera o tamanho da borda no display e ativa/desativa o LED verde. A interrupção verifica se o botão foi pressionado, e um *debounce* de 200 ms é aplicado para evitar múltiplas leituras falsas.
- **Botão A (GPIO 5)**: Ao ser pressionado, alterna a funcionalidade dos LEDs. Também usa interrupção e debounce para evitar múltiplos acionamentos acidentais.

### Debounce:
- O **debounce** é um tempo de espera configurado (200 ms) após o acionamento do botão, garantindo que o sistema não registre múltiplas pressões em um curto intervalo.

## Bibliotecas Necessárias

- **pico/stdlib**: Biblioteca padrão do Raspberry Pi Pico.
- **hardware/adc**: Para a leitura do joystick via ADC.
- **hardware/pwm**: Para o controle dos LEDs via PWM.
- **hardware/i2c**: Para comunicação com o display OLED.
- **lib/ssd1306**: Biblioteca para controle do display OLED 128x64 via I2C.

## Como Funciona

1. **Inicialização dos Componentes**:
   - A comunicação I2C é configurada para o display OLED.
   - O ADC é configurado para ler os valores do joystick.
   - Os LEDs são configurados para controle via PWM.
   - Os botões são configurados com interrupções para controle, com debounce de 200 ms.

2. **Leitura do Joystick**:
   - A leitura dos eixos X e Y do joystick é feita via ADC.
   - Os valores dos eixos são mapeados para o controle do PWM dos LEDs e para o movimento do quadrado no display OLED.

3. **Controle de LEDs e Quadrado**:
   - O LED **azul** é controlado com o eixo X do joystick.
   - O LED **vermelho** é controlado com o eixo Y do joystick.
   - O quadrado no display se move de acordo com os valores dos eixos do joystick.

4. **Botões de Controle com Interrupções**:
   - **Botão A**: Alterna a funcionalidade dos LEDs.
   - **Botão Joystick**: Altera o tamanho da borda do display e ativa/desativa o LED verde.
   - Ambos os botões usam interrupções para detectar pressionamentos e aplicam debounce para evitar leituras múltiplas.


## Resultados Esperados

- O quadrado será desenhado no display OLED e se moverá conforme o joystick.
- Os LEDs serão controlados pelo movimento do joystick com a intensidade ajustada pelo eixo X e Y.
- O botão A alternará a funcionalidade dos LEDs e o botão Joystick alternará o tamanho da borda do display, além de ativar o LED verde.

## Video de Funcionamento

[AQUI!]()