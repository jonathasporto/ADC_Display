#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"  // Biblioteca do display SSD1306

// Definições de pinos
#define I2C_PORT i2c0
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define BUTTON_JOYSTICK_PIN 22
#define BUTTON_A_PIN 5
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

// Variáveis de estado
bool leds_pwm_enabled = true;
bool green_led_state = false;
int border_style = 0;

// Inicialização do display SSD1306
ssd1306_t display;

// Função para tratar debounce
bool debounce(uint gpio) {
    static absolute_time_t last_time;
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_time, now) > 200000) {  // 200ms debounce
        last_time = now;
        return true;
    }
    return false;
}

// Função de callback para o botão do joystick
void joystick_button_callback(uint gpio, uint32_t events) {
    if (!debounce(gpio)) return;

    // Alterna o LED verde
    green_led_state = !green_led_state;
    gpio_put(LED_GREEN_PIN, green_led_state);

    // Alterna o estilo de borda
    border_style = (border_style + 1) % 3;

    // Atualiza o display com a nova borda
    ssd1306_fill(&display, false);  // Limpa o display
    switch (border_style) {
        case 0:
            ssd1306_rect(&display, 0, 0, 128, 64, true, false);  // Borda simples
            break;
        case 1:
            ssd1306_rect(&display, 2, 2, 124, 60, true, false);  // Borda interna
            break;
        case 2:
            ssd1306_rect(&display, 4, 4, 120, 56, true, false);  // Borda menor
            break;
    }
    ssd1306_send_data(&display);  // Atualiza o display
}

// Função de callback para o botão A
void button_a_callback(uint gpio, uint32_t events) {
    if (!debounce(gpio)) return;

    // Ativa/desativa os LEDs PWM
    leds_pwm_enabled = !leds_pwm_enabled;
    if (!leds_pwm_enabled) {
        pwm_set_gpio_level(LED_RED_PIN, 0);
        pwm_set_gpio_level(LED_BLUE_PIN, 0);
    }
}

// Função para configurar os pinos de PWM
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 65535);  // 16 bits de resolução
    pwm_set_enabled(slice_num, true);
}

int main() {
    // Inicializa os pinos
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

    // Configura os pinos de PWM
    setup_pwm(LED_RED_PIN);
    setup_pwm(LED_BLUE_PIN);

    // Configura os botões com interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK_PIN, GPIO_IRQ_EDGE_FALL, true, joystick_button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, button_a_callback);

    // Inicializa o display SSD1306
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    ssd1306_init(&display, 128, 64, false, 0x3C, i2c_default);
    ssd1306_fill(&display, false);  // Limpa o display
    ssd1306_send_data(&display);    // Atualiza o display

    // Posição inicial do quadrado
    int x_pos = 60;
    int y_pos = 28;

    while (true) {
        // Leitura do eixo X
        adc_select_input(0);  // Seleciona o ADC para o eixo X
        uint16_t x_val = adc_read();

        // Leitura do eixo Y
        adc_select_input(1);  // Seleciona o ADC para o eixo Y
        uint16_t y_val = adc_read();

        // Ajuste de PWM conforme o joystick
        if (leds_pwm_enabled) {
            // LED Vermelho (eixo X)
            uint16_t red_intensity = (x_val > 2048) ? (x_val - 2048) * 16 : (2048 - x_val) * 16;
            pwm_set_gpio_level(LED_RED_PIN, red_intensity);

            // LED Azul (eixo Y)
            uint16_t blue_intensity = (y_val > 2048) ? (y_val - 2048) * 16 : (2048 - y_val) * 16;
            pwm_set_gpio_level(LED_BLUE_PIN, blue_intensity);
        }

        // Atualização da posição do quadrado
        x_pos = (x_val * 120) / 4095;  // Escala para 0-120 (128 - 8 pixels)
        y_pos = (y_val * 56) / 4095;   // Escala para 0-56 (64 - 8 pixels)

        // Desenha o quadrado no display
        ssd1306_fill(&display, false);  // Limpa o display
        ssd1306_rect(&display, x_pos, y_pos, 8, 8, true, true);  // Quadrado 8x8 preenchido
        ssd1306_send_data(&display);  // Atualiza o display

        sleep_ms(10);  // Atualização a cada 10ms
    }

    return 0;
}