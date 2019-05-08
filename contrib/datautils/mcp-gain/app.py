#!/usr/bin/python3
import base64
import datetime
import dash
from dash.dependencies import Input, Output, State
import dash_core_components as dcc
import dash_html_components as html
import dash_table
import pandas as pd
import plotly.plotly as py
import plotly.graph_objs as go
import sqlite3
import io
import os
import glob

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
datapath = ['/home/toshi/data/hpk-mcp/2019-04-25']

dirlist = glob.glob('/home/toshi/data/hpk-mcp/2019-04-25/gain/*')
pattern = '/home/toshi/data/hpk-mcp/2019-04-25/mcp1/gain/*.adfs'

step = 5
tof = 99.8765e-6
tof_width = 10e-8
tof_lower = tof - tof_width / 2
tof_upper = tof - tof_width / 2
max_y = 0

query = "SELECT *,COUNT(*) AS COUNTS FROM (SELECT MIN(peak_time),ROUND((-peak_intensity)/{0})*{0} AS Threshold \
FROM trigger,peak WHERE id=idTrigger AND peak_time > ({1}-{2}) AND peak_time < ({1}+{2}) GROUP BY id) \
GROUP BY Threshold".format(step, tof, tof_width)

quary_arg = "SELECT sum(-Threshold*COUNTS)/sum(COUNTS) FROM \
(SELECT *,COUNT(*) AS COUNTS FROM \
(SELECT MIN(peak_time),ROUND(peak_intensity/{0})*{0} AS Threshold FROM trigger,peak WHERE id=idTrigger \
AND peak_time > ({1}-{2}) AND peak_time < ({1}+{2}) GROUP BY id) \
GROUP BY Threshold ) WHERE COUNTS > {3}'".format( step, tof, tof_width, max_y * 0.5 )

#query = "SELECT * from trigger,peak WHERE id=idTrigger AND peak_time > ({0} - {1}) AND peak_time < ({0} + {1})".format( tof, tof_width )

print(query)

app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

app.layout = html.Div(children=[
    html.H1(children='MCP Gain Plot'),

    html.Div(children='''
        Dash: A web application framework for Python.
    '''),

    dcc.Input(id='my-id', value='initial value', type='text'),
    html.Div(id='my-div'),

    dcc.Dropdown( id='dropdown', options=[{'label':name, 'value':name} for name in glob.glob( pattern )], value=''),
    dcc.Input(id='filename', value='', type='text'),

    html.Div( [ html.P(query) ] ),
    html.Div(id='gain-table'),
])

def mcp_gain_query( filename ):
    conn = sqlite3.connect( filename )
    df = pd.read_sql_query( query, conn )
    #print(df.to_dict('records'))
    return html.Div( [ html.H6(filename),
                       html.Div([
                           html.Div([
                               dcc.Graph(
                                   figure={
                                       'data': [
                                           {'x': df["Threshold"], 'y': df["COUNTS"], 'type': 'scatter', 'name': 'SF'},
                                       ],
                                       'layout': {
                                           'title': 'MCP Peak Height Distribution'
                                       }
                                   }
                               ),
                           ], className="six columns"),
                           html.Div([
                               dash_table.DataTable( data=df.to_dict('records'), columns=[{'name': i, 'id': i} for i in df.columns] ),
                           ], className="six columns"),
                       ], className="row"),

                       html.Hr(),
    ])

@app.callback( Output('gain-table', 'children'),
               [Input(component_id='dropdown', component_property='value')] )
def update_output( sqlfile ):
    if sqlfile != "":
        children = [ mcp_gain_query( sqlfile ) ]
        return children

@app.callback(
    Output(component_id='my-div', component_property='children'),
    [Input(component_id='my-id', component_property='value')]
)

def update_output_div(input_value):
    return 'You\'ve entered "{}"'.format(input_value)

if __name__ == '__main__':
    app.run_server(debug=True,port=8050,host='0.0.0.0')
    #    app.run_server(debug=False,port=8080,host='0.0.0.0')
