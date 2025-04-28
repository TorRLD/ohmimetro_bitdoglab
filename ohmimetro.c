/*
 * Por: Heitor Rodrigues Lemos Dias
 *    Ohmímetro com Código de Cores utilizando o ADC da BitDogLab
 *
 * Neste exemplo, utilizamos o ADC do RP2040 para medir a resistência de um resistor
 * desconhecido, identificamos o valor comercial E24 mais próximo (5% de tolerância),
 * e exibimos as cores correspondentes das faixas do resistor.
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <stdbool.h>
 #include "pico/stdlib.h"
 #include "hardware/adc.h"
 #include "hardware/i2c.h"
 #include "hardware/pwm.h"  // Necessário para o buzzer
 #include "pico/bootrom.h"
 #include "pico/time.h"     // Para funções de tempo
 #include "lib/ssd1306.h"
 #include "lib/font.h"
 
 #define I2C_PORT i2c1
 #define I2C_SDA 14
 #define I2C_SCL 15
 #define endereco 0x3C
 #define ADC_PIN 28 // GPIO para o voltímetro
 #define Botao_A 5  // GPIO para botão A
 #define BUZZER_PIN 10  // GPIO para o buzzer
 
 int R_conhecido = 10000;   // Resistor de 10k ohm
 float R_x = 0.0;           // Resistor desconhecido
 int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
 
 // Valores da série E24 (tolerância 5%)
 const float SERIE_E24[] = {
     1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0,
     3.3, 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1
 };
 #define NUM_E24 24
 
 // Array com os nomes das cores para as faixas
 const char* CORES[] = {
     "PRETO", "MARROM", "VERMELHO", "LARANJA", "AMARELO",
     "VERDE", "AZUL", "VIOLETA", "CINZA", "BRANCO"
 };
 
 // Funções protótipo
 float encontrar_valor_comercial(float resistencia);
 void calcular_faixas_de_cor(float valor, int* primeira, int* segunda, int* multiplicador);
 void formatar_valor_resistencia(float valor, char* buffer);
 void tocar_beep(void);
 
 // Variáveis globais para os botões
 volatile bool botao_a_pressionado = false;
 volatile uint32_t ultimo_tempo_botao = 0;
 const uint32_t TEMPO_DEBOUNCE = 200; // ms
 bool cor = true; // Flag para alternar cores no display
 
 // Função de callback para interrupção dos botões
 void botao_callback(uint gpio, uint32_t events) {
     uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
     
     // Debounce simples
     if (tempo_atual - ultimo_tempo_botao < TEMPO_DEBOUNCE) {
         return;
     }
     
     ultimo_tempo_botao = tempo_atual;
     
     if (gpio == Botao_A) {
         botao_a_pressionado = true;
     }
 }
 
 int main()
 {
   // Inicializa o botão A com interrupção
   gpio_init(Botao_A);
   gpio_set_dir(Botao_A, GPIO_IN);
   gpio_pull_up(Botao_A);
   
   // Configura a interrupção para o botão
   gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &botao_callback);
 
   // Inicializa o buzzer
   gpio_init(BUZZER_PIN);
   gpio_set_dir(BUZZER_PIN, GPIO_OUT);
   gpio_put(BUZZER_PIN, 0);
 
   // I2C Initialisation. Using it at 400Khz.
   i2c_init(I2C_PORT, 400 * 1000);
 
   gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
   gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
   gpio_pull_up(I2C_SDA);                                        // Pull up the data line
   gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
   ssd1306_t ssd;                                                // Inicializa a estrutura do display
   ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
   ssd1306_config(&ssd);                                         // Configura o display
   ssd1306_send_data(&ssd);                                      // Envia os dados para o display
 
   // Limpa o display. O display inicia com todos os pixels apagados.
   ssd1306_fill(&ssd, false);
   ssd1306_send_data(&ssd);
 
   adc_init();
   adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica
 
   // Variáveis para armazenar valores e strings
   float media = 0.0;
   float valor_comercial = 0.0;
   int primeira_faixa = 0;
   int segunda_faixa = 0;
   int multiplicador = 0;
   char str_adc[8];
   char str_rx[16];
   char str_comercial[16];
   char str_cores[32];
   
   // Tela de boas-vindas
   ssd1306_fill(&ssd, false);
   ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);
   ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);
   ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);
   ssd1306_draw_string(&ssd, "OhmiColorMeter", 10, 28);
   ssd1306_draw_string(&ssd, "Pressione botao A", 15, 45);
   ssd1306_send_data(&ssd);
   
   // Emite um beep inicial
   tocar_beep();
   
   sleep_ms(2000);
 
   while (true)
   {
     // Verifica se o botão foi pressionado via interrupção
     if (botao_a_pressionado) {
       // Toca beep para indicar início da medição
       tocar_beep();
       botao_a_pressionado = false; // Reseta o flag
     }
     
     adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica
 
     float soma = 0.0f;
     for (int i = 0; i < 500; i++)
     {
       soma += adc_read();
       sleep_ms(1);
     }
     media = soma / 500.0f;
 
     // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
     R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
 
     // Encontra o valor comercial mais próximo da série E24
     valor_comercial = encontrar_valor_comercial(R_x);
     
     // Calcula as cores das faixas
     calcular_faixas_de_cor(valor_comercial, &primeira_faixa, &segunda_faixa, &multiplicador);
 
     // Prepara as strings para exibição
     sprintf(str_adc, "%04.0f", media); // Mostra o valor ADC com 4 dígitos
     formatar_valor_resistencia(R_x, str_rx); // Formata a resistência medida
     formatar_valor_resistencia(valor_comercial, str_comercial); // Formata o valor comercial
     
     // Cores formatadas com iniciais para economia de espaço
     char cor1[10], cor2[10], cor3[10];
     // Abreviações/iniciais das cores para economizar espaço
     switch(primeira_faixa) {
       case 0: strcpy(cor1, "PR"); break; // Preto
       case 1: strcpy(cor1, "MR"); break; // Marrom
       case 2: strcpy(cor1, "VM"); break; // Vermelho
       case 3: strcpy(cor1, "LR"); break; // Laranja
       case 4: strcpy(cor1, "AM"); break; // Amarelo
       case 5: strcpy(cor1, "VD"); break; // Verde
       case 6: strcpy(cor1, "AZ"); break; // Azul
       case 7: strcpy(cor1, "VL"); break; // Violeta
       case 8: strcpy(cor1, "CZ"); break; // Cinza
       case 9: strcpy(cor1, "BR"); break; // Branco
       default: strcpy(cor1, "--"); break;
     }
 
     switch(segunda_faixa) {
       case 0: strcpy(cor2, "PR"); break;
       case 1: strcpy(cor2, "MR"); break;
       case 2: strcpy(cor2, "VM"); break;
       case 3: strcpy(cor2, "LR"); break;
       case 4: strcpy(cor2, "AM"); break;
       case 5: strcpy(cor2, "VD"); break;
       case 6: strcpy(cor2, "AZ"); break;
       case 7: strcpy(cor2, "VL"); break;
       case 8: strcpy(cor2, "CZ"); break;
       case 9: strcpy(cor2, "BR"); break;
       default: strcpy(cor2, "--"); break;
     }
 
     switch(multiplicador) {
       case 0: strcpy(cor3, "PR"); break;
       case 1: strcpy(cor3, "MR"); break;
       case 2: strcpy(cor3, "VM"); break;
       case 3: strcpy(cor3, "LR"); break;
       case 4: strcpy(cor3, "AM"); break;
       case 5: strcpy(cor3, "VD"); break;
       case 6: strcpy(cor3, "AZ"); break;
       case 7: strcpy(cor3, "VL"); break;
       case 8: strcpy(cor3, "CZ"); break;
       case 9: strcpy(cor3, "BR"); break;
       default: strcpy(cor3, "--"); break;
     }
     
     sprintf(str_cores, "%s-%s-%s", cor1, cor2, cor3);
 
     // Atualiza o conteúdo do display
     ssd1306_fill(&ssd, false);
     ssd1306_rect(&ssd, 3, 3, 122, 60, cor, false);  // Borda externa
     
     // Título centralizado
     ssd1306_draw_string(&ssd, "OhmiColorMeter", 15, 10);
     
     // Área de informações de resistência
     ssd1306_draw_string(&ssd, "Medido", 5, 25);
     ssd1306_draw_string(&ssd, str_rx, 55, 25);
     ssd1306_draw_string(&ssd, "Comerc", 5, 35);
     ssd1306_draw_string(&ssd, str_comercial, 55, 35);
     
     // Adiciona o valor ADC bruto para diagnóstico
     ssd1306_draw_string(&ssd, "ADC", 5, 45);
     ssd1306_draw_string(&ssd, str_adc, 55, 45);
     
     // Área de código de cores
     ssd1306_draw_string(&ssd, "Cores:", 5, 55);
     ssd1306_draw_string(&ssd, str_cores, 55, 55);
     
     // Desenha um resistor simples com as faixas coloridas
     int pos_x = 80;
     int pos_y = 55;
     int tam_x = 10;
     int tam_y = 6;
     
     ssd1306_send_data(&ssd);  // Atualiza o display
     sleep_ms(700);
   }
 }
 
 // Toca um beep simples
 void tocar_beep(void) {
     // Método simplificado: toggle rápido para gerar som
     for (int i = 0; i < 250; i++) {
         gpio_put(BUZZER_PIN, 1);
         sleep_us(500);
         gpio_put(BUZZER_PIN, 0);
         sleep_us(500);
     }
 }
 
 // Encontra o valor comercial mais próximo na série E24
 float encontrar_valor_comercial(float resistencia) {
     if (resistencia <= 0) {
         return 0.0f;
     }
     
     // Determina a década (potência de 10)
     float log_valor = log10(resistencia);
     int decada = floor(log_valor);
     
     // Normaliza para um valor entre 1.0 e 9.99
     float valor_normalizado = resistencia / pow(10, decada);
     
     // Encontra o valor E24 mais próximo
     float melhor_diferenca = fabs(SERIE_E24[0] - valor_normalizado);
     float melhor_valor = SERIE_E24[0];
     
     for (int i = 1; i < NUM_E24; i++) {
         float diferenca = fabs(SERIE_E24[i] - valor_normalizado);
         if (diferenca < melhor_diferenca) {
             melhor_diferenca = diferenca;
             melhor_valor = SERIE_E24[i];
         }
     }
     
     // Retorna o valor comercial
     return melhor_valor * pow(10, decada);
 }
 
 // Calcula as faixas de cores de um resistor
 void calcular_faixas_de_cor(float valor, int* primeira, int* segunda, int* multiplicador) {
     if (valor <= 0) {
         *primeira = 0;
         *segunda = 0;
         *multiplicador = 0;
         return;
     }
     
     // Converte para notação científica
     char buffer[32];
     sprintf(buffer, "%.10g", valor);
     
     // Para debug
     printf("Valor: %s\n", buffer);
     
     // Remove caracteres que não são dígitos
     char digitos[32] = "";
     int j = 0;
     for (int i = 0; buffer[i] != '\0' && buffer[i] != '.'; i++) {
         if (buffer[i] >= '0' && buffer[i] <= '9') {
             digitos[j++] = buffer[i];
         }
     }
     digitos[j] = '\0';
     
     // Para debug
     printf("Digitos: %s\n", digitos);
     
     // Primeiro e segundo dígitos
     *primeira = (digitos[0] - '0');
     *segunda = (j > 1) ? (digitos[1] - '0') : 0;
     
     // Multiplicador (número de zeros depois dos 2 primeiros dígitos)
     *multiplicador = strlen(digitos) - 2;
     
     // Para debug
     printf("Cores: %d, %d, %d\n", *primeira, *segunda, *multiplicador);
 }
 
 // Formata o valor da resistência para exibição
 void formatar_valor_resistencia(float valor, char* buffer) {
     if (valor <= 0) {
         strcpy(buffer, "-----");
         return;
     }
     
     if (valor >= 1000000) {
         sprintf(buffer, "%.2fM", valor / 1000000.0f);
     } else if (valor >= 1000) {
         sprintf(buffer, "%.2fk", valor / 1000.0f);
     } else {
         sprintf(buffer, "%.1f", valor);
     }
     
     strcat(buffer, "\xF4"); // Símbolo de ohm no font.h
 }