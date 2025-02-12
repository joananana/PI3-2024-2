# Apresentação do Firmware

Este firmware foi desenvolvido com o propósito de controlar a bancada FESTO MPA Compact Workstation à partir de um ponto de acesso Wi-fi.

A ESP32 foi configurada para o modo Access Point para a conexão via wi-fi visando facilitar o conexão do usuário. Basta saber o SSID e a senha padrão configurados e você tem o controle da bancada à partir do seu celular ou notebook.

Para utilizar o wi-fi da ESP32, é necessário as bibliotecas:

```C++
#include "esp_wifi.h"
#include "esp_netif.h"	
```

A partir da função **wifi_init(void);** podemos verificar a configuração da rede. 

Essa configuração inclui alguns passos que são padrão para qualquer modo wi-fi da ESP32, por exemplo, criar um loop de eventos padrão e a inicialização da pilha TCP/IP:

```C++
// Inicializa a rede
    esp_netif_init();
    esp_event_loop_create_default();
```
Após isso, usamos uma estrutura para configurar os parâmetros do nosso access point:

```C++
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
```
E por fim, inicializamos o módulo wi-fi. Aqui usamos os métodos padrões fornecidos pela API:

```C++
    // Inicializa a configuração do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);  // Define o modo Access Point

    // Define a configuração do AP
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();
```

Após a configuração do Access Point, criamos um servidor via HTTP para atender às requisições da interface. Dessa forma, poderíamos comunicar com o dispositivo conectado, enviando as informações dos sensores e recebendo os comandos para acionamento dos atuadores.

Para isso, utilizamos a biblioteca:

```C++
#include "esp_http_server.h"
```

Para inicializar o servidor também utilizamos os modos padrões fornecidos pela API:

```C++
	// Configuração do Webserver 
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
```

E então registramos os **handlers** necessários para atender os diferentes tipos de requisições para nossa interface:

```C++
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
    }
```

Os três handlers iniciais tem como objetivo servir ao dispositivo conectado, à partir de um web browser, os arquivos necessários para gerar a página de controle da bancada FESTO.

Após isso, registramos os handlers para cuidar das requisições de acionamento dos atuadores e leitura dos sensores:

```C++
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
```

Essas requisições são feitas à partir da página Web, onde:

* Requisições do tipo "/data" são feitas de tempos em tempos solicitado os valores dos sensores;
* Requisições do tipo "/command" são feitas ao clicar os botões para acionamento dos atuadores binários (liga/desliga);
* Requisições do tipo "/dac" são especificamente para controlar o nível de tensão gerado na saída dos conversores digitais-analógicos da ESP32 para controle da válvula proporcional e da bomba hidráulica presentes na bancada.

Para podermos armazenar os arquivos da interface, que são utilizados para criar a página web de controle, precisamos criar uma partição na flash da ESP32. Para isso, utilizamos a biblioteca:

```C++
#include "esp_spiffs.h"
```

A partição é criada à partir da função **spiffs_init(void)** onde registramos o caminho base de nossa partição e alguns outros parâmetros:

```C++
	esp_vfs_spiffs_conf_t fs_config = {
		.base_path = "/storage",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true,
	};
	
	esp_err_t result = esp_vfs_spiffs_register(&fs_config);
```
Também precisamos criar uma _partition table_:

```
# ESP-IDF Partition Table
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 1M,
storage, data, spiffs, 0x110000, 500k,
```
Configurar, à partir do **sdkconfig** para utilizar uma _custom partition table_  e executar o comando para criar a imagem da nossa partição dentro do arquivo **CMakeLists.txt**:

```
spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)
```

Dessa forma, temos os arquivos index.html, style.css e script.js armazenados dentro da flash do microcontrolador. Estes arquivos são acessados posteriormente pelos respectivos caminhos:

* "/storage/index.html"
* "/storage/style.css"
* "/storage/script.js"

Após essas configurações, também fizemos a configuração dos periféricos que precisaríamos para controlar todos os sensores e atuadores. A comunicação entre ESP e a bancada foi feita ponto a ponto. Utilizamos pinos de GPIO, ADC e DAC para comunicar de forma adequada com cada sensor e atuador.

Desabilitamos os pinos de pullup e pulldown, pois implementamos nosso próprio pulldown em hardware.
```C++
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
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
		gpio_config(&input_config);
}
```
Aqui, os pinos estão apresentados conforme a nomenclatura de seus respectivos sensores e atuadores.

Utilizamos os dois canais do DAC disponível no ESP32 WROOM, localizados nos pinos 25 e 26 do kit de desenvolvimento. Cada canal possui uma resolução de 8 bits (0 a 255), variando a tensão de saída entre 0 e 3.3V. 

O modo oneshot foi implementado, onde a alteração do DAC só é feita à partir da requisição do webserver.

```C++
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
```

Utilizamos os canais 4, 5, 6 e 7 do ADC1 presente na ESP32 no modo Oneshot. Onde cada leitura do ADC era feita à partir da requisição gerada pelo webserver. 

Cada canal do ADC possui uma resolução de 12 bits (0 a 4096) para leitura de tensão entre 0 e 3.3V.

```C++
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
```

## Possíveis melhorias

O código está bem funcional e permite a comunicação entre a bancada e a interface web. No entanto, podem ser implementadas novas funcionalidades:

### Aprimorar a interface web para melhor experiência do usuário:

* Tornar a interface web mais intuitiva para o usuário;
* Melhorar a responsividade em diferentes dispositivos;
* Implementar gráficos das leituras dos sensores.

### Implementação de sistemas de controle de malha fechada para automação:

A bancada possui quatro possíveis circuitos de malha fechada detalhados em seu manual que podem ser usados individualmente ou em cascata:

* Sistema de controle de nível da água nos tanques;
* Sistema de controle do fluxo de água;
* Sistema de controle de pressão;
* Sistema de controle de temperatura.

Implementar controladores PID para cada um destes sistemas aumentaria a eficiência e usabilidade da bancada. Seria ainda mais interessante permitir a variação dos parâmetros dos controladores à partir do usuário, visto que a bancada é para uso didático e aprendizado de sistemas de automação e controle.