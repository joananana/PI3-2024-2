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


#define WIFI_SSID "DIGITO-FAMILIA DICK"	// SSID da rede
#define WIFI_PASS "0813191201"			// Senha da rede
#define STATIC_IP      "192.168.0.50"   // IP fixo desejado para a ESP32
#define GATEWAY_ADDR   "192.168.0.1"    // Gateway (IP do roteador)
#define NETMASK_ADDR   "255.255.255.0"  // Máscara de sub-rede

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_NUM_12) | (1ULL<<GPIO_NUM_13))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_NUM_14) | (1ULL<<GPIO_NUM_27))

volatile int led_12_state = 0;
volatile int led_13_state = 0;


static const char* htmlPage = "<!DOCTYPE html>"
							"<html>"
							"<head>"
							"<title>ESP32 Controle</title>"
							"</head>"
							"<body>"
							"<h1>Dados dos Sensores</h1>"
							"<!-- Elementos para mostrar dados dos sensores -->"
							"<p id='tempSensorData'>Temperatura: Aguardando dados...</p>"
							"<p id='flowSensorData'>Fluxo: Aguardando dados...</p> "
							"<!-- Botão para enviar comandos -->"
							"<button onclick='sendCommand1()'>Enviar Comando 1</button>"
							"<button onclick='sendCommand2()'>Enviar Comando 2</button>"
							"<script>"
							"function fetchTempData() {fetch('/data?tipo=temperatura').then(response => response.text()).then(data => document.getElementById('tempSensorData').innerText = 'Temperatura: ' + data);}"
							"function fetchFlowData() {fetch('/data?tipo=fluxo').then(response => response.text()).then(data => document.getElementById('flowSensorData').innerText = 'Fluxo: ' + data);}"
							"function sendCommand1() {fetch('/command?tipo=command1').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"function sendCommand2() {fetch('/command?tipo=command2').then(response => response.text()).then(data => console.log('Comando enviado:', data));}"
							"setInterval(fetchTempData, 1000);"
							"setInterval(fetchFlowData, 1000);"
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
    char param[32];
    char sensor_data[32];
    //int value = gpiostate;  // Substitua pela leitura do sensor
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        if (strstr(param, "tipo=temperatura")) {
            // Código para leitura do sensor de temperatura
            snprintf(sensor_data, sizeof(sensor_data), "%d", gpio_get_level(GPIO_NUM_14));
        } else if (strstr(param, "tipo=fluxo")) {
            // Código para leitura do sensor de umidade
            snprintf(sensor_data, sizeof(sensor_data), "%d", gpio_get_level(GPIO_NUM_27));
        }
    }
    httpd_resp_send(req, sensor_data, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t command_get_handler(httpd_req_t *req) {
    // Exemplo: Toggle em um GPIO
    char param[32];
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        if (strstr(param, "tipo=command1")) {
            // Código para leitura do sensor de temperatura
            led_12_state = !led_12_state;
            gpio_set_level(GPIO_NUM_12, led_12_state);
        } else if (strstr(param, "tipo=command2")) {
            // Código para leitura do sensor de temperatura
            led_13_state = !led_13_state;
            gpio_set_level(GPIO_NUM_13, led_13_state);
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
   
}

