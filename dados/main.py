print("▶️ Iniciando o script main.py...") # LINHA NOVA ADICIONADA

import paho.mqtt.client as mqtt
import psycopg2
import os
import time

# --- CONFIGURAÇÃO ---
# Preencha com os seus dados

# Configurações do Broker MQTT
MQTT_BROKER = "192.168.1.107"  # Endereço do seu broker MQTT (ex: "localhost" ou "192.168.1.10")
MQTT_PORT = 4004           # Porta padrão do MQTT
MQTT_TOPIC = "+/dados/Sensor_Luz" # << IMPORTANTE: Coloque aqui o tópico MQTT exato que seu sensor publica

# Configurações do Banco de Dados PostgreSQL
DB_HOST = "localhost"
DB_PORT = "5432"
DB_NAME = "sensor_luminosidade" # O nome do seu banco de dados
DB_USER = "postgres"            # Seu usuário do Postgres
DB_PASS = "adminpostgres"       # << IMPORTANTE: Coloque aqui a senha do seu banco de dados

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

def inserir_leitura(conn, valor):
    """Insere um novo valor de luminosidade na tabela."""
    # A query SQL para inserir o dado. O %s é um placeholder para evitar SQL Injection.
    sql = """INSERT INTO leituras_luminosidade (valor) VALUES (%s);"""
    
    cursor = None
    try:
        # Um cursor é usado para executar comandos no banco de dados
        cursor = conn.cursor()
        # Executa o comando SQL, passando o valor em uma tupla
        cursor.execute(sql, (valor,))
        # Confirma a transação, salvando os dados permanentemente
        conn.commit()
        print(f"✔️ Dado inserido com sucesso: {valor}")
    except (Exception, psycopg2.Error) as error:
        print(f"🔥 Erro ao inserir dado: {error}")
        # Desfaz a transação em caso de erro
        if conn:
            conn.rollback()
    finally:
        # Garante que o cursor seja fechado
        if cursor:
            cursor.close()


# --- Funções Callback do MQTT ---

# Adicionamos 'properties=None' para aceitar o 5º argumento da nova API
def on_connect(client, userdata, flags, rc, properties=None):
    """Esta função é chamada quando o cliente se conecta ao broker."""
    if rc == 0:
        print("✅ Conectado ao Broker MQTT!")
        # Inscreve-se no tópico de interesse assim que a conexão é estabelecida
        client.subscribe(MQTT_TOPIC)
        print(f"👂 Inscrito no tópico: {MQTT_TOPIC}")
    else:
        print(f"❌ Falha na conexão com o MQTT, código de retorno: {rc}")

def on_message(client, userdata, msg):
    """Esta função é chamada toda vez que uma mensagem é recebida no tópico inscrito."""
    # 'userdata' é onde armazenamos a conexão com o banco
    db_connection = userdata['db_conn']
    
    # O payload da mensagem chega como bytes, então decodificamos para string
    payload_str = msg.payload.decode('utf-8')
    print(f"📨 Mensagem recebida no tópico '{msg.topic}': {payload_str}")
    
    try:
        # Tenta converter o payload para um número (float)
        valor_luminosidade = float(payload_str)
        # Chama a função para inserir o dado no banco
        inserir_leitura(db_connection, valor_luminosidade)
    except ValueError:
        print(f"⚠️ Ignorando mensagem. Payload não é um número válido: '{payload_str}'")
    except Exception as e:
        print(f"🔥 Ocorreu um erro inesperado: {e}")


# --- Bloco Principal de Execução ---
if __name__ == "__main__":
    # 1. Tenta conectar ao banco de dados primeiro
    db_conn = conectar_banco()
    
    # 2. Se a conexão com o banco for bem-sucedida, continua para o MQTT
    if db_conn:
        # Cria um cliente MQTT
        # Cria um cliente MQTT usando a API de Callback mais recente
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) # <--- LINHA ALTERADA
        
        # Armazena a conexão do banco de dados no 'userdata' do cliente MQTT
        # para que possamos acessá-la dentro do callback on_message
        client.user_data_set({'db_conn': db_conn})
        
        # Associa as funções de callback
        client.on_connect = on_connect
        client.on_message = on_message

        try:
            # Conecta ao broker MQTT
            client.connect(MQTT_BROKER, MQTT_PORT, 60)
            
            # Inicia o loop para processar as mensagens.
            # loop_forever() bloqueia a execução e mantém o script rodando para ouvir o MQTT.
            client.loop_forever()
            
        except ConnectionRefusedError:
            print("❌ Erro: Conexão com o broker MQTT recusada. Verifique o endereço e a porta.")
        except OSError as e:
            print(f"❌ Erro de rede ou de sistema operacional: {e}")
        except KeyboardInterrupt:
            # Permite que o usuário pare o script com Ctrl+C
            print("\n🔌 Desconectando...")
        finally:
            # Garante que a conexão com o banco de dados seja fechada ao sair
            db_conn.close()
            print("✔️ Conexão com o banco de dados fechada.")