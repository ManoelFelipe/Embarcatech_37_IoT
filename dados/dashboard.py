# -*- coding: utf-8 -*-

# =================================================================================
# --- IMPORTAÇÃO DAS BIBLIOTECAS ---
# =================================================================================
import dash
from dash import dcc, html, dash_table
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import pandas as pd
from sqlalchemy import create_engine
from sklearn.ensemble import IsolationForest
from sklearn.linear_model import LinearRegression # <-- MUDANÇA: Modelo para previsão
import numpy as np # <-- MUDANÇA: Para cálculos numéricos na previsão

# =================================================================================
# --- CONFIGURAÇÕES ---
# =================================================================================
DB_HOST = "localhost"
DB_PORT = "5432"
DB_NAME = "dados_sensores"
DB_USER = "postgres"
DB_PASS = "adminpostgres"

# =================================================================================
# --- FUNÇÕES AUXILIARES ---
# =================================================================================

def criar_engine_banco():
    """Cria e retorna um engine de conexão do SQLAlchemy."""
    try:
        db_uri = f"postgresql+psycopg2://{DB_USER}:{DB_PASS}@{DB_HOST}:{DB_PORT}/{DB_NAME}"
        engine = create_engine(db_uri)
        return engine
    except Exception as e:
        print(f"❌ Erro ao criar o engine do banco de dados: {e}")
        return None

def buscar_dados(engine):
    """Busca os últimos 200 registros do banco de dados."""
    if engine is None:
        return pd.DataFrame()
    try:
        query = "SELECT id, data_hora, temperatura, umidade, luminosidade FROM leituras_sensores ORDER BY data_hora DESC LIMIT 200;"
        df = pd.read_sql_query(query, engine)
        return df.iloc[::-1].reset_index(drop=True)
    except Exception as error:
        print(f"🔥 Erro ao buscar dados: {error}")
        return pd.DataFrame()

def detectar_anomalias(df):
    """Usa IsolationForest para detectar anomalias no DataFrame."""
    if df.shape[0] < 2:
        df['anomalia'] = 0
        return df
    features = ['temperatura', 'umidade', 'luminosidade']
    model = IsolationForest(contamination='auto', random_state=42)
    model.fit(df[features])
    df['anomalia'] = model.predict(df[features])
    return df

# <-- MUDANÇA: Nova função para o modelo de previsão
def prever_proxima_temperatura(df):
    """Usa Regressão Linear para prever o próximo valor de temperatura."""
    if df.shape[0] < 10: # Precisa de um mínimo de dados para uma previsão razoável
        return "Dados insuficientes para prever"

    # Prepara os dados: X é o índice (tempo), y é a temperatura
    X = np.array(df.index).reshape(-1, 1)
    y = df['temperatura'].values

    # Cria e treina o modelo
    model = LinearRegression()
    model.fit(X, y)

    # Prevê o valor para o próximo índice
    proximo_index = len(df)
    previsao = model.predict(np.array([[proximo_index]]))

    return f"{previsao[0]:.2f} °C"

# =================================================================================
# --- INICIALIZAÇÃO E LAYOUT DO APP DASH ---
# =================================================================================

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)
app.title = "Dashboard Avançado de Sensores"

app.layout = html.Div(children=[
    html.H1(children='Dashboard Avançado com Machine Learning', style={'textAlign': 'center', 'marginBottom': '20px'}),

    # <-- MUDANÇA: Legenda explicativa sobre o ML
    html.Div([
        dcc.Markdown("""
            **Como o Machine Learning é usado aqui?**
            Este dashboard utiliza dois modelos de Machine Learning em tempo real:
            1.  **Detecção de Anomalias (Isolation Forest):** O modelo aprende o comportamento "normal" dos seus sensores. Se uma combinação de leituras (ex: temperatura alta com umidade muito baixa) foge desse padrão, ela é marcada como uma **anomalia (X vermelho)** nos gráficos e listada na tabela.
            2.  **Previsão de Tendência (Regressão Linear):** O modelo analisa a tendência recente da temperatura para **prever qual será o próximo valor** registrado, auxiliando na antecipação de mudanças.
        """, style={'backgroundColor': '#f9f9f9', 'border': '1px solid #ddd', 'padding': '10px', 'borderRadius': '5px'})
    ], style={'width': '80%', 'margin': 'auto', 'marginBottom': '20px'}),

    html.Div([
        html.Div([
            html.H2(children='Estatísticas Descritivas', style={'textAlign': 'center'}),
            html.Table(id='tabela-estatisticas')
        ], className="eight columns"), # Ajustado para dar mais espaço

        # <-- MUDANÇA: Card para a previsão de ML
        html.Div([
            html.H2(children='Previsão de ML', style={'textAlign': 'center'}),
            html.Div(id='card-previsao', style={
                'textAlign': 'center',
                'padding': '20px',
                'backgroundColor': '#e7f3ff',
                'borderRadius': '10px',
                'border': '2px solid #007bff'
            })
        ], className="four columns"),
    ], className="row", style={'width': '90%', 'margin': 'auto'}),

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
            page_size=5, # Mostra 5 anomalias por vez
            style_cell={'textAlign': 'left'},
            style_header={'backgroundColor': 'lightcoral', 'fontWeight': 'bold', 'color': 'white'},
        )
    ], style={'width': '80%', 'margin': 'auto'}),

    dcc.Graph(id='grafico-temperatura'),
    dcc.Graph(id='grafico-umidade'),
    dcc.Graph(id='grafico-luminosidade'),

    dcc.Interval(id='intervalo-atualizacao', interval=5*1000, n_intervals=0)
])

# =================================================================================
# --- CALLBACKS (LÓGICA INTERATIVA) ---
# =================================================================================

@app.callback(
    [Output('grafico-temperatura', 'figure'),
     Output('grafico-umidade', 'figure'),
     Output('grafico-luminosidade', 'figure'),
     Output('tabela-estatisticas', 'children'),
     Output('tabela-anomalias', 'data'),
     Output('card-previsao', 'children')], # <-- MUDANÇA: Output para o card de previsão
    [Input('intervalo-atualizacao', 'n_intervals')]
)
def atualizar_componentes(n):
    engine = criar_engine_banco()
    df_raw = buscar_dados(engine)

    if df_raw.empty:
        empty_fig = go.Figure().update_layout(title='Aguardando dados...')
        empty_table = [html.Thead(html.Tr(html.Th("Aguardando dados...")))]
        empty_card = html.P("Aguardando dados...")
        return empty_fig, empty_fig, empty_fig, empty_table, [], empty_card

    # Aplica os modelos de ML
    df = detectar_anomalias(df_raw)
    df_anomalias = df[df['anomalia'] == -1]
    previsao_temp = prever_proxima_temperatura(df) # <-- MUDANÇA: Chama a função de previsão

    # Cria os gráficos com anomalias
    def criar_grafico_com_anomalias(df_plot, y_col, titulo, cor):
        fig = go.Figure()
        fig.add_trace(go.Scatter(x=df_plot[df_plot['anomalia'] == 1]['data_hora'], y=df_plot[df_plot['anomalia'] == 1][y_col], mode='lines+markers', name='Normal', marker_color=cor))
        if not df_anomalias.empty:
            fig.add_trace(go.Scatter(x=df_anomalias['data_hora'], y=df_anomalias[y_col], mode='markers', name='Anomalia', marker_symbol='x', marker_size=10, marker_color='red'))
        fig.update_layout(title=titulo, margin=dict(l=40, r=40, t=40, b=40))
        return fig

    fig_temp = criar_grafico_com_anomalias(df, 'temperatura', 'Temperatura (°C)', 'blue')
    fig_umid = criar_grafico_com_anomalias(df, 'umidade', 'Umidade Relativa (%)', 'orange')
    fig_lum = criar_grafico_com_anomalias(df, 'luminosidade', 'Luminosidade (Lux)', 'green')

    # <-- MUDANÇA: Tabela de estatísticas mais completa
    stats_df = df[['temperatura', 'umidade', 'luminosidade']]
    stats = {
        'Última Leitura': stats_df.iloc[-1],
        'Média': stats_df.mean(),
        'Mediana': stats_df.median(),
        'Mínimo': stats_df.min(),
        'Máximo': stats_df.max(),
        'Desvio Padrão': stats_df.std(),
        'Variância': stats_df.var(),
    }
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

    anomalias_data = df_anomalias.to_dict('records')

    # <-- MUDANÇA: Cria o conteúdo para o card de previsão
    card_previsao_children = [
        html.H4("Próxima Temperatura"),
        html.H3(previsao_temp, style={'color': '#007bff', 'fontSize': '2em'})
    ]

    return fig_temp, fig_umid, fig_lum, tabela_stats_children, anomalias_data, card_previsao_children

# =================================================================================
# --- EXECUÇÃO DO SERVIDOR ---
# =================================================================================

if __name__ == '__main__':
    app.run(debug=True)
