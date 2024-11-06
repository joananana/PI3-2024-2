#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "soc/gpio_num.h"
#include "lwip/inet.h"
#include "driver/dac_oneshot.h"
#include "esp_adc/adc_oneshot.h"


#define WIFI_SSID "WIFI_SSID"	// SSID da rede
#define WIFI_PASS "WIFI_PASSWORD"			// Senha da rede
#define STATIC_IP      "192.168.0.50"   // IP fixo desejado para a ESP32
#define GATEWAY_ADDR   "192.168.0.1"    // Gateway (IP do roteador)
#define NETMASK_ADDR   "255.255.255.0"  // Máscara de sub-rede

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_NUM_12) | (1ULL<<GPIO_NUM_13)) 	// Máscara pinos de saída digitais
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_NUM_14) | (1ULL<<GPIO_NUM_27))		// Máscara pinos entrada digitais

#define EXAMPLE_DAC_CHAN0_ADC_CHAN          ADC_CHANNEL_6   // GPIO25, DAC channel 0
#define EXAMPLE_DAC_CHAN1_ADC_CHAN          ADC_CHANNEL_7   // GPIO26, DAC channel 1

#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_4			// GPIO33
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_5			// GPIO32
#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_12			// Atenuação ADC

// Variáveis de estado das saídas digitais
volatile int led_12_state = 0;	
volatile int led_13_state = 0;

// Handles dos canais do DAC
dac_oneshot_handle_t chan0_handle;
dac_oneshot_handle_t chan1_handle;

// Handle ADC
adc_oneshot_unit_handle_t adc1_handle;

// Variáveis de estado saídas analógicas
volatile uint8_t val0_dac = 0;
volatile uint8_t val1_dac = 0;

static const char* htmlPage = "<!DOCTYPE html>"
							"<html>"
							"<head>"
							"<title>ESP32 Controle</title>"
							"</head>"
							"<body>"
							"<h1>Dados dos Sensores</h1>"
							"<!-- Elementos para mostrar dados dos sensores -->"
							"<p id='sensorDig1'>Sensor Digital 1: Aguardando dados...</p>"
							"<p id='sensorDig2'>Sensor Digital 2: Aguardando dados...</p> "
							"<p id='sensorAnalog1'>Sensor Analógico 1: Aguardando dados...</p> "
							"<p id='sensorAnalog2'>Sensor Analógico 2: Aguardando dados...</p> "
							"<!-- Botão para enviar comandos -->"
							"<button onclick='sendCommand1()'>Enviar Comando 1</button>"
							"<button onclick='sendCommand2()'>Enviar Comando 2</button>"
							"<button onclick='sendCommand3()'>Enviar Comando 3</button>"
							"<button onclick='sendCommand4()'>Enviar Comando 4</button>"
							"<script>"
							"function fetchSensorDig1() {fetch('/data?tipo=sensorDig1').then(response => response.text()).then(data => document.getElementById('sensorDig1').innerText = 'Sensor Digital 1: ' + data);}"
							"function fetchSensorDig2() {fetch('/data?tipo=sensorDig2').then(response => response.text()).then(data => document.getElementById('sensorDig2').innerText = 'Sensor Digital 2: ' + data);}"
							"function fetchSensorAnalog1() {fetch('/data?tipo=sensorAnalog1').then(response => response.text()).then(data => document.getElementById('sensorAnalog1').innerText = 'Sensor Analogico 1: ' + data);}"
							"function fetchSensorAnalog2() {fetch('/data?tipo=sensorAnalog2').then(response => response.text()).then(data => document.getElementById('sensorAnalog2').innerText = 'Sensor Analogico 2: ' + data);}"
							"function sendCommand1() {fetch('/command?tipo=command1').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"function sendCommand2() {fetch('/command?tipo=command2').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"function sendCommand3() {fetch('/command?tipo=command3').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"function sendCommand4() {fetch('/command?tipo=command4').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"setInterval(fetchSensorDig1, 1000);"
							"setInterval(fetchSensorDig2, 1000);"
							"setInterval(fetchSensorAnalog1, 1000);"
							"setInterval(fetchSensorAnalog2, 1000);"
							"</script>"
							"</body>"
							"</html>";



static void wifi_init(void) {
    // Inicialize a rede e configure o modo station
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

    // Configurar o IP fixo
    esp_netif_ip_info_t ip_info;
    inet_aton(STATIC_IP, &ip_info.ip);           				// Converte o IP fixo para formato binário
    inet_aton(GATEWAY_ADDR, &ip_info.gw);       					// Converte o gateway para formato binário
    inet_aton(NETMASK_ADDR, &ip_info.netmask);   				// Converte a máscara para formato binário
    esp_netif_dhcpc_stop(netif);                          // Desativa o DHCP
    esp_netif_set_ip_info(netif, &ip_info);       // Define as configurações de IP

    // Configuração do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);							// Modo "Station"

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, htmlPage, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_get_handler(httpd_req_t *req) {
    char param[32]; // Variavel de leitura ID da requisição
    char sensor_data[32]; // Variavel de leitura sensor
    int val = 0;
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        if (strstr(param, "tipo=sensorDig1")) {
          	// Leitura pino de entrada sensor 1
            snprintf(sensor_data, sizeof(sensor_data), "%d", gpio_get_level(GPIO_NUM_14));
        } else if (strstr(param, "tipo=sensorDig2")) {
            // Leitura pino de entrada sensor 2
            snprintf(sensor_data, sizeof(sensor_data), "%d", gpio_get_level(GPIO_NUM_27));
        } else if (strstr(param, "tipo=sensorAnalog1")) {
            // Leitura pino de entrada sensor analógico 1
            adc_oneshot_read(adc1_handle,EXAMPLE_ADC1_CHAN0, &val);
            snprintf(sensor_data, sizeof(sensor_data), "%d", val);
        } else if (strstr(param, "tipo=sensorAnalog2")) {
            // Leitura pino de entrada sensor analógico 2
            adc_oneshot_read(adc1_handle,EXAMPLE_ADC1_CHAN1, &val);
            snprintf(sensor_data, sizeof(sensor_data), "%d", val);
        }
    }
    httpd_resp_send(req, sensor_data, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t command_get_handler(httpd_req_t *req) {

    char param[32]; // Variavel de leitura ID da requisição
    
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        if (strstr(param, "tipo=command1")) {
			// Comando 1 - Toggle LED
            led_12_state = !led_12_state;
            gpio_set_level(GPIO_NUM_12, led_12_state);
        } else if (strstr(param, "tipo=command2")) {
			// Comando 2 - Toggle LED
            led_13_state = !led_13_state;
            gpio_set_level(GPIO_NUM_13, led_13_state);
        } else if (strstr(param, "tipo=command3")) {
			// Comando 3 - Incrementa DAC
			(val0_dac < 250) ? (val0_dac += 50) : (val0_dac = 0);            
            dac_oneshot_output_voltage(chan0_handle, val0_dac);
        } else if (strstr(param, "tipo=command4")) {
			// Comando 4 - Incrementa DAC
			(val1_dac < 250) ? (val1_dac += 50) : (val1_dac = 0);            
            dac_oneshot_output_voltage(chan1_handle, val1_dac);
        }
    }
    httpd_resp_send(req, "Comando recebido", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
	// Configuração do Webserver 
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    // Registro de handlers
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t data_uri = {
            .uri       = "/data",
            .method    = HTTP_GET,
            .handler   = data_get_handler
        };
        httpd_register_uri_handler(server, &data_uri);

        httpd_uri_t command_uri = {
            .uri       = "/command",
            .method    = HTTP_GET,
            .handler   = command_get_handler
        };
        httpd_register_uri_handler(server, &command_uri);
    }
    return server;
}

void app_main(void) {
    nvs_flash_init();       // Inicializa a memória não volátil
    wifi_init();            // Inicializa o Wi-Fi
    start_webserver();      // Inicia o servidor HTTP
    
    /*GPIO OUTPUT CONFIG*/
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    
    /*GPIO INPUT CONFIG*/
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    
     /* DAC oneshot init */
    dac_oneshot_config_t chan0_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &chan0_handle));

    dac_oneshot_config_t chan1_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &chan1_handle));

    // ADC1 Init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));
   
}

