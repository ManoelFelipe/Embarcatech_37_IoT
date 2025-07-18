#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
@file data_ingestor.py
@version 1.0
@date 18/07/2025
@author Manoel Felipe Costa Furtado
@copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)

@brief Script para ingestão de dados de sensores via MQTT e armazenamento em um banco de dados PostgreSQL.

@details
Este script atua como uma ponte (bridge) entre um broker MQTT e um banco de dados.
Ele se conecta a um broker, se inscreve em um tópico específico para receber dados de
sensores em formato JSON e, para cada mensagem recebida, ele a processa e insere
as informações em uma tabela no PostgreSQL.

A arquitetura é baseada em eventos (event-driven), utilizando os callbacks da biblioteca
Paho-MQTT para reagir a conexões e novas mensagens de forma assíncrona. O script é
projetado para ser robusto, com tratamento de erros tanto na conexão com o banco
de dados quanto no processamento das mensagens MQTT.
"""

print("▶️ Iniciando o script de ingestão de dados (versão JSON)...")

# --- MÓDULOS IMPORTADOS ---
import paho.mqtt.client as mqtt  # Biblioteca para comunicação com o broker MQTT.
import psycopg2                  # Driver para conexão com o banco de dados PostgreSQL.
import json                      # Módulo para manipular dados no formato JSON (parse de strings).
import os                        # Módulo de sistema operacional (não utilizado neste script, pode ser removido).
import time                      # Módulo de tempo (não utilizado neste script, pode ser removido).

# --- SEÇÃO DE CONFIGURAÇÃO ---
# Centralizar as configurações aqui facilita a manutenção e a adaptação do script
# para diferentes ambientes (desenvolvimento, produção) sem alterar a lógica.

# Configurações do Broker MQTT
MQTT_BROKER = "192.168.1.104"  # Endereço IP do servidor onde o broker MQTT está rodando.
MQTT_PORT = 4004               # Porta de comunicação do broker MQTT.
# Tópico MQTT para inscrição. O caractere '+' é um wildcard (curinga) de nível único.
# Isso significa que o script receberá mensagens de tópicos como "Sensores/dados/json",
# "Dispositivo2/dados/json", etc. Torna o script flexível para múltiplos dispositivos.
MQTT_TOPIC = "+/dados/json"

# Configurações do Banco de Dados PostgreSQL
DB_HOST = "localhost"   # Endereço do servidor do banco de dados (neste caso, na mesma máquina).
DB_PORT = "5432"        # Porta padrão do PostgreSQL.
DB_NAME = "dados_sensores" # Nome do banco de dados a ser utilizado.
DB_USER = "postgres"    # Nome de usuário para acessar o banco de dados.
DB_PASS = "adminpostgres" # Senha para o usuário do banco de dados.

# --- FIM DA CONFIGURAÇÃO ---


def conectar_banco():
    """
    Tenta estabelecer uma conexão com o banco de dados PostgreSQL.

    Utiliza as credenciais e endereços definidos na seção de configuração.
    A função inclui tratamento de erro para falhas de conexão.

    @return: Retorna um objeto de conexão (`psycopg2.connection`) em caso de sucesso,
             ou `None` se a conexão falhar.
    """
    try:
        # Tenta conectar usando os parâmetros globais.
        conn = psycopg2.connect(
            host=DB_HOST,
            port=DB_PORT,
            dbname=DB_NAME,
            user=DB_USER,
            password=DB_PASS
        )
        print("✅ Conectado ao banco de dados PostgreSQL com sucesso!")
        return conn
    except psycopg2.OperationalError as e:
        # Captura erros comuns de conexão (ex: host não encontrado, porta errada, autenticação).
        print(f"❌ Erro ao conectar ao banco de dados: {e}")
        return None


def inserir_leitura(conn, temperatura, umidade, luminosidade):
    """
    Insere uma nova linha com as leituras dos sensores na tabela do banco de dados.

    @param conn: O objeto de conexão ativo com o banco de dados.
    @param temperatura: O valor da temperatura (float).
    @param umidade: O valor da umidade (float).
    @param luminosidade: O valor da luminosidade (float).
    """
    # A query SQL para inserir os dados. Usar '%s' como placeholder é a forma
    # recomendada pelo psycopg2 para evitar ataques de injeção de SQL.
    sql = """INSERT INTO leituras_sensores (temperatura, umidade, luminosidade) VALUES (%s, %s, %s);"""

    cursor = None
    try:
        # Um cursor é um objeto que permite executar comandos no banco de dados.
        cursor = conn.cursor()
        # Executa a query, passando os valores em uma tupla para preencher os placeholders.
        cursor.execute(sql, (temperatura, umidade, luminosidade))
        # `commit()` efetivamente salva as alterações no banco de dados. Sem isso, a transação seria descartada.
        conn.commit()
        print(f"✔️ Dados inseridos com sucesso: Temp={temperatura}°C, Umid={umidade}%, Lux={luminosidade}")
    except (Exception, psycopg2.Error) as error:
        print(f"🔥 Erro ao inserir dado no banco de dados: {error}")
        if conn:
            # `rollback()` desfaz a transação em caso de erro, garantindo a integridade dos dados.
            conn.rollback()
    finally:
        # O bloco `finally` garante que o cursor seja fechado, liberando recursos,
        # independentemente de a operação ter tido sucesso ou falha.
        if cursor:
            cursor.close()


# --- Funções de Callback do MQTT ---
# Callbacks são funções que são executadas automaticamente pela biblioteca Paho-MQTT
# em resposta a determinados eventos da rede.

def on_connect(client, userdata, flags, rc, properties=None):
    """
    Função de callback chamada quando o cliente se conecta ao broker MQTT.

    @param client: A instância do cliente.
    @param userdata: Dados do usuário passados para o cliente (não usado aqui diretamente).
    @param flags: Flags de resposta do broker.
    @param rc: O código de resultado da conexão (Return Code). 0 significa sucesso.
    @param properties: Propriedades da conexão (para MQTT v5).
    """
    if rc == 0:
        print("✅ Conectado ao Broker MQTT!")
        # Uma vez conectado, inscreve-se no tópico de interesse para começar a receber mensagens.
        client.subscribe(MQTT_TOPIC)
        print(f"👂 Inscrito no tópico: {MQTT_TOPIC}")
    else:
        # Se `rc` for diferente de 0, a conexão falhou. Os códigos indicam o motivo (ex: protocolo inválido, ID rejeitado).
        print(f"❌ Falha na conexão com o MQTT, código de retorno: {rc}")


def on_message(client, userdata, msg):
    """
    Função de callback chamada toda vez que uma mensagem é recebida em um tópico inscrito.

    @param client: A instância do cliente.
    @param userdata: Dados do usuário. Usamos para acessar a conexão com o banco de dados.
    @param msg: A mensagem recebida. Contém `msg.topic` e `msg.payload`.
    """
    # Recupera a conexão com o banco de dados que foi armazenada no `userdata`.
    db_connection = userdata['db_conn']

    # O payload da mensagem chega como uma sequência de bytes. É preciso decodificá-lo
    # para uma string (usando o padrão UTF-8) antes de processá-lo.
    payload_str = msg.payload.decode('utf-8')
    print(f"📨 Mensagem recebida no tópico '{msg.topic}': {payload_str}")

    try:
        # 1. Converte a string do payload (que deve ser um JSON) para um dicionário Python.
        dados_sensores = json.loads(payload_str)

        # 2. Extrai cada valor do dicionário usando sua chave correspondente.
        #    A conversão para `float` garante que os dados sejam numéricos.
        temp = float(dados_sensores['temperatura'])
        umid = float(dados_sensores['umidade'])
        lum = float(dados_sensores['luminosidade'])

        # 3. Com os dados extraídos e validados, chama a função para inseri-los no banco.
        inserir_leitura(db_connection, temp, umid, lum)

    # Tratamento de erros para tornar o script robusto contra mensagens malformadas.
    except json.JSONDecodeError:
        print(f"⚠️ Ignorando mensagem. Payload não é um JSON válido: '{payload_str}'")
    except KeyError as e:
        print(f"⚠️ Ignorando mensagem. Chave JSON esperada não encontrada: {e}")
    except ValueError:
        print(f"⚠️ Ignorando mensagem. Um dos valores no JSON não é um número válido.")
    except Exception as e:
        print(f"🔥 Ocorreu um erro inesperado ao processar a mensagem: {e}")


# --- Bloco Principal de Execução ---
# O código dentro deste `if` só é executado quando o script é rodado diretamente,
# e não quando é importado como um módulo em outro script.
if __name__ == "__main__":
    # 1. Tenta conectar ao banco de dados antes de iniciar o cliente MQTT.
    db_conn = conectar_banco()

    # 2. Prossiga apenas se a conexão com o banco for bem-sucedida.
    if db_conn:
        # Cria uma instância do cliente MQTT.
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        # Armazena a conexão do banco de dados no "userdata" do cliente.
        # Isso torna o objeto `db_conn` acessível dentro dos callbacks (como on_message).
        client.user_data_set({'db_conn': db_conn})

        # Associa nossas funções de callback aos eventos correspondentes do cliente.
        client.on_connect = on_connect
        client.on_message = on_message

        try:
            # 3. Tenta conectar ao broker MQTT.
            client.connect(MQTT_BROKER, MQTT_PORT, 60)
            # 4. Inicia o loop de rede. `loop_forever()` é uma função bloqueante que
            #    mantém o script rodando, ouvindo por mensagens, e tratando
            #    reconexões automaticamente, até que o script seja interrompido.
            client.loop_forever()

        # Tratamento de erros que podem ocorrer ao iniciar o loop.
        except ConnectionRefusedError:
            print("❌ Erro: Conexão com o broker MQTT recusada. Verifique o endereço, a porta e se o broker está ativo.")
        except OSError as e:
            print(f"❌ Erro de rede ou de sistema operacional: {e}")
        except KeyboardInterrupt:
            # Captura o sinal de interrupção do teclado (Ctrl+C) para um desligamento limpo.
            print("\n🔌 Desconectando e finalizando o script...")
        finally:
            # Este bloco é executado sempre ao final, garantindo que a conexão com o banco
            # seja fechada de forma limpa, liberando os recursos no servidor do banco de dados.
            if db_conn:
                db_conn.close()
                print("✔️ Conexão com o banco de dados fechada.")