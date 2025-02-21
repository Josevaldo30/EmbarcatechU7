// Projeto de varal inteligente utilizando sensor de chuva com placa LM393 e motor de passo modelo A4988 ou DRV8825.
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/adc.h"

// Definições de pinos e constantes
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco_display 0x3C // Endereço do display SSD1306
#define MAX_STEPS 1000 // limite para evitar que o motor gire indefinidamente

// Definições para o motor de passo
#define STEP_PIN 12
#define DIR_PIN 13

// Estrutura para mapear os pinos utilizados no projeto
typedef struct {
    const int VRX;            // Pino analógico para leitura do sensor de chuva
    const int ADC_CHANNEL_0;  // Canal ADC para leitura do sinal analógico
    const int DIN;            // Pino digital para leitura do sinal do sensor de chuva
    const int LED_R;          // Pino para o LED vermelho (simulação)
    const int LED_G;          // Pino para o LED verde (simulação)
    const int LED_B;          // Pino para o LED azul (simulação)
    const int button_B;       // Pino para o botão (simulação)
} PIN;

// Inicialização dos pinos conforme a estrutura
static const PIN def = {26, 1, 5, 13, 12, 11, 6};

// Enumeração para os estados do sistema
typedef enum {
    ESTADO_INICIAL = 0,  // Estado inicial do sistema
    ESTADO_CHUVA = 1,    // Estado quando chuva é detectada
    ESTADO_SEM_CHUVA = 2 // Estado quando não há chuva
} Estados;

uint8_t estado_anterior = ESTADO_INICIAL; // Variável para armazenar o estado atual do sistema
uint8_t estado = ESTADO_INICIAL; // Variável para armazenar o estado atual do sistema

// Estrutura para o display OLED
ssd1306_t ssd;

// Função para gerar um pulso no pino STEP (para mover o motor de passo)
void step() {
    gpio_put(STEP_PIN, 1);  // Seta o pino STEP para HIGH
    sleep_ms(250);          // Aguarda um curto período
    gpio_put(STEP_PIN, 0);  // Seta o pino STEP para LOW
    sleep_ms(250);          // Aguarda outro curto período
    //O tempo do sleep_ms define a duração do pulso HIGH e o intervalo entre pulsos. Quanto menor o intervalo, mais rápido será o movimento.
}

// Função para mover o motor em uma direção específica
void move_steps(uint32_t steps, bool direction) {
    gpio_put(DIR_PIN, direction);  // Define a direção (0 ou 1) no drive do motor de passo, modelo (A4988 ou DRV8825).
    if (steps > MAX_STEPS)
        steps = MAX_STEPS;
    for (uint32_t i = 0; i < steps; i++) {
        step();  // Gera um pulso para mover um passo
    }
}

// Função para configurar o sensor de chuva
void setup_Sensor_de_chuva() {
    adc_init();               // Inicializa o ADC
    adc_gpio_init(def.VRX);   // Configura o pino VRX como entrada analógica
}

// Função para ler os valores do sensor de chuva
void read_Sensores_de_chuva(uint16_t *VRX_value) {
    adc_select_input(def.ADC_CHANNEL_0);  // Seleciona o canal ADC correspondente ao sensor
    sleep_ms(2);                          // Aguarda estabilização do sinal
    *VRX_value = adc_read();              // Lê o valor analógico do sensor
}

// Função para configurar a comunicação I2C e o display OLED
void setup_i2c() {
    i2c_init(I2C_PORT, 400 * 1000);  // Inicializa o I2C com frequência de 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL para I2C
    gpio_pull_up(I2C_SDA);  // Habilita pull-up no pino SDA
    gpio_pull_up(I2C_SCL);  // Habilita pull-up no pino SCL

    // Inicializa o display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco_display, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);  // Configura o display
    ssd1306_fill(&ssd, false);  // Limpa o display
    ssd1306_send_data(&ssd);  // Envia os dados para o display
}

// Função para configurar os LEDs
void setup_leds() {
    gpio_init(def.LED_R);  // Inicializa o pino do LED vermelho
    gpio_set_dir(def.LED_R, GPIO_OUT);  // Configura o pino como saída
    gpio_init(def.LED_G);  // Inicializa o pino do LED verde
    gpio_set_dir(def.LED_G, GPIO_OUT);  // Configura o pino como saída
    gpio_init(def.LED_B);  // Inicializa o pino do LED azul
    gpio_set_dir(def.LED_B, GPIO_OUT);  // Configura o pino como saída

    // Desliga todos os LEDs inicialmente
    gpio_put(def.LED_R, 0);
    gpio_put(def.LED_G, 0);
    gpio_put(def.LED_B, 0);
}

// Função para configurar os botões
void setup_button() {
    gpio_init(def.DIN);  // Inicializa o pino do sensor digital
    gpio_set_dir(def.DIN, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(def.DIN);  // Habilita pull-up no pino
    gpio_init(def.button_B);  // Inicializa o pino do botão
    gpio_set_dir(def.button_B, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(def.button_B);  // Habilita pull-up no pino
}

// Função para configurar o motor de passo
void setup_motor_passo() {
    gpio_init(STEP_PIN);  // Inicializa o pino STEP
    gpio_set_dir(STEP_PIN, GPIO_OUT);  // Configura o pino como saída
    gpio_init(DIR_PIN);  // Inicializa o pino DIR
    gpio_set_dir(DIR_PIN, GPIO_OUT);  // Configura o pino como saída
}

// Função de callback para interrupções dos sensores
static volatile uint32_t last_time = ESTADO_INICIAL;
void sensor_callback(uint gpio, uint32_t events) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if (gpio == 5) {
        if (tempo_atual - last_time > 200000) {
            last_time = tempo_atual;
            estado = ESTADO_CHUVA;
            estado_anterior = ESTADO_CHUVA;
        }  // Se o pino 5 for acionado, define o estado como "Chuva"
    }
    if(gpio == 6){
        if (tempo_atual - last_time > 200000) {
            last_time = tempo_atual;
            estado = ESTADO_SEM_CHUVA;  // Caso contrário, define o estado como "Sem Chuva"
            estado_anterior = ESTADO_SEM_CHUVA;
        }
    }
}

// Função principal
int main() {
    uint16_t VRX_value;  // Variável para armazenar o valor lido do sensor de chuva
    stdio_init_all();  // Inicializa todas as funcionalidades padrão do Pico
    setup_leds();  // Configura os LEDs
    setup_i2c();  // Configura o I2C e o display
    setup_Sensor_de_chuva();  // Configura o sensor de chuva
    setup_button();  // Configura os botões
    setup_motor_passo();  // Configura o motor de passo

    char string[15];  // Buffer para armazenar a string a ser exibida no display

    // Configura interrupções para os pinos do sensor e botão
    gpio_set_irq_enabled_with_callback(def.DIN, GPIO_IRQ_EDGE_FALL, true, sensor_callback);
    gpio_set_irq_enabled_with_callback(def.button_B, GPIO_IRQ_EDGE_FALL, true, sensor_callback);

    // Loop principal do programa
    while (true) {
        read_Sensores_de_chuva(&VRX_value);  // Lê o valor do sensor de chuva
        sprintf(string, "umidade " "%d%%", (VRX_value * 100 / 4095));  // Formata a string com a umidade

        // Máquina de estados para controlar o comportamento do sistema
        switch (estado) {
            case ESTADO_CHUVA:
                ssd1306_fill(&ssd, 0);  // Limpa o display
                ssd1306_draw_string(&ssd, "Chuva detectada", 0, 10);  // Exibe mensagem de chuva
                ssd1306_draw_string(&ssd, "DESATIVANDO!", 0, 30);  // Exibe mensagem de desativação
                ssd1306_send_data(&ssd);  // Envia os dados para o display
                move_steps(20, 1);  // Move o motor de passo na direção horária
                estado = ESTADO_INICIAL;  // Retorna ao estado inicial
                break;
            case ESTADO_SEM_CHUVA:
                ssd1306_fill(&ssd, 0);  // Limpa o display
                ssd1306_draw_string(&ssd, "Sem chuva", 0, 10);  // Exibe mensagem de sem chuva
                ssd1306_draw_string(&ssd, "ATIVANDO!", 0, 30);  // Exibe mensagem de ativação
                ssd1306_send_data(&ssd);  // Envia os dados para o display
                move_steps(20, 0);  // Move o motor de passo na direção anti-horária
                estado = ESTADO_INICIAL;  // Retorna ao estado inicial
                break;
            default:
                ssd1306_fill(&ssd, 0);  // Limpa o display
                ssd1306_draw_string(&ssd, string, 0, 10);  // Exibe a umidade atual
                ssd1306_draw_string(&ssd, "STAND BY", 0, 30);  // Exibe mensagem de espera
                if (estado_anterior==ESTADO_CHUVA)
                    ssd1306_draw_string(&ssd, "DESATIVADO", 0, 50);  // Exibe mensagem de estado atual
                if (estado_anterior==ESTADO_SEM_CHUVA)
                    ssd1306_draw_string(&ssd, "ATIVADO", 0, 50);  // Exibe mensagem de estado atual
                ssd1306_send_data(&ssd);  // Envia os dados para o display
                break;
        }
        sleep_ms(10);  // Aguarda um curto período antes de repetir o loop
    }
}