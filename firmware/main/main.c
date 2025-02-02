#include <stdio.h>			// Bibibliotecas gerais
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "esp_err.h"		
#include "esp_spiffs.h"			// API do file system
#include "esp_log.h"			// API p/ mensagens de erro
#include "esp_netif.h"			// Bibliotecas p/ configuração wifi
#include "esp_wifi.h"
#include "hal/adc_types.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "lwip/inet.h"
#include "esp_http_server.h"	// Bibliotecas p/ HTTP server
#include "freertos/FreeRTOS.h"	// Bibliotecas gerais do FreeRTOS
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
#include "esp_adc/adc_oneshot.h"

// Definições gerais para uso do moto Station
//#define 	WIFI_SSID 		"DIGITO-FAMILIA DICK"	// SSID da rede
//#define 	WIFI_PASS 		"0813191201"			// Senha da rede
//#define 	STATIC_IP      	"192.168.0.50"   		// IP fixo desejado para a ESP32
//#define 	GATEWAY_ADDR   	"192.168.0.1"    		// Gateway (IP do roteador)
//#define 	NETMASK_ADDR   	"255.255.255.0"  		// Máscara de sub-rede

//Definiçoes dos pinos

//Saidas binarias
#define 	M102			GPIO_NUM_15
#define 	E104			GPIO_NUM_27
#define 	K1				GPIO_NUM_14
#define 	M1				GPIO_NUM_12
#define 	M106			GPIO_NUM_13

//Entradas binarias
#define		B102			GPIO_NUM_2
#define		S111			GPIO_NUM_4
#define 	S112			GPIO_NUM_5
#define 	B113			GPIO_NUM_18
#define 	B114			GPIO_NUM_19
#define 	S115			GPIO_NUM_22
#define 	S116			GPIO_NUM_23

//Saidas analogicas
#define 	P101			GPIO_NUM_26
#define 	V106			GPIO_NUM_25

//Entradas analogicas
#define 	LIC_B101		ADC_CHANNEL_4
#define 	FIC_B102		ADC_CHANNEL_5
#define 	PIC_B103		ADC_CHANNEL_7
#define 	TIC_B104		ADC_CHANNEL_6

static const char* TAG = "PI3";


// Handles dos canais do DAC
dac_oneshot_handle_t V106_dac_handle;
dac_oneshot_handle_t P101_dac_handle;

// Handles dos canais do ADC
adc_oneshot_unit_handle_t adc1_handle;

// Variaveis globais
static int M102_status = 0;
static int E104_status = 0;
static int K1_status = 0;
static int M1_status = 0;
static int M106_status = 0;

void spiffs_init(void){
	esp_vfs_spiffs_conf_t fs_config = {
		.base_path = "/storage",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true,
	};
	
	esp_err_t result = esp_vfs_spiffs_register(&fs_config);
	
	if(result != ESP_OK){
		ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(result));
	}
	
	size_t total = 0, used = 0;
	result = esp_spiffs_info(fs_config.partition_label, &total, &used);
	if(result != ESP_OK){
		ESP_LOGE(TAG, "Failed to get partition info (%s)", esp_err_to_name(result));
	}
	else{
		ESP_LOGI(TAG, "Partition size: total = %d, used = %d", total, used);
	}
}

//static void wifi_init(void) {
//    // Inicialize a rede e configure o modo station
//    esp_netif_init();
//    esp_event_loop_create_default();
//    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
//
//    // Configurar o IP fixo
//    esp_netif_ip_info_t ip_info;
//    inet_aton(STATIC_IP, &ip_info.ip);           				// Converte o IP fixo para formato binário
//    inet_aton(GATEWAY_ADDR, &ip_info.gw);       					// Converte o gateway para formato binário
//    inet_aton(NETMASK_ADDR, &ip_info.netmask);   				// Converte a máscara para formato binário
//    esp_netif_dhcpc_stop(netif);                          // Desativa o DHCP
//    esp_netif_set_ip_info(netif, &ip_info);       // Define as configurações de IP
//
//    // Configuração do Wi-Fi
//    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//    esp_wifi_init(&cfg);
//    esp_wifi_set_mode(WIFI_MODE_STA);							// Modo "Station"
//
//    wifi_config_t wifi_config = {
//        .sta = {
//            .ssid = WIFI_SSID,
//            .password = WIFI_PASS
//        },
//    };
//    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
//    esp_wifi_start();
//    esp_wifi_connect();
//}

static void wifi_init(void) {
    // Inicializa a rede e configura o modo Access Point
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();

    // Configuração do AP (Ponto de Acesso)
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_AP",           // Nome da rede Wi-Fi
            .password = "senha123",        // Senha do ponto de acesso
            .ssid_len = strlen("ESP32_AP"),
            .channel = 1,                 // Canal Wi-Fi
            .authmode = WIFI_AUTH_WPA2_PSK, // Tipo de autenticação
            .max_connection = 4,          // Número máximo de conexões
            .beacon_interval = 100        // Intervalo de Beacon
        },
    };
    
    esp_netif_dhcpc_stop(netif);                          // Desativa o DHCP

    // Inicializa a configuração do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);  // Define o modo Access Point

    // Define a configuração do AP
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "AP Configurado com SSID: ESP32_AP");
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/index.html", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/index.html for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html"); // Define o tipo de conteúdo
    char buf[129]; // Buffer para leitura
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0'; // Adiciona o terminador de string
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}

static esp_err_t style_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/style.css", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/style.css for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/css"); // Define o tipo de conteúdo
    char buf[129];	// Buffer para leitura
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0'; // Adiciona o terminador de string
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}

static esp_err_t script_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/script.js", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/script.js for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/javascript"); // Define o tipo de conteúdo
    char buf[129];
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0';
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}


static esp_err_t data_get_handler(httpd_req_t *req) {
    char param[32]; // Variável para armazenar a query string
    char sensor[32]; // Variável para armazenar o valor de "tipo"
    char sensor_data[32];
    int adc_val;

    // Obtém a query string da requisição
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        ESP_LOGI(TAG, "Query string recebida: %s", param);

        // Procura o parâmetro "tipo" e armazena o valor em "command"
        if (httpd_query_key_value(param, "tipo", sensor, sizeof(sensor)) == ESP_OK) {
            ESP_LOGI(TAG, "Sensor requisitado: %s", sensor);

            // Processa o comando recebido
            if (strcmp(sensor, "sensorDig1") == 0) {
                ESP_LOGI(TAG, "Sensor digital 1 lido");
                
                if(gpio_get_level(B102) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}

            } else if (strcmp(sensor, "sensorDig2") == 0) {
                // Código para comando 2
                ESP_LOGI(TAG, "Sensor digital 2 lido");

                if(gpio_get_level(S111) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
                
            } else if (strcmp(sensor, "sensorDig3") == 0) {
                // Código para comando 3
                ESP_LOGI(TAG, "Sensor digital 3 lido");
                
                if(gpio_get_level(S112) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
				
            } else if (strcmp(sensor, "sensorDig4") == 0) {
                // Código para comando 4
                ESP_LOGI(TAG, "Sensor digital 4 lido");
                
                if(gpio_get_level(B113) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
				
            } else if (strcmp(sensor, "sensorDig5") == 0) {
                // Código para comando 5
                ESP_LOGI(TAG, "Sensor digital 5 lido");
                
               	if(gpio_get_level(B114) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
			
            } else if (strcmp(sensor, "sensorDig6") == 0) {
                // Código para comando 6
                ESP_LOGI(TAG, "Sensor digital 6 lido");
                
               	if(gpio_get_level(S115) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
			
            } else if (strcmp(sensor, "sensorDig7") == 0) {
                // Código para comando 5
                ESP_LOGI(TAG, "Sensor digital 7 lido");
                
               	if(gpio_get_level(S116) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
			
            } else if(strcmp(sensor, "sensorAnalog1") == 0) {
				
				ESP_LOGI(TAG, "Sensor analógico 1 lido");
				
				adc_oneshot_read(adc1_handle, LIC_B101, &adc_val);
				snprintf(sensor_data, sizeof(sensor_data), "%d", adc_val);
			} else if(strcmp(sensor, "sensorAnalog2") == 0) {
				
				ESP_LOGI(TAG, "Sensor analógico 2 lido");
				
				adc_oneshot_read(adc1_handle, FIC_B102, &adc_val);
				snprintf(sensor_data, sizeof(sensor_data), "%d", adc_val);
			} else if(strcmp(sensor, "sensorAnalog3") == 0) {
				
				ESP_LOGI(TAG, "Sensor analógico 3 lido");
				
				adc_oneshot_read(adc1_handle, PIC_B103, &adc_val);
				snprintf(sensor_data, sizeof(sensor_data), "%d", adc_val);
			} else if(strcmp(sensor, "sensorAnalog4") == 0) {
				
				ESP_LOGI(TAG, "Sensor analógico 4 lido");
				
				adc_oneshot_read(adc1_handle, TIC_B104, &adc_val);
				snprintf(sensor_data, sizeof(sensor_data), "%d", adc_val);
			} else {
                ESP_LOGW(TAG, "Comando não reconhecido: %s", sensor);
            }
        } else {
            ESP_LOGE(TAG, "Parâmetro 'tipo' não encontrado na query string");
        }
    } else {
        ESP_LOGE(TAG, "Erro ao obter a query string da requisição");
    }
	
	httpd_resp_send(req, sensor_data, HTTPD_RESP_USE_STRLEN);
	
    return ESP_OK;
}

static esp_err_t command_get_handler(httpd_req_t *req) {
    char param[32]; // Variável para armazenar a query string
    char command[32]; // Variável para armazenar o valor de "tipo"

    // Obtém a query string da requisição
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        ESP_LOGI(TAG, "Query string recebida: %s", param);

        // Procura o parâmetro "tipo" e armazena o valor em "command"
        if (httpd_query_key_value(param, "tipo", command, sizeof(command)) == ESP_OK) {
            ESP_LOGI(TAG, "Comando recebido: %s", command);
            // Processa o comando recebido
            if (strcmp(command, "command1") == 0) {
                
                ESP_LOGI(TAG, "Comando 1 executado");
                
                M102_status = !M102_status;
            	gpio_set_level(M102, M102_status);
                
            } else if (strcmp(command, "command2") == 0) {
                // Código para comando 2
                ESP_LOGI(TAG, "Comando 2 executado");
                
                E104_status = !E104_status;
                gpio_set_level(E104, E104_status);
                
            } else if (strcmp(command, "command3") == 0) {
                // Código para comando 3
                ESP_LOGI(TAG, "Comando 3 executado");
                
                K1_status = !K1_status;
                gpio_set_level(K1, K1_status);
                
            } else if (strcmp(command, "command4") == 0) {
                // Código para comando 4
                ESP_LOGI(TAG, "Comando 4 executado");
                
                M1_status = !M1_status;
                gpio_set_level(M1, M1_status);
                
            } else if (strcmp(command, "command5") == 0) {
                // Código para comando 5
                ESP_LOGI(TAG, "Comando 5 executado");
                
                M106_status = !M106_status;
                gpio_set_level(M106, M106_status);
            } else {
                ESP_LOGW(TAG, "Comando não reconhecido: %s", command);
            }
        } else {
            ESP_LOGE(TAG, "Parâmetro 'tipo' não encontrado na query string");
        }
    } else {
        ESP_LOGE(TAG, "Erro ao obter a query string da requisição");
    }

    // Responde ao cliente
    httpd_resp_send(req, "Comando recebido", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static esp_err_t dac_get_handler(httpd_req_t *req) {
    char param[64]; // Buffer para armazenar a string da query
    char canal_str[8], valor_str[8]; // Buffers para armazenar os valores extraídos
    int valor;
    // Obtém a string de query da URL
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        
        ESP_LOGI(TAG, "Query string recebida: %s", param);
        // Extrai o valor do canal do DAC
        if (httpd_query_key_value(param, "tipo", canal_str, sizeof(canal_str)) == ESP_OK) {
			
			ESP_LOGI(TAG, "Key encontrada: %s", canal_str);
			
			// Extrai o valor do DAC
        	if (httpd_query_key_value(param, "value", valor_str, sizeof(valor_str)) == ESP_OK) {
            	valor = atoi(valor_str); // Converte string para inteiro
            	
            	        
        		ESP_LOGI("DAC_HANDLER", "Canal: %s, Valor: %d", canal_str, valor);
        
        		// Aqui você pode configurar o DAC do ESP32 com o valor recebido
        		if (strcmp(canal_str, "dac1")) {
         			dac_oneshot_output_voltage(P101_dac_handle, valor);   
     			} else if (strcmp(canal_str, "dac2")) {
        			dac_oneshot_output_voltage(V106_dac_handle, valor);	
        		} 
        	}

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
  
        httpd_uri_t css_uri = {
    		.uri = "/style.css",
    		.method = HTTP_GET,
    		.handler = style_get_handler,
    		.user_ctx = NULL
		};
		httpd_register_uri_handler(server, &css_uri);

		httpd_uri_t js_uri = {
    		.uri = "/script.js",
    		.method = HTTP_GET,
    		.handler = script_get_handler,
    		.user_ctx = NULL
		};
		httpd_register_uri_handler(server, &js_uri);
		
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

		httpd_uri_t dac_uri = {
			.uri	   = "/dac",
			.method	   = HTTP_GET,
			.handler   = dac_get_handler
		};
		httpd_register_uri_handler(server, &dac_uri);
    }
    return server;
}

void gpio_init(void){
	
	// Inicialização dos pinos de entrada e saída
	gpio_config_t output_config = {
		.pin_bit_mask = (1<<M102) | (1<<E104) | (1<<K1) | (1<<M1) | (1<<M106),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE 
	};
	gpio_config(&output_config);
	
	gpio_config_t input_config = {
		.pin_bit_mask = (1<<B102) | (1<<S111) | (1<<S112) | (1<<B113) | (1<<B114) | (1<<S115) | (1<<S116), 
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
		gpio_config(&input_config);
}

void dac_init(void){
	
	 /* DAC oneshot init */
    dac_oneshot_config_t V106_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&V106_cfg, &V106_dac_handle));

    dac_oneshot_config_t P101_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&P101_cfg, &P101_dac_handle));
}

void adc_init(void){
	
	// ADC1 Init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LIC_B101, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, FIC_B102, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, PIC_B103, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, TIC_B104, &config)); 
}

void app_main(void)
{
	
	spiffs_init();			// Inicializa o sistema de arquivos
	nvs_flash_init();       // Inicializa a memória não volátil
    wifi_init();            // Inicializa o Wi-Fi
    start_webserver();
	gpio_init();
	dac_init();
	adc_init();
}
