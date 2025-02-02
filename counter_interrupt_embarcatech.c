#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"

#define BOTAO_A 5
#define BOTAO_B 6
#define WS2812_PINO 7
#define LED_VERM 13
#define QTD_LEDS 25

absolute_time_t debounce_A;
absolute_time_t debounce_B;
volatile int numero = 0;
PIO pio = pio0;
int unused_sm = 0;

static void gpio_irq_handler(uint gpio, uint32_t events);
bool pisca_led_vermelho(struct repeating_timer *t);
void initialize_config();

// mapeamento da ordem dos leds da matriz de led
static const uint8_t mapeamento_led[5][5] = {
    {24, 23, 22, 21, 20},
    {15, 16, 17, 18, 19},
    {14, 13, 12, 11, 10},
    {5, 6, 7, 8, 9},
    {4, 3, 2, 1, 0}};

// mapeamento dos numeros na matriz de leds
static const uint8_t numeros_matriz[10][QTD_LEDS] = {
    {// 0
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 1
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     1, 1, 1, 1, 1},
    {// 2
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 3
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 4
     1, 0, 0, 1, 0,
     1, 0, 0, 1, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 1, 0,
     0, 0, 0, 1, 0},
    {// 5
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 6
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 7
     1, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 1,
     0, 0, 0, 1, 0,
     0, 0, 0, 1, 0},
    {// 8
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 9
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1}};

void initialize_config()
{
    stdio_init_all(); // Inicializa o sistema de entrada/saída padrão
    // inicia e configura o led vermelho
    gpio_init(LED_VERM);
    gpio_set_dir(LED_VERM, GPIO_OUT);

    // inicia e configura o botão A
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // inicia e configura o botão B
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // configura a interrupção do botão e define o callback
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);

    // PIO para WS2812
    unused_sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, unused_sm, offset, WS2812_PINO, 800000, false); // configura o ws2812
}

void atualizar_matriz_led()
{
    // Define as cores para os LEDs. 'cor_acesa' é a cor que o LED acende (verde),
    // e 'cor_apagada' é a cor quando o LED está apagado (preto).
    uint32_t cor_acesa = 0xFF0000 << 8;  // 0xFF0000 é a cor verde, e o << 8 faz um deslocamento de bits.
    uint32_t cor_apagada = 0x000000 << 8; // 0x000000 é a cor preta (desligado), com deslocamento de bits.

    // Cria um buffer para armazenar o estado de cada LED. O tamanho do buffer é QTD_LEDS,
    // que é o número total de LEDs na matriz.
    uint32_t buffer_led[QTD_LEDS];

    // Inicializa todos os LEDs com a cor "apagada" (preto).
    for (int i = 0; i < QTD_LEDS; i++)
    {
        buffer_led[i] = cor_apagada;
    }

    // Percorre as 5 linhas da matriz de LEDs.
    for (int linha = 0; linha < 5; linha++)
    {
        // Percorre as 5 colunas da matriz de LEDs.
        for (int coluna = 0; coluna < 5; coluna++)
        {
            // Calcula o índice lógico do LED na matriz de 5x5.
            int indice_logico = linha * 5 + coluna;

            // Verifica se o valor para o LED na posição [linha][coluna] na matriz 'numeros_matriz' é 1,
            // o que significa que o LED deve ser aceso.
            if (numeros_matriz[numero][indice_logico] == 1)
            {
                // Mapeia o índice lógico para o índice físico usando a tabela 'mapeamento_led'.
                int indice_fisico = mapeamento_led[linha][coluna];

                // Atualiza o valor no 'buffer_led' para acender o LED na posição mapeada.
                buffer_led[indice_fisico] = cor_acesa;
            }
        }
    }

    // Envia os valores de 'buffer_led' para o hardware de LED, acendendo ou apagando os LEDs.
    for (int i = 0; i < QTD_LEDS; i++)
    {
        // Envia cada valor do buffer para a máquina de estado de PIO (pico I/O) para atualização do display.
        pio_sm_put_blocking(pio, unused_sm, buffer_led[i]);
    }
}


// realiza o debounce
// só atualiza o estado do botão, caso o botão permaneça pressionado pelo tempo mínimo
bool debounce_bt(uint pino, absolute_time_t *ultimo_tempo)
{
    absolute_time_t agora = get_absolute_time();
    if (absolute_time_diff_us(*ultimo_tempo, agora) >= 200000)
    {
        *ultimo_tempo = agora;
        return (gpio_get(pino) == 0);
    }
    return false;
}

void gpio_irq_handler(uint gpio, uint32_t events) // callback da interrupção
{

    if (events & GPIO_IRQ_EDGE_FALL) // borda de decida
    {

        if (gpio == BOTAO_A)
        {
            if (debounce_bt(BOTAO_A, &debounce_A))
            {
                if (numero < 9)
                {
                    numero = (numero + 1); // incrementa o botao A, caso tenha sido pressionado e seja menor que 9
                }
            }
        }

        else if (gpio == BOTAO_B)
        {
            if (debounce_bt(BOTAO_B, &debounce_B))
            {
                if (numero > 0)
                {
                    numero = (numero - 1); // decrementa o botao B, caso tenha sido pressionado e seja menor que 9
                }
            }
        }
    }
}

bool pisca_led_vermelho(struct repeating_timer *t) // callback do timer de repetição. inverte o estado do led a cada chamada.
{
    gpio_put(LED_VERM, !gpio_get(LED_VERM));
    return true;
}

int main()
{
    stdio_init_all();
    initialize_config();

    debounce_A = get_absolute_time();
    debounce_B = get_absolute_time();

    struct repeating_timer timer;
    add_repeating_timer_ms(200, pisca_led_vermelho, NULL, &timer); // inicializa e configura a repetição do timer pra piscar o led vermelho a cada 200ms

    while (true)
    {
        atualizar_matriz_led(); // atualiza a matriz de leds
        sleep_ms(100);
    }
}
