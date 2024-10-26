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

#define WIFI_SSID "DIGITO-FAMILIA DICK"
#define WIFI_PASS "0813191201"
#define STATIC_IP      "192.168.0.50"   // IP fixo desejado para a ESP32
#define GATEWAY_ADDR   "192.168.0.1"    // Gateway (IP do roteador)
#define NETMASK_ADDR   "255.255.255.0"  // Máscara de sub-rede

volatile int sensorvalue = 0;

static const char* htmlPage = "<!DOCTYPE html><html><head><title>ESP32 Controle</title></head><body>"
                              "<h1>Dados do Sensor</h1><p id='sensorData'>Aguardando dados...</p>"
                              "<button onclick='sendCommand()'>Enviar Comando</button>"
                              "<script>"
                              "function fetchData() { fetch('/data').then(response => response.text()).then(data => document.getElementById('sensorData').innerText = data); }"
                              "function sendCommand() { fetch('/command').then(response => response.text()); }"
                              "setInterval(fetchData, 1000);"
                              "</script></body></html>";


static void wifi_init(void) {
    // Inicialize a rede e configure o modo station
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

    // Configurar o IP fixo
    esp_netif_ip_info_t ip_info;
    inet_aton(STATIC_IP, &ip_info.ip);           // Converte o IP fixo para formato binário
    inet_aton(GATEWAY_ADDR, &ip_info.gw);        // Converte o gateway para formato binário
    inet_aton(NETMASK_ADDR, &ip_info.netmask);   // Converte a máscara para formato binário
    esp_netif_dhcpc_stop(netif);                          // Desativa o DHCP
    esp_netif_set_ip_info(netif, &ip_info);               // Define as configurações de IP

    // Configuração do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

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
    char sensor_data[32];
    //int value = gpiostate;  // Substitua pela leitura do sensor
    snprintf(sensor_data, sizeof(sensor_data), "%d", sensorvalue);
    httpd_resp_send(req, sensor_data, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t command_get_handler(httpd_req_t *req) {
    // Exemplo: Toggle em um GPIO
    sensorvalue = 0;
    httpd_resp_send(req, "Comando recebido", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
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
    
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    
    while(1){
		
		if(sensorvalue >= 255){
			sensorvalue = 0;
		}
		sensorvalue++;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		
	}
}

