// Função de leitura dos sensores digitais
function fetchSensorDig1() {
    fetch('/data?tipo=sensorDig1')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig1').innerText = 'Sensor Digital 1: ' + data);}

function fetchSensorDig2() {
    fetch('/data?tipo=sensorDig2')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig2').innerText = 'Sensor Digital 2: ' + data);}
    
function fetchSensorDig3() {
    fetch('/data?tipo=sensorDig3')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig3').innerText = 'Sensor Digital 3: ' + data);}

function fetchSensorDig4() {
    fetch('/data?tipo=sensorDig4')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig4').innerText = 'Sensor Digital 4: ' + data);}

function fetchSensorDig5() {
    fetch('/data?tipo=sensorDig5')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig5').innerText = 'Sensor Digital 5: ' + data);}

function fetchSensorDig6() {
    fetch('/data?tipo=sensorDig6')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig6').innerText = 'Sensor Digital 6: ' + data);}

function fetchSensorDig7() {
    fetch('/data?tipo=sensorDig7')
    .then(response => response.text())
    .then(data => document.getElementById('sensorDig7').innerText = 'Sensor Digital 7: ' + data);}

// Função de leitura dos sensores analógicos
function fetchSensorAnalog1() {
    fetch('/data?tipo=sensorAnalog1')
    .then(response => response.text())
    .then(data => document.getElementById('sensorAnalog1').innerText = 'Sensor Analogico 1: ' + data);}

function fetchSensorAnalog2() {
    fetch('/data?tipo=sensorAnalog2')
    .then(response => response.text())
    .then(data => document.getElementById('sensorAnalog2').innerText = 'Sensor Analogico 2: ' + data);}

function fetchSensorAnalog3() {
    fetch('/data?tipo=sensorAnalog3')
    .then(response => response.text())
    .then(data => document.getElementById('sensorAnalog3').innerText = 'Sensor Analogico 3: ' + data);}

function fetchSensorAnalog4() {
    fetch('/data?tipo=sensorAnalog4')
    .then(response => response.text())
    .then(data => document.getElementById('sensorAnalog4').innerText = 'Sensor Analogico 4: ' + data);}

// Função para cada comando dos botões
function sendCommand1() {
    fetch('/command?tipo=command1')
    .then(response => response.text())
    .then(data => console.log('Comando enviado:', data));}

function sendCommand2() {
    fetch('/command?tipo=command2')
    .then(response => response.text())
    .then(data => console.log('Comando enviado:', data));}

function sendCommand3() {
    fetch('/command?tipo=command3')
    .then(response => response.text())
    .then(data => console.log('Comando enviado:', data));}

function sendCommand4() {
    fetch('/command?tipo=command4')
    .then(response => response.text())
    .then(data => console.log('Comando enviado:', data));}

function sendCommand5() {
    fetch('/command?tipo=command5')
    .then(response => response.text())
    .then(data => console.log('Comando enviado:', data));}

// Intervalos para cada leitura de sensor, configurada para 1s
setInterval(fetchSensorDig1, 1000);
setInterval(fetchSensorDig2, 1000);
setInterval(fetchSensorDig3, 1000);
setInterval(fetchSensorDig4, 1000);
setInterval(fetchSensorDig5, 1000);
setInterval(fetchSensorDig6, 1000);
setInterval(fetchSensorDig7, 1000);
setInterval(fetchSensorAnalog1, 1000);
setInterval(fetchSensorAnalog2, 1000);
setInterval(fetchSensorAnalog3, 1000);
setInterval(fetchSensorAnalog4, 1000);

// Função para comando dos DACs
document.addEventListener("DOMContentLoaded", () => {
    // Função para enviar os valores ao ESP32
    const setDAC = (dacId, value) => {
        fetch(`/dac?tipo=dac${dacId}&value=${value}`)
            .then(response => response.text())
            .then(data => console.log(`DAC ${dacId} atualizado para ${value}: ${data}`))
            .catch(error => console.error("Erro:", error));
    };

    // Configuração para os sliders e inputs de DAC
    const setupDACControl = (sliderId, inputId, dacId) => {
        const slider = document.getElementById(sliderId);
        const input = document.getElementById(inputId);

        // Sincroniza input e slider
        slider.addEventListener("input", () => {
            input.value = slider.value;
            setDAC(dacId, slider.value);
        });

        input.addEventListener("input", () => {
            let value = parseInt(input.value, 10);
            if (isNaN(value) || value < 0) value = 0;
            if (value > 255) value = 255;
            slider.value = value;
            setDAC(dacId, value);
        });
    };

    // Configurar DAC 1 e DAC 2
    setupDACControl("dac1-slider", "dac1-input", 1);
    setupDACControl("dac2-slider", "dac2-input", 1);
});