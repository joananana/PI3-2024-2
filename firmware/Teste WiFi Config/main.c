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

volatile int tempSensorValue = 0; // Teste inicial
volatile int flowSensorValue = 0; // Teste inicial


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
            snprintf(sensor_data, sizeof(sensor_data), "%d", tempSensorValue);
        } else if (strstr(param, "tipo=fluxo")) {
            // Código para leitura do sensor de umidade
            snprintf(sensor_data, sizeof(sensor_data), "%d", flowSensorValue);
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
            tempSensorValue = 0;
        } else if (strstr(param, "tipo=command2")) {
            // Código para leitura do sensor de umidade
            flowSensorValue = 0;
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
    
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT); // Teste inicial
    
    while(1){
		
		// Contador teste inicial
		if(tempSensorValue >= 255){
			tempSensorValue = 0;
		}
		
		if(flowSensorValue >= 255){
			flowSensorValue = 0;
		}
		tempSensorValue++;
		flowSensorValue++;
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		
	}
}

