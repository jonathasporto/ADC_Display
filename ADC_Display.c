#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"

// Definição do endereço I2C do display OLED
#define OLED_ADDR 0x3C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// Definição dos pinos dos LEDs
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12

// Definição dos pinos dos botões
#define BUTTON_A_PIN 5
#define BUTTON_JOYSTICK_PIN 22

// Definição dos pinos do joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27

// Definição da faixa de zona morta (joystick solto)
#define DEADZONE_MIN 1850
#define DEADZONE_MAX 2000

// Definição do valor máximo do ADC
#define ADC_MAX 4095

bool led_enabled =  true;
bool led_green_state = false;
bool border_style = true;
int border_size = 2;

volatile uint32_t last_time_Button_A = 0;
volatile uint32_t last_time_Button_Joystick = 0;
const uint32_t debounce = 200;

// Função para inicializar o PWM nos pinos dos LEDs
void init_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, ADC_MAX);
    pwm_set_enabled(slice_num, true);
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    // verfica se o botao A ou o botao do joystick foi pressionado e aplica o debounce
    uint32_t start_time = to_ms_since_boot(get_absolute_time()); 
    if (gpio == BUTTON_JOYSTICK_PIN && (start_time - last_time_Button_Joystick > debounce)){
        last_time_Button_Joystick = start_time;
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN_PIN, led_green_state);
        border_size = (border_size == 2) ? 4 : 2;
    } else if (gpio == BUTTON_A_PIN && (start_time - last_time_Button_A > debounce)){
        last_time_Button_A = start_time;
        led_enabled = !led_enabled;
    }
}

int main() {
    // Inicialização do stdio
    stdio_init_all();

    // Inicialização do display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_t display;
    ssd1306_init(&display, WIDTH, HEIGHT, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Inicialização do ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Inicialização do PWM para os LEDs
    init_pwm(LED_RED_PIN);
    init_pwm(LED_BLUE_PIN);

    // Inicialização e configuração dos GPIOs
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_init(BUTTON_JOYSTICK_PIN);
    gpio_set_dir(BUTTON_JOYSTICK_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Laço principal
    while (1) {
        // Leitura dos valores do joystick
        adc_select_input(0);
        uint16_t x_value = adc_read();  // Eixo X (LED Vermelho)

        adc_select_input(1);
        uint16_t y_value = adc_read();  // Eixo Y (LED Azul)

        // calcular os valores de x e y para o pwm
        uint16_t pwm_x = led_enabled ? abs(x_value - 2048) : 0;
        uint16_t pwm_y = led_enabled ? abs(y_value - 2048) : 0;

        // Controle do LED Vermelho
        if(x_value < DEADZONE_MIN || x_value > DEADZONE_MAX){
            pwm_set_gpio_level(LED_BLUE_PIN, pwm_x);
        } else {
            pwm_set_gpio_level(LED_BLUE_PIN, 0);
        }

        // Controle do LED Azul
        if(y_value < DEADZONE_MIN || y_value > DEADZONE_MAX){
            pwm_set_gpio_level(LED_RED_PIN, pwm_y);
        } else {
            pwm_set_gpio_level(LED_RED_PIN, 0);
        }

        //converte os valores do joystick para o Display
        uint8_t x_display = (y_value * (WIDTH - 8)) / ADC_MAX;
        uint8_t y_display = (x_value * (HEIGHT - 8)) / ADC_MAX;
        y_display = HEIGHT - 8 - y_display;

        // Atualização do display OLED
        ssd1306_fill(&display, false);
        ssd1306_rect(&display, y_display, x_display, 8, 8, true, true); // Desenha o quadrado controlado pelo joystick

        // desenha a borda do display
        for(int i = 0; i < border_size; i++){
            ssd1306_rect(&display, i, i, WIDTH - (2*i), HEIGHT - (2*i), true, false);
        }

        ssd1306_send_data(&display);

        // Pequeno delay para evitar leituras muito rápidas
        sleep_ms(10);
    }

    return 0;
}
