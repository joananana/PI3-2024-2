# PI3-2024-2
Repositório destinado ao Projeto Integrador 3 de Engenharia Eletrônica 


Desenvolvimento de uma interface microcontrolada para a bancada MPS PA Compact Workstation

## Introdução sobre a bancada MPS PA Compact Workstation

A **MPS PA Compact Workstation**, desenvolvida pela **Festo**, é uma plataforma de treinamento didático projetada para fins educacionais no campo da automação de processos. É um sistema versátil e compacto que facilita o treinamento industrial ao oferecer experiência prática com vários circuitos de controle, incluindo **nível**, **fluxo**, **pressão** e **temperatura**. Esta estação de trabalho permite que os alunos se envolvam com cenários de automação do mundo real, incluindo sistemas de controle de **circuito aberto** e **fechado**.

Um recurso interessante para o estudo é seu suporte para **controle em cascata**, uma estratégia que aprimora a estabilidade e a precisão do controle, especialmente no gerenciamento de sistemas complexos, como controles de nível e fluxo. A plataforma é equipada com diversos sensores e atuadores, como sensores capacitivos, medidores de fluxo magnético-indutivos e reguladores de pressão, proporcionando um aprendizado prático em automação e controle de processos.

O objetivo do projeto é desenvolver uma interface baseada em **microcontroladores** para a bancada com o intuito de aumentar a **flexibilidade** e a **autonomia** do sistema, permitindo maior integração com sistemas de controle personalizados e facilitando a interação de conhecimentos em sistemas de controle e microcontroladores, tornando a aplicação mais prática e personalizável.

Microcontroladores, como o **STM32** ou **ESP32**, podem ser usados para criar uma interface que se comunique diretamente com os sensores e atuadores da bancada, controlando variáveis como **temperatura**, **pressão** e **fluxo**. Isso possibilita a implementação de **protocolos de comunicação modernos** e o desenvolvimento de **interfaces gráficas** mais acessíveis, melhorando o desempenho e facilitando a integração com outros sistemas industriais ou acadêmicos.

### Características do ESP32 e Justificativa para sua Escolha

O **ESP32** é um microcontrolador altamente integrado, desenvolvido pela Espressif, que se destaca por suas diversas funcionalidades e desempenho em projetos de automação e controle. Uma das principais vantagens é seu excelente custo-benefício. Com um preço acessível, ele oferece uma ampla gama de recursos que competem com microcontroladores de maior custo. Isso o torna ideal para projetos acadêmicos e comerciais, onde a otimização de recursos financeiros é fundamental.

O ESP32 possui um potente processador dual-core que opera a até 240 MHz, proporcionando um desempenho significativo para tarefas complexas. Essa capacidade de processamento é especialmente útil em aplicações que exigem multitarefas, como o controle de diversos sensores e atuadores simultaneamente. Além disso, o ESP32 conta com uma quantidade considerável de memória RAM e flash, permitindo o desenvolvimento de aplicações mais robustas.

Outro ponto crucial para a escolha do ESP32 é sua conectividade Wi-Fi integrada. Essa funcionalidade permite a comunicação sem fio com outros dispositivos e a internet, facilitando a implementação de projetos de IoT (Internet das Coisas). A capacidade de se conectar facilmente a redes Wi-Fi torna o ESP32 uma escolha popular para aplicações que requerem monitoramento e controle remoto, além de possibilitar a troca de dados entre dispositivos.

A Espressif fornece um ambiente de desenvolvimento integrado (IDE) que facilita o aprendizado e a implementação de projetos com o ESP32. Com uma interface intuitiva e uma vasta documentação, o desenvolvimento do sistema pode rapidamente se tornar familiar com a programação e as funcionalidades do microcontrolador.

Diante dessas características, o ESP32 se destaca como uma excelente escolha para projetos que exigem um microcontrolador eficiente e versátil. Sua combinação de custo-benefício, poder de processamento, conectividade Wi-Fi e um ambiente de desenvolvimento amigável torna-o ideal para essa aplicação de automação e controle, além de promover o aprendizado prático em sistemas de IoT.

## Mapeamento dos Sensores e Atuadores presentes na bancada Festo - MPS PA Compact Workstation

## Resumo dos Sensores
- **Sensor de nível ultrassônico** - B101
- **Sensores capacitivo de proximidade** - B113 / B114
- **Interruptores de nível tipo boia** - S111 / S112
- **Sensor de vazão** - B102
- **Sensor de pressão** - B103
- **Sensor de temperatura** - B104

## Resumo dos Atuadores
- **Bomba centrífuga** - P101
- **Válvula proporcional para controle de vazão** - V106
- **Válvula esférica** - V102
- **Aquecedor** - E104

---

## Tabelas de Mapeamento
- **Terminal I/O Digital** - XMA1
- **Terminal I/O Analógico** - X2

---

## Tabela 1 - Sensores

| Descrição                           | Referência Datasheet |
|-------------------------------------|----------------------|
| Sensor de nível ultrassônico         | B101                 |
| Sensores capacitivo de proximidade   | B113 / B114          |
| Interruptores de nível tipo boia     | S111 / S112          |
| Sensor de vazão                      | B102                 |
| Sensor de pressão                    | B103                 |
| Sensor de temperatura                | B104                 |

### Sensor de nível ultrassônico - B101
O sinal de corrente analógico (4 ... 20 mA) do sensor ultrassônico está conectado como um sinal padrão ao terminal analógico X2 (IE1) no canal 0. O sinal de corrente também está conectado ao transformador de medição A1, que converte o sinal de corrente analógico em um sinal de tensão padrão (0 ... 10 V). O sinal de tensão padrão também está conectado ao terminal analógico X2 (UE1).

### Sensores capacitivo de proximidade - B113 / B114
Dois interruptores de proximidade capacitivos, B113 (1) e B114 (2), estão localizados na lateral do tanque inferior B101 e montados em uma placa de perfil. Os interruptores de proximidade podem ser ajustados mecanicamente. A distância de detecção através da parede do tanque pode ser ajustada com um parafuso. Os sinais de entrada binários de 24 V são conectados ao terminal de E/S XMA1.

### Interruptores de nível tipo boia - S111 / S112 / S117
- **S111**: O transbordamento no tanque B101 é monitorado com o interruptor de boia. Se o nível no tanque exceder o nível máximo, o cilindro de boia transparente é empurrado para cima, ativando um contato reed. Os sinais de entrada binários de 24 V (normalmente abertos) são conectados ao terminal de E/S XMA1.
- **S112**: Monitora o nível mínimo no tanque superior B102. Os sinais de entrada binários de 24 V (normalmente fechados) são conectados ao terminal de E/S XMA1.
- **S117**: Monitora a diminuição do nível de enchimento do tanque B101, evitando que o aquecimento continue caso o nível fique abaixo do ponto crítico. O cabo do interruptor está conectado diretamente ao aquecimento.

### Sensor de vazão - B102
Um sinal de onda quadrada constante do sensor de vazão está conectado a uma entrada binária no terminal de E/S XMA1 (I0). O nível do sinal depende da tensão de alimentação aplicada (8...24 V). O sinal de frequência também está conectado ao transformador de medição A2, que converte o sinal em uma tensão padrão de 0 a 10 V.

### Sensor de pressão - B103
O sensor de pressão piezoresistivo que envia um sinal analógico de  a 10V de acordo com a pressão no tanque B103 entre 0 a 400mbar.
### Sensor de temperatura - B104
A resistência do sensor de temperatura está conectada ao transformador de medição A3, que converte a resistência em um sinal de tensão padrão (0 a 10 V), conectado ao terminal analógico X2 (UE4). O aquecimento é controlado por um relé interno, ativado por uma saída digital (O1 no XMA1).

---

## Tabela 2 - Atuadores

| Descrição                                  | Referência Datasheet |
|--------------------------------------------|----------------------|
| Bomba centrífuga                           | P101                 |
| Válvula proporcional para controle de vazão| V106                 |
| Válvula esférica                           | V102                 |
| Aquecedor                                  | E104                 |

### Bomba centrífuga - P101
A bomba é acionada pelo controlador de motor A4 e pelo relé K1. Com uma saída digital (O2 no XMA1), é possível alternar entre controle binário e controle analógico (0 a 24 V). No controle binário, a bomba é ligada/desligada com uma saída adicional (O3 no XMA1). No controle analógico, a tensão de acionamento define a velocidade da bomba (0 a 10 V).

### Válvula proporcional para controle de vazão - V106
A eletrônica de controle da válvula é ativada com uma saída binária (O4 no XMA1). Um sinal analógico do canal 1 (UA2 no X2) aciona a válvula com um sinal padrão de 0 a 10 V.

### Válvula esférica - V102
O acessório de detecção de posição final é composto por dois micro relés elétricos. Os sinais binários de 24 VDC (S115 e S116) estão conectados como entradas no terminal de E/S XMA1.

### Aquecedor - E104
O aquecimento é controlado por um microcontrolador interno, ativado por uma saída binária (Q1 no XMA1). O controle do aquecimento pode ser binário ou contínuo (modulação por largura de pulso - PWM).

---

## Tabelas de Mapeamento

### Terminal I/O Digital

A bancada utiliza um terminal I/O digital, identificado como XMA1, para comunicação entre os sensores de saída binária com sua interface. Este terminal I/O possui uma conexão de 24 pinos padrão IEEE-488 a qual podemos utilizar em nosso benefício para comunicação entre nosso microcontrolador e a bancada.

Dessa forma, utilizando as informações encontradas no datasheet da bancada e seu diagrama elétrico fizemos o mapeamento de cada sensor e sua respectiva posição no terminal. Apresentados nas tabelas abaixo, divididos entre sensores e atuadores.

O terminal I/O digital trabalha com uma tensão de 24V, a qual precisaremos adequar para trabalhar com nosso microcontrolador que trabalha com tensão de 3.3V.

### Tabela 3 - Entradas Digitais XMA1

| Descrição                                | Símbolo | Atribuição de pinos | Referência |
|------------------------------------------|---------|---------------------|------------|
| Sensor de vazão                          | B102    | I0                  | XMA1.13    |
| Interruptor de nível tipo boia           | S111    | I1                  | XMA1.14    |
| Interruptor de nível tipo boia           | S112    | I2                  | XMA1.15    |
| Sensor capacitivo de proximidade         | B113    | I3                  | XMA1.16    |
| Sensor capacitivo de proximidade         | B114    | I4                  | XMA1.17    |
| Micro relé para válvula esférica (V102)  | S115    | I5                  | XMA1.18    |
| Micro relé para válvula esférica (V102)  | S116    | I6                  | XMA1.19    |
| N/A                                      | N/A     | I7                  | XMA1.20    |

### Tabela 4 - Saídas Digitais XMA1

| Descrição                            | Símbolo | Atribuição de pinos | Referência |
|--------------------------------------|---------|---------------------|------------|
| Acionamento válvula esférica          | M102    | O0                  | XMA1.1     |
| Aquecedor                             | E104    | O1                  | XMA1.2     |
| Relé controle analógico da bomba      | K1      | O2                  | XMA1.3     |
| Relé controle binário da bomba        | M1      | O3                  | XMA1.4     |
| Acionamento válvula proporcional      | M106    | O4                  | XMA1.5     |
| N/A                                  | N/A     | O5                  | XMA1.6     |
| N/A                                  | N/A     | O6                  | XMA1.7     |
| N/A                                  | N/A     | O7                  | XMA1.8     |

---

### Terminal I/O Analógico

Os componentes analógicos da bancada também estão conectados a um terminal I/O, identificado como X2, para facilitar a integração e comunicação. Todos os sinais são convertidos para níveis de tensão entre 0 e 10V, os quais teremos que adequar para leitura nos conversores analógico-digitais de nosso microcontrolador.

Este terminal utiliza um conector D-Sub 15 pinos para comunicação com a interface, o qual utilizaremos em nosso benefício para conexão de nossa interface. Para isso, mapeamos os pinos do terminal e suas funções de controle dos componentes da bancada, conforme apresentado abaixo:

### Tabela 5 - Entradas Analógicas X2

| Descrição                       | Símbolo | Atribuição de pinos | Referência |
|----------------------------------|---------|---------------------|------------|
| Sensor de nível ultrassônico     | LIC B101| UE1                 | X2.8       |
| Sensor de vazão                  | FIC B102| UE2                 | X2.7       |
| Sensor de pressão                | PIC B103| UE3                 | X2.15      |
| Sensor de temperatura            | TIC B104| UE4                 | X2.14      |

### Tabela 6 - Saídas Analógicas X2

| Descrição                                  | Símbolo | Atribuição de pinos | Referência |
|--------------------------------------------|---------|---------------------|------------|
| Bomba centrífuga                           | P101    | UA1                 | X2.1       |
| Válvula proporcional para controle de vazão | V106    | UA2                 | X2.2       |

O restante dos pinos são utilizados para alimentação e referência ou não são utilizados.

## Fluxograma do Sistema

O fluxograma apresentado abaixo descreve o processo de funcionamento da interface microcontrolada para a bancada MPS Festo com comunicação via Wi-Fi. Esse sistema visa gerenciar a comunicação e o controle de dispositivos da bancada de forma eficiente e prática.

![Fluxograma](https://github.com/joananana/PI3-2024-2/blob/main/imagens/Fluxograma%20Firmware.drawio.png)



**Início**: O sistema é inicializado. Nesta fase, o ESP32 é ligado e começa a executar o código que controla o processo.


**Conecta Wi-Fi**: O próximo passo é estabelecer uma conexão Wi-Fi. Essa conexão é essencial, pois será utilizada para a comunicação remota entre o microcontrolador e o sistema de controle, permitindo o monitoramento e o envio de comandos para a bancada.


**Inicia Servidor WEB**: Após a conexão com a rede Wi-Fi, o sistema inicia um servidor web. Esse servidor permite que dispositivos externos acessem e controlem a bancada por meio de uma interface web, onde será possível enviar comandos e receber dados sobre as variáveis do processo, como temperatura, pressão e fluxo.


**Configura Periféricos (ADC, Timers, GPIO)**: Nesta etapa, o microcontrolador configura os periféricos necessários, incluindo o conversor analógico-digital (ADC) para leitura de sensores, temporizadores para o controle de tarefas periódicas e os pinos de entrada/saída digital (GPIO) para o controle de atuadores e leitura de sensores digitais.


**Requisição de Dados?**: Uma vez que o sistema está configurado e em operação, ele monitora constantemente se há uma requisição de dados. Esse ponto de decisão verifica se o sistema recebeu uma solicitação de dados de algum dispositivo externo.
Caso não haja uma requisição de dados, o sistema retorna para o estado de espera, aguardando novas solicitações. Se houver uma requisição, o sistema processa o pedido.


**Retorna Dado**: Se uma requisição é identificada, o microcontrolador processa as informações solicitadas (como leituras de sensores ou estados dos atuadores) e as envia de volta ao dispositivo solicitante via servidor web, permitindo o monitoramento remoto em tempo real.



Esse fluxograma representa um sistema eficiente para o controle remoto da bancada MPS Festo usando um microcontrolador com conectividade Wi-Fi. Esse processo garante que o usuário consiga acessar e controlar a bancada remotamente, monitorando e ajustando os parâmetros em tempo real por meio de uma interface web. A interface microcontrolada desenvolvida com esse fluxograma proporciona maior flexibilidade e autonomia para o gerenciamento dos processos na bancada, além de melhorar a integração com sistemas de controle personalizados.
