# -*- coding: utf-8 -*-

# =================================================================================
# --- IMPORTAÇÃO DAS BIBLIOTECAS ---
# =================================================================================
import dash
from dash import dcc  # Dash Core Components (para gráficos, sliders, etc.)
from dash import html # Dash HTML Components (para tags HTML como Div, H1, etc.)
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import psycopg2
import pandas as pd

# =================================================================================
# --- CONFIGURAÇÕES ---
# =================================================================================
# Preencha com os dados do seu banco de dados PostgreSQL
DB_HOST = "localhost"
DB_PORT = "5432"
DB_NAME = "dados_sensores" # ATENÇÃO: Verifique se o nome do banco está correto
DB_USER = "postgres"
DB_PASS = "adminpostgres"

# =================================================================================
# --- FUNÇÕES AUXILIARES ---
# =================================================================================

def conectar_banco():
    """Cria e retorna uma conexão com o banco de dados."""
    try:
        conn = psycopg2.connect(
            host=DB_HOST,
            port=DB_PORT,
            dbname=DB_NAME,
            user=DB_USER,
            password=DB_PASS
        )
        # Removido o print para não poluir o console a cada atualização
        return conn
    except psycopg2.OperationalError as e:
        print(f"❌ Erro ao conectar ao banco de dados: {e}")
        return None

def buscar_dados(conn):
    """Busca os últimos 100 registros do banco de dados e retorna como um DataFrame do Pandas."""
    if conn is None:
        return pd.DataFrame()
    try:
        # ATENÇÃO: Verifique se o nome da tabela 'leituras_sensores' está correto
        query = "SELECT data_hora, temperatura, umidade, luminosidade FROM leituras_sensores ORDER BY data_hora DESC LIMIT 100;"
        df = pd.read_sql_query(query, conn)
        return df.iloc[::-1]
    except (Exception, psycopg2.Error) as error:
        print(f"🔥 Erro ao buscar dados: {error}")
        return pd.DataFrame()

# =================================================================================
# --- INICIALIZAÇÃO DO APP DASH ---
# =================================================================================

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)
app.title = "Dashboard de Sensores"

# =================================================================================
# --- LAYOUT DO DASHBOARD ---
# =================================================================================

app.layout = html.Div(children=[
    html.H1(
        children='Dashboard de Sensores em Tempo Real',
        style={'textAlign': 'center'}
    ),

    # --- MUDANÇA: Adicionando a Tabela de Estatísticas ---
    html.H2(children='Estatísticas (Últimos 100 registros)', style={'textAlign': 'center'}),
    html.Table(id='tabela-estatisticas', style={'margin': 'auto', 'width': '50%'}),
    # --- Fim da Mudança ---

    dcc.Graph(id='grafico-temperatura'),
    dcc.Graph(id='grafico-umidade'),
    dcc.Graph(id='grafico-luminosidade'),

    dcc.Interval(
        id='intervalo-atualizacao',
        interval=5*1000,
        n_intervals=0
    )
])

# =================================================================================
# --- CALLBACKS (LÓGICA INTERATIVA) ---
# =================================================================================

@app.callback(
    # --- MUDANÇA: Adicionando a tabela como um Output ---
    [Output('grafico-temperatura', 'figure'),
     Output('grafico-umidade', 'figure'),
     Output('grafico-luminosidade', 'figure'),
     Output('tabela-estatisticas', 'children')], # A saída é o "filho" da tabela
    # --- Fim da Mudança ---
    [Input('intervalo-atualizacao', 'n_intervals')]
)
def atualizar_componentes(n):
    """
    Esta função agora atualiza tanto os gráficos quanto a tabela de estatísticas.
    """
    conn = conectar_banco()
    df = buscar_dados(conn)
    if conn:
        conn.close()

    # Se não houver dados, retorna figuras e tabela vazias
    if df.empty:
        empty_fig = go.Figure().update_layout(title='Aguardando dados...')
        empty_table = [html.Thead(html.Tr(html.Th("Aguardando dados...")))]
        return empty_fig, empty_fig, empty_fig, empty_table

    # --- Criação das Figuras (Gráficos) ---
    fig_temp = go.Figure(
        data=[go.Scatter(x=df['data_hora'], y=df['temperatura'], mode='lines+markers', name='Temperatura')]
    ).update_layout(title='Temperatura (°C)', yaxis_title='°C')

    fig_umid = go.Figure(
        data=[go.Scatter(x=df['data_hora'], y=df['umidade'], mode='lines+markers', name='Umidade', marker_color='orange')]
    ).update_layout(title='Umidade Relativa (%)', yaxis_title='%')

    fig_lum = go.Figure(
        data=[go.Scatter(x=df['data_hora'], y=df['luminosidade'], mode='lines+markers', name='Luminosidade', marker_color='green')]
    ).update_layout(title='Luminosidade (Lux)', yaxis_title='Lux')

    # --- MUDANÇA: Cálculo e Montagem da Tabela de Estatísticas ---
    stats = {
        'Última Leitura': df[['temperatura', 'umidade', 'luminosidade']].iloc[-1],
        'Média': df[['temperatura', 'umidade', 'luminosidade']].mean(),
        'Mínimo': df[['temperatura', 'umidade', 'luminosidade']].min(),
        'Máximo': df[['temperatura', 'umidade', 'luminosidade']].max(),
        'Desvio Padrão': df[['temperatura', 'umidade', 'luminosidade']].std(),
    }

    header = [html.Thead(html.Tr([
        html.Th("Estatística"),
        html.Th("Temperatura"),
        html.Th("Umidade"),
        html.Th("Luminosidade")
    ]))]

    body = [html.Tbody([
        html.Tr([
            html.Td(stat_name),
            html.Td(f"{values['temperatura']:.2f}"),
            html.Td(f"{values['umidade']:.2f}"),
            html.Td(f"{values['luminosidade']:.2f}")
        ]) for stat_name, values in stats.items()
    ])]

    tabela_children = header + body
    # --- Fim da Mudança ---

    # Retorna todos os componentes atualizados
    return fig_temp, fig_umid, fig_lum, tabela_children

# =================================================================================
# --- EXECUÇÃO DO SERVIDOR ---
# =================================================================================

if __name__ == '__main__':
    app.run(debug=True)
