#include "driver/dac.h"
#include "driver/gpio.h" 
#include "driver/adc.h"
#include "esp_adc_cal.h"

// Saida analogica DAC
#define V106 DAC_CHAN_0  // GPIO25
#define P101 DAC_CHAN_1  // GPIO26

// Entrada analogica ADC
#define LIC_B101 ADC1_CHANNEL_4  // GPIO32 para LIC B101
#define FIC_B102 ADC1_CHANNEL_5  // GPIO33 para FIC B102
#define PIC_B103 ADC1_CHANNEL_6  // GPIO34 para PIC B103
#define TIC_B104 ADC1_CHANNEL_7  // GPIO35 para TIC B104

// Entradas digitais
#define B102 GPIO_NUM_17 	// Sensor de vazao B102
#define S111 GPIO_NUM_18 	// Interruptor de nivel tipo boia S111
#define S112 GPIO_NUM_19 	// Interruptor de nivel tipo boia S112
#define B113 GPIO_NUM_21 	// Sensor capacitivo de proximidade B113
#define B114 GPIO_NUM_22 	// Sensor capacitivo de proximidade B114
#define S115 GPIO_NUM_23 	// Micro rele para valvula esferica V102 (S115)
#define S116 GPIO_NUM_24 	// Micro rele para valvula esferica V102 (S116)


// Saidas digitais
#define M102 GPIO_NUM_12  		// Acionamento valvula esferica M102
#define E104 GPIO_NUM_13        // Aquecedor E104
#define K1 GPIO_NUM_14 			// Rele controle analogico da bomba K1
#define M1 GPIO_NUM_15  		// Rele controle binario da bomba M1
#define M106 GPIO_NUM_16 		// Acionamento valvula proporcional M106


void app_main(void)
{
    // Habilitando o DAC nos pinos associados aos dispositivos
    dac_output_enable(V106);  
    dac_output_enable(P101);  


	// Inicializando os canais ADC
    adc1_config_width(ADC_WIDTH_BIT_12);  // Configura a resolução de 12 bits

    // Configurando cada pino como entrada analogica
    adc1_config_channel_atten(LIC_B101, ADC_ATTEN_DB_0);  // Sem atenuação (0 dB) para LIC B101
    adc1_config_channel_atten(FIC_B102, ADC_ATTEN_DB_0);  // Sem atenuação (0 dB) para FIC B102
    adc1_config_channel_atten(PIC_B103, ADC_ATTEN_DB_0);  // Sem atenuação (0 dB) para PIC B103
    adc1_config_channel_atten(TIC_B104, ADC_ATTEN_DB_0);  // Sem atenuação (0 dB) para TIC B104

	// Configurando os pinos como saidas
    gpio_pad_select_gpio(M102);
    gpio_pad_select_gpio(E104);
    gpio_pad_select_gpio(K1);
    gpio_pad_select_gpio(M1);
    gpio_pad_select_gpio(M106);

    gpio_set_direction(M102, GPIO_MODE_OUTPUT);
    gpio_set_direction(E104, GPIO_MODE_OUTPUT);
    gpio_set_direction(K1, GPIO_MODE_OUTPUT);
    gpio_set_direction(M1, GPIO_MODE_OUTPUT);
    gpio_set_direction(M106, GPIO_MODE_OUTPUT);

    while (1) {
        
    }
}
