#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "numeros/numeros.h"
#include "ProjetoFinal.pio.h"

#define WS2812_PIN 7

#define JOY_Y_ADC 0

#define RED_LED_PIN 13

#define BUTTON_A 5

#define DEBOUNCE_DELAY_MS 500

#define PWM_WRAP 255

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

volatile static uint32_t last_button_press = 0;
volatile bool system_on = true; // true = ligado, false = desligado
volatile bool last_system_state = true;
uint8_t current_percent = 0;
static bool cond = true;

void iniciar() {
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

void desligar(ssd1306_t *ssd, PIO pio, uint sm, bool *previous_system_on) {
    // Desliga o LED (PWM para 0)
    pwm_set_gpio_level(RED_LED_PIN, 0);
    // Limpa o display
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    // Apaga a matriz de LEDs (envia 0 para todos os pixels)
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, 0);
    }
    *previous_system_on = false; // Atualiza a flag no escopo principal
    sleep_ms(100);
}

void ssd1306(uint16_t adc_y, ssd1306_t *ssd) {
    static uint8_t last_display_percent = 255; // Guarda o último valor exibido

    // Verifica a direção do joystick e ajusta a intensidade
    if (adc_y >= 4070) { // Joystick para cima
        current_percent += 20;
        if (current_percent > 100) {
            current_percent = 100;
        }
        ssd1306_fill(ssd, false);
        ssd1306_draw_string(ssd, "Intensidade: %d", 4, 25);
        ssd1306_send_data(ssd);
    } else if (adc_y <= 20) { // Joystick para baixo
        if (current_percent < 20)
            current_percent = 0;
        else
            current_percent -= 20;
    }

    // Se houve mudança na intensidade, atualiza o display
    if (current_percent != last_display_percent) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Intensidade: %3d%%", current_percent);
        ssd1306_fill(ssd, false);
        ssd1306_draw_string(ssd, buffer, 4, 28);
        ssd1306_send_data(ssd);
        last_display_percent = current_percent;
    }

    // Atualiza o PWM do LED vermelho
    uint16_t pwm_level = (current_percent * PWM_WRAP) / 100;
    pwm_set_gpio_level(RED_LED_PIN, pwm_level);
    sleep_ms(100);
}

uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    // WS2812 espera os dados no formato GRB nos 24 bits mais significativos
    return (G << 24) | (R << 16) | (B << 8);
}

void desenho_pio(double *desenho, PIO pio, uint sm, double r, double g, double b) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t led_val = matrix_rgb(desenho[24 - i] * r, 0, 0);
        pio_sm_put_blocking(pio, sm, led_val);
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - last_button_press) < DEBOUNCE_DELAY_MS)
        return;
    last_button_press = now;

    if (gpio == BUTTON_A && (events & GPIO_IRQ_EDGE_FALL)) {
        last_system_state = system_on;
        cond = !cond;
        system_on = !system_on;
    }
}

double *definir_desenho(double *desenho) {
    switch (current_percent) {
        case 0:
            desenho = zeroPorcento;
            break;
        case 20:
            desenho = vintePorcento;
            break;
        case 40:
            desenho = quarentaPorcento;
            break;
        case 60:
            desenho = sessentaPorcento;
            break;
        case 80:
            desenho = oitentaPorcento;
            break;
        case 100:
            desenho = cemPorcento;
            break;
        default:
            desenho = zeroPorcento;
            break;
    }
    sleep_ms(100);
    return desenho;
}

int main() {
    stdio_init_all();
    iniciar();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ProjetoFinal_program);
    uint sm = pio_claim_unused_sm(pio, true);
    ProjetoFinal_program_init(pio, sm, offset, WS2812_PIN);

    adc_init();
    adc_gpio_init(26);

    gpio_set_function(RED_LED_PIN, GPIO_FUNC_PWM);
    uint slice_num_red = pwm_gpio_to_slice_num(RED_LED_PIN);
    pwm_set_wrap(slice_num_red, PWM_WRAP);
    pwm_set_enabled(slice_num_red, true);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    uint8_t last_display_percent = 255;
    bool previous_system_on = false; // Variável para detectar a transição de desligado para ligado

    while (true) {
        if (!system_on) {
            desligar(&ssd, pio, sm, &previous_system_on);
            sleep_ms(100); // Evita uso excessivo da CPU
            continue;      // Pula o restante do loop
        }

        // Verifica se o sistema acabou de ser ligado após estar desligado
        if (!previous_system_on) {
            // Exibe a mensagem de sistema iniciado
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "Sistema iniciado", 4, 25);
            ssd1306_send_data(&ssd);
            previous_system_on = true; // Atualiza o estado anterior
        }

        // Sistema ligado: processa entradas
        adc_select_input(JOY_Y_ADC);
        uint16_t adc_y = adc_read();

        // Zona morta e atualização do display
        const uint16_t DEADZONE_LOW = 1800, DEADZONE_HIGH = 2300;
        if (adc_y < DEADZONE_LOW || adc_y > DEADZONE_HIGH) {
            ssd1306(adc_y, &ssd);
            sleep_ms(100);
        }
        uint16_t pwm_level = (current_percent * PWM_WRAP) / 100;
        pwm_set_gpio_level(RED_LED_PIN, pwm_level);

        // Atualização da matriz de LEDs
        double *desenho = definir_desenho(desenho);
        desenho_pio(desenho, pio, sm, 1.0, 0.0, 0.0);

        sleep_ms(100); // Reduz o uso da CPU
    }
}