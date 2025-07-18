#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
@file dashboard.py
@version 1.0
@date 18/07/2025
@author Manoel Felipe Costa Furtado
@copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)

@brief Script de um dashboard web interativo para visualização e análise de dados de sensores.

@details
Este script utiliza a biblioteca Dash para criar uma aplicação web que exibe dados de
sensores em tempo real, lidos de um banco de dados PostgreSQL. O dashboard não apenas
visualiza os dados em gráficos, mas também aplica dois modelos de Machine Learning:
1.  Isolation Forest: Para detectar anomalias (leituras atípicas) nos dados.
2.  Regressão Linear: Para prever a tendência futura da temperatura.

A aplicação se atualiza automaticamente a cada 5 segundos, buscando novos dados,
recalculando as análises e atualizando todos os componentes visuais.
"""

# =================================================================================
# --- IMPORTAÇÃO DAS BIBLIOTECAS ---
# =================================================================================
import dash
from dash import dcc, html, dash_table # Componentes principais do Dash para layout e interatividade.
from dash.dependencies import Input, Output # Para criar os callbacks que dão vida ao dashboard.
import plotly.graph_objs as go # Usado para criar as figuras (gráficos) que serão exibidas.
import pandas as pd # Biblioteca fundamental para manipulação e análise de dados (usa DataFrames).
from sqlalchemy import create_engine # Para criar uma conexão eficiente com o banco de dados, otimizada para o Pandas.
from sklearn.ensemble import IsolationForest # Modelo de ML para detecção de anomalias/outliers.
from sklearn.linear_model import LinearRegression # Modelo de ML para previsão de tendência linear.
import numpy as np # Biblioteca para cálculos numéricos, usada para preparar os dados para a previsão.

# =================================================================================
# --- CONFIGURAÇÕES ---
# =================================================================================
# Credenciais e endereço do banco de dados. Centralizar aqui facilita a manutenção.
DB_HOST = "localhost"
DB_PORT = "5432"
DB_NAME = "dados_sensores"
DB_USER = "postgres"
DB_PASS = "adminpostgres"

# =================================================================================
# --- FUNÇÕES AUXILIARES ---
# =================================================================================

def criar_engine_banco():
    """
    Cria e retorna um engine de conexão do SQLAlchemy para o banco PostgreSQL.

    Um 'engine' gerencia um pool de conexões, o que é mais eficiente do que
    abrir e fechar conexões para cada consulta.

    @return: Um objeto `sqlalchemy.engine.Engine` em caso de sucesso, ou `None` em caso de erro.
    """
    try:
        # Formata a string de conexão (URI) no padrão esperado pelo SQLAlchemy.
        db_uri = f"postgresql+psycopg2://{DB_USER}:{DB_PASS}@{DB_HOST}:{DB_PORT}/{DB_NAME}"
        engine = create_engine(db_uri)
        return engine
    except Exception as e:
        print(f"❌ Erro ao criar o engine do banco de dados: {e}")
        return None

def buscar_dados(engine):
    """
    Busca os últimos 200 registros de leituras do banco de dados.

    @param engine: O engine de conexão do SQLAlchemy.
    @return: Um DataFrame do Pandas com os dados, ou um DataFrame vazio em caso de erro.
    """
    if engine is None:
        return pd.DataFrame() # Retorna um DataFrame vazio se não houver engine.
    try:
        # Query SQL para selecionar as colunas desejadas, ordenando pela data mais recente
        # e limitando o resultado aos últimos 200 registros.
        query = "SELECT id, data_hora, temperatura, umidade, luminosidade FROM leituras_sensores ORDER BY data_hora DESC LIMIT 200;"
        df = pd.read_sql_query(query, engine)
        # Os dados vêm do banco em ordem decrescente (do mais novo para o mais antigo).
        # Invertemos o DataFrame (`.iloc[::-1]`) para que a ordem no gráfico seja cronológica
        # (do mais antigo para o mais novo), o que é mais intuitivo para visualização.
        return df.iloc[::-1].reset_index(drop=True)
    except Exception as error:
        print(f"🔥 Erro ao buscar dados: {error}")
        return pd.DataFrame()

def detectar_anomalias(df):
    """
    Utiliza o modelo Isolation Forest para detectar anomalias no DataFrame.

    O Isolation Forest "isola" observações aleatoriamente. Anomalias são mais
    fáceis de isolar e, portanto, são identificadas como tal.

    @param df: DataFrame do Pandas contendo os dados dos sensores.
    @return: O mesmo DataFrame com uma nova coluna 'anomalia' (-1 para anomalia, 1 para normal).
    """
    # O modelo precisa de pelo menos 2 pontos para funcionar.
    if df.shape[0] < 2:
        df['anomalia'] = 1 # Marca como normal se não houver dados suficientes.
        return df
        
    features = ['temperatura', 'umidade', 'luminosidade']
    # 'contamination'='auto' permite que o modelo decida o limiar de anomalia.
    # 'random_state' garante que o resultado seja o mesmo a cada execução (reprodutibilidade).
    model = IsolationForest(contamination='auto', random_state=42)
    model.fit(df[features])
    # `predict` retorna -1 para anomalias e 1 para pontos normais.
    df['anomalia'] = model.predict(df[features])
    return df

def prever_proxima_temperatura(df):
    """
    Utiliza Regressão Linear para prever o próximo valor de temperatura com base na tendência.

    O modelo traça uma "linha de tendência" através dos pontos de dados existentes e a
    estende para prever o valor no próximo passo de tempo.

    @param df: DataFrame com os dados históricos.
    @return: Uma string formatada com a previsão ou uma mensagem de aviso.
    """
    # A previsão requer uma quantidade mínima de dados para ser minimamente confiável.
    if df.shape[0] < 10:
        return "Dados insuficientes para prever"

    # Prepara os dados para o modelo:
    # X: Variável independente (o tempo, representado pelo índice do DataFrame).
    # y: Variável dependente (o que queremos prever, a temperatura).
    X = np.array(df.index).reshape(-1, 1) # `reshape` é necessário para o formato esperado pelo scikit-learn.
    y = df['temperatura'].values

    # Cria e treina o modelo de Regressão Linear.
    model = LinearRegression()
    model.fit(X, y)

    # Prevê o valor para o próximo índice (que representa o próximo ponto no tempo).
    proximo_index = len(df)
    previsao = model.predict(np.array([[proximo_index]]))

    # Retorna a previsão formatada com duas casas decimais.
    return f"{previsao[0]:.2f} °C"

# =================================================================================
# --- INICIALIZAÇÃO E LAYOUT DO APP DASH ---
# =================================================================================

# `external_stylesheets` permite adicionar um CSS externo para estilizar a página.
external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
# Cria a instância da aplicação Dash.
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)
app.title = "Dashboard Avançado de Sensores"

# `app.layout` define a estrutura visual da página, como uma árvore de componentes HTML e Dash.
app.layout = html.Div(children=[
    html.H1(children='Dashboard Avançado com Machine Learning', style={'textAlign': 'center', 'marginBottom': '20px'}),

    # Bloco explicativo sobre o uso de Machine Learning no dashboard.
    html.Div([
        dcc.Markdown("""
            **Como o Machine Learning é usado aqui?**
            Este dashboard utiliza dois modelos de Machine Learning em tempo real:
            1.  **Detecção de Anomalias (Isolation Forest):** O modelo aprende o comportamento "normal" dos seus sensores. Se uma combinação de leituras (ex: temperatura alta com umidade muito baixa) foge desse padrão, ela é marcada como uma **anomalia (X vermelho)** nos gráficos e listada na tabela.
            2.  **Previsão de Tendência (Regressão Linear):** O modelo analisa a tendência recente da temperatura para **prever qual será o próximo valor** registrado, auxiliando na antecipação de mudanças.
        """, style={'backgroundColor': '#f9f9f9', 'border': '1px solid #ddd', 'padding': '10px', 'borderRadius': '5px'})
    ], style={'width': '80%', 'margin': 'auto', 'marginBottom': '20px'}),

    # Linha contendo a tabela de estatísticas e o card de previsão.
    html.Div([
        # Coluna da esquerda para as estatísticas.
        html.Div([
            html.H2(children='Estatísticas Descritivas', style={'textAlign': 'center'}),
            html.Table(id='tabela-estatisticas') # Tabela que será preenchida pelo callback.
        ], className="eight columns"), # `className` refere-se a classes CSS para layout em colunas.

        # Coluna da direita para a previsão de ML.
        html.Div([
            html.H2(children='Previsão de ML', style={'textAlign': 'center'}),
            html.Div(id='card-previsao', style={ # Div que atuará como um "card" para a previsão.
                'textAlign': 'center', 'padding': '20px', 'backgroundColor': '#e7f3ff',
                'borderRadius': '10px', 'border': '2px solid #007bff'
            })
        ], className="four columns"),
    ], className="row", style={'width': '90%', 'margin': 'auto'}),

    # Título e tabela para listar apenas as anomalias detectadas.
    html.H2(children='Anomalias Detectadas', style={'textAlign': 'center', 'marginTop': '40px'}),
    html.Div([
        dash_table.DataTable(
            id='tabela-anomalias',
            columns=[
                {'name': 'Horário', 'id': 'data_hora'},
                {'name': 'Temperatura', 'id': 'temperatura'},
                {'name': 'Umidade', 'id': 'umidade'},
                {'name': 'Luminosidade', 'id': 'luminosidade'},
            ],
            page_size=5, # Mostra 5 anomalias por página.
            style_cell={'textAlign': 'left'},
            style_header={'backgroundColor': 'lightcoral', 'fontWeight': 'bold', 'color': 'white'},
        )
    ], style={'width': '80%', 'margin': 'auto'}),

    # Componentes de gráfico que serão preenchidos pelo callback.
    dcc.Graph(id='grafico-temperatura'),
    dcc.Graph(id='grafico-umidade'),
    dcc.Graph(id='grafico-luminosidade'),

    # Componente invisível que funciona como um "timer".
    # A cada `interval` (5000 ms), ele atualiza sua propriedade `n_intervals`,
    # o que dispara o callback para atualizar todos os dados da página.
    dcc.Interval(id='intervalo-atualizacao', interval=5*1000, n_intervals=0)
])

# =================================================================================
# --- CALLBACKS (LÓGICA INTERATIVA) ---
# =================================================================================

# O decorador `@app.callback` conecta as saídas (Output) com as entradas (Input).
# Sempre que a propriedade do Input mudar, a função abaixo será executada,
# e seu retorno atualizará as propriedades dos Outputs.
@app.callback(
    [Output('grafico-temperatura', 'figure'),
     Output('grafico-umidade', 'figure'),
     Output('grafico-luminosidade', 'figure'),
     Output('tabela-estatisticas', 'children'),
     Output('tabela-anomalias', 'data'),
     Output('card-previsao', 'children')],
    [Input('intervalo-atualizacao', 'n_intervals')]
)
def atualizar_componentes(n):
    """
    Função executada a cada intervalo de tempo para atualizar todos os componentes do dashboard.

    @param n: O número de vezes que o intervalo foi acionado (não usado diretamente, mas necessário para o Input).
    @return: Uma tupla com os novos valores para cada Output na ordem em que foram declarados.
    """
    # 1. Busca os dados mais recentes do banco.
    engine = criar_engine_banco()
    df_raw = buscar_dados(engine)

    # Se o DataFrame estiver vazio, retorna componentes vazios para evitar erros.
    if df_raw.empty:
        empty_fig = go.Figure().update_layout(title='Aguardando dados...')
        empty_table = [html.Thead(html.Tr(html.Th("Aguardando dados...")))]
        empty_card = html.P("Aguardando dados...")
        return empty_fig, empty_fig, empty_fig, empty_table, [], empty_card

    # 2. Aplica os modelos de Machine Learning.
    df = detectar_anomalias(df_raw.copy()) # Usa uma cópia para evitar SettingWithCopyWarning
    df_anomalias = df[df['anomalia'] == -1]
    previsao_temp = prever_proxima_temperatura(df)

    # 3. Cria os gráficos, destacando as anomalias.
    def criar_grafico_com_anomalias(df_plot, y_col, titulo, cor):
        fig = go.Figure()
        # Adiciona a série de dados normais (anomalia == 1).
        fig.add_trace(go.Scatter(x=df_plot[df_plot['anomalia'] == 1]['data_hora'], y=df_plot[df_plot['anomalia'] == 1][y_col], mode='lines+markers', name='Normal', marker_color=cor))
        # Se houver anomalias, adiciona-as como marcadores 'X' vermelhos.
        if not df_anomalias.empty:
            fig.add_trace(go.Scatter(x=df_anomalias['data_hora'], y=df_anomalias[y_col], mode='markers', name='Anomalia', marker_symbol='x', marker_size=10, marker_color='red'))
        fig.update_layout(title=titulo, margin=dict(l=40, r=40, t=40, b=40))
        return fig

    fig_temp = criar_grafico_com_anomalias(df, 'temperatura', 'Temperatura (°C)', 'blue')
    fig_umid = criar_grafico_com_anomalias(df, 'umidade', 'Umidade Relativa (%)', 'orange')
    fig_lum = criar_grafico_com_anomalias(df, 'luminosidade', 'Luminosidade (Lux)', 'green')

    # 4. Calcula as estatísticas descritivas usando o Pandas.
    stats_df = df[['temperatura', 'umidade', 'luminosidade']]
    stats = {
        'Última Leitura': stats_df.iloc[-1], 'Média': stats_df.mean(),
        'Mediana': stats_df.median(), 'Mínimo': stats_df.min(),
        'Máximo': stats_df.max(), 'Desvio Padrão': stats_df.std(),
        'Variância': stats_df.var(),
    }
    # Monta a tabela de estatísticas com componentes HTML.
    header = [html.Thead(html.Tr([html.Th("Estatística"), html.Th("Temperatura"), html.Th("Umidade"), html.Th("Luminosidade")]))]
    body = [html.Tbody([
        html.Tr([
            html.Td(stat_name),
            html.Td(f"{values['temperatura']:.2f}"),
            html.Td(f"{values['umidade']:.2f}"),
            html.Td(f"{values['luminosidade']:.2f}")
        ]) for stat_name, values in stats.items()
    ])]
    tabela_stats_children = header + body

    # 5. Prepara os dados para a tabela de anomalias.
    anomalias_data = df_anomalias.to_dict('records') # Converte o DataFrame de anomalias para o formato esperado pela DataTable.

    # 6. Cria o conteúdo para o card de previsão.
    card_previsao_children = [
        html.H4("Próxima Temperatura"),
        html.H3(previsao_temp, style={'color': '#007bff', 'fontSize': '2em'})
    ]

    # 7. Retorna todos os componentes atualizados para o layout.
    return fig_temp, fig_umid, fig_lum, tabela_stats_children, anomalias_data, card_previsao_children

# =================================================================================
# --- EXECUÇÃO DO SERVIDOR ---
# =================================================================================

# Este bloco só é executado quando o script é rodado diretamente.
if __name__ == '__main__':
    # `app.run(debug=True)` inicia um servidor web local para hospedar o dashboard.
    # `debug=True` habilita o "hot-reloading" (o app atualiza ao salvar o código)
    # e exibe mensagens de erro detalhadas no navegador, o que é útil para desenvolvimento.
    app.run(debug=True)