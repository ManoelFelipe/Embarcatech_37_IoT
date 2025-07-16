print("▶️ Iniciando o script main.py (versão JSON)...")

import paho.mqtt.client as mqtt
import psycopg2
import json
import os
import time

# --- CONFIGURAÇÃO ---

# Configurações do Broker MQTT
MQTT_BROKER = "192.168.1.104"
MQTT_PORT = 4004
# Tópico atualizado para receber o payload JSON de qualquer dispositivo.
MQTT_TOPIC = "+/dados/json" 

# Configurações do Banco de Dados PostgreSQL
DB_HOST = "localhost"
DB_PORT = "5432"
DB_NAME = "dados_sensores"
DB_USER = "postgres"
DB_PASS = "adminpostgres"

# --- FIM DA CONFIGURAÇÃO ---


def conectar_banco():
    """Tenta conectar ao banco de dados PostgreSQL e retorna a conexão."""
    try:
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
        print(f"❌ Erro ao conectar ao banco de dados: {e}")
        return None

# <-- MUDANÇA: A função agora aceita os três valores do sensor.
def inserir_leitura(conn, temperatura, umidade, luminosidade):
    """Insere os novos valores dos sensores na tabela."""
    
    # <-- MUDANÇA: Query SQL atualizada para a nova estrutura da tabela.
    sql = """INSERT INTO leituras_sensores (temperatura, umidade, luminosidade) VALUES (%s, %s, %s);"""
    
    cursor = None
    try:
        cursor = conn.cursor()
        # <-- MUDANÇA: Passa uma tupla com os três valores para a query.
        cursor.execute(sql, (temperatura, umidade, luminosidade))
        conn.commit()
        # <-- MUDANÇA: Mensagem de log mais detalhada.
        print(f"✔️ Dados inseridos com sucesso: Temp={temperatura}°C, Umid={umidade}%, Lux={luminosidade}")
    except (Exception, psycopg2.Error) as error:
        print(f"🔥 Erro ao inserir dado: {error}")
        if conn:
            conn.rollback()
    finally:
        if cursor:
            cursor.close()


# --- Funções Callback do MQTT ---

def on_connect(client, userdata, flags, rc, properties=None):
    """Esta função é chamada quando o cliente se conecta ao broker."""
    if rc == 0:
        print("✅ Conectado ao Broker MQTT!")
        client.subscribe(MQTT_TOPIC)
        print(f"👂 Inscrito no tópico: {MQTT_TOPIC}")
    else:
        print(f"❌ Falha na conexão com o MQTT, código de retorno: {rc}")

# <-- MUDANÇA: Lógica principal da função alterada para processar JSON.
def on_message(client, userdata, msg):
    """Esta função é chamada toda vez que uma mensagem é recebida no tópico inscrito."""
    db_connection = userdata['db_conn']
    
    payload_str = msg.payload.decode('utf-8')
    print(f"📨 Mensagem recebida no tópico '{msg.topic}': {payload_str}")
    
    try:
        # 1. Converte o payload (string JSON) para um dicionário Python
        dados_sensores = json.loads(payload_str)
        
        # 2. Extrai cada valor do dicionário
        temp = float(dados_sensores['temperatura'])
        umid = float(dados_sensores['umidade'])
        lum = float(dados_sensores['luminosidade'])
        
        # 3. Chama a função para inserir os dados no banco
        inserir_leitura(db_connection, temp, umid, lum)

    except json.JSONDecodeError:
        print(f"⚠️ Ignorando mensagem. Payload não é um JSON válido: '{payload_str}'")
    except KeyError as e:
        print(f"⚠️ Ignorando mensagem. Chave não encontrada no JSON: {e}")
    except ValueError:
        print(f"⚠️ Ignorando mensagem. Um dos valores no JSON não é um número válido.")
    except Exception as e:
        print(f"🔥 Ocorreu um erro inesperado: {e}")


# --- Bloco Principal de Execução ---
if __name__ == "__main__":
    db_conn = conectar_banco()
    
    if db_conn:
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        client.user_data_set({'db_conn': db_conn})
        
        client.on_connect = on_connect
        client.on_message = on_message

        try:
            client.connect(MQTT_BROKER, MQTT_PORT, 60)
            client.loop_forever()
            
        except ConnectionRefusedError:
            print("❌ Erro: Conexão com o broker MQTT recusada. Verifique o endereço e a porta.")
        except OSError as e:
            print(f"❌ Erro de rede ou de sistema operacional: {e}")
        except KeyboardInterrupt:
            print("\n🔌 Desconectando...")
        finally:
            db_conn.close()
            print("✔️ Conexão com o banco de dados fechada.")