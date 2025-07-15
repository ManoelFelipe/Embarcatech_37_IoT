# Embarcatech_37_IoT

# 💡 Projeto de Aquisição e Análise de Dados de Sensores IoT com Pico W (BITDOGLAB), WiFi com MQTT.

![Linguagem](https://img.shields.io/badge/Linguagem-C-blue.svg)
![Linguagem](https://img.shields.io/badge/Linguagem-Python-yellow.svg)
![Plataforma](https://img.shields.io/badge/Plataforma-Raspberry%20Pi%20Pico%20W-purple.svg)
![Protocolo](https://img.shields.io/badge/Protocolo-MQTT-orange.svg)
![Protocolo](https://img.shields.io/badge/Protocolo-WiFi-blue)
![Visualização](https://img.shields.io/badge/Visualização-Node--RED-red.svg)
![Framework](https://img.shields.io/badge/Framework-Dash-blue?logo=plotly)
![Ciência de Dados](https://img.shields.io/badge/Ciência%20de%20Dados-Ativa-blueviolet?logo=scikit-learn)
![Análise de Dados](https://img.shields.io/badge/Análise%20de%20Dados-Em%20Curso-lightgrey?logo=pandas&logoColor=black)
![Pipeline](https://img.shields.io/badge/Pipeline-Dados-green?logo=airflow&logoColor=white)



## ✨ Funcionalidades Principais

* **Leitura de Sensor I2C:** Interface com o sensor de luminosidade BH1750.
* **Conectividade Wi-Fi:** Conexão a uma rede local usando o chip CYW43439 do Pico W.
* **Protocolo MQTT:** Publicação dos dados de telemetria (QoS 1) para um broker MQTT.
* **Arquitetura Modular:** O código é organizado em módulos (main, sensor, mqtt) para maior clareza e manutenibilidade.
* **Sistema Robusto:** Inclui lógica de reconexão automática ao broker MQTT em caso de falha na conexão.
* **Configuração Centralizada:** Todas as configurações (credenciais, IPs, tópicos) estão em um único arquivo `configura_geral.h`.
* **Firmware Otimizado:** Configurações da pilha de rede lwIP ajustadas para garantir estabilidade e evitar erros de alocação de memória.
* **Firmware Otimizado:** 

## 📊 Painel de Controle (Resultado Final)

## 🏗️ Arquitetura do Sistema

O fluxo de dados segue a seguinte arquitetura:

`[Pico W com Sensores]` -> `[Wi-Fi]` -> `[Broker Mosquitto MQTT]` -> `[Banco de Daods PostgreSQL]` -> `[DashBoard_Python]` -> Retorna ao `[Pico W com Sensores]` para ajustes.


### Hardware
* Placa com Raspberry Pi Pico W (neste projeto, foi usada a **BitDogLab**)
* Sensor de Luminosidade I2C BH1750

### Software

* [Raspberry Pi Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk)
* [Mosquitto](https://mosquitto.org/) (ou qualquer outro broker MQTT)
* [Node-RED](https://nodered.org/)
* Um ambiente de compilação C/C++ (GCC para ARM, CMake, etc.)

## 🚀 Como Compilar e Usar

## 📈 Configuração do Node-RED

## 📈 Configuração Banco de Dados Postgres

## 📈 Configuração Ambiente de Criação de Dashboards, Análise de Dados e Ciência dados.

## 📂 Estrutura dos Arquivos

## 🔮 Possíveis Melhorias Futuras

