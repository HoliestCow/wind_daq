
############### CVRS STUFF ###############
import sys
import glob

############ Website stuff ###############
import dash
import dash_core_components as dcc
import dash_html_components as html
from dash.dependencies import Input, Output, State, Event
import plotly.plotly as py
from plotly.graph_objs import *
from scipy.stats import rayleigh
from flask import Flask
import numpy as np
import pandas as pd
import os
import sqlite3
import datetime as dt
import time

############ Database Stuff ##############

# from .thrift_uuid import *
from database import DatabaseOperations

########## DONE IMPORTING ################

start_clock = int(time.time() * 1000)

#### WEB STUFF ####
app = dash.Dash('WIND Online Radiation Monitoring')
webserver = app.server

app.layout = html.Div([
    html.Div([
        html.H2("Radiation Monitoring"),
        html.Img(src="https://s3-us-west-1.amazonaws.com/plotly-tutorials/logo/new-branding/dash-logo-by-plotly-stripe-inverted.png"),
    ], className='banner'),
    html.Div([
        html.Div([
            html.H3("Gamma Counts Per Second (cps)")
        ], className='Title'),
        html.Div([
            dcc.Graph(id='gamma-cps'),
        ], className='twelve columns wind-speed'),
        dcc.Interval(id='gamma-cps-update', interval=1000, n_intervals=0),
    ], className='row gamma-row'),
    html.Div([
        html.Div([
            html.H3("Gamma Spectrum (counts per bin)")
        ], className='Title'),
        html.Div([
            dcc.Slider(
                id='bin-slider',
                min=1,
                max=60,
                step=1,
                value=20,
                updatemode='drag'
            ),
        ], className='histogram-slider'),
        html.P('# of Bins: Auto', id='bin-size', className='bin-size'),
        html.Div([
            dcc.Checklist(
                id='bin-auto',
                options=[
                    {'label': 'Auto', 'value': 'Auto'}
                ],
                values=['Auto']
            ),
        ], className='bin-auto'),
        dcc.Graph(id='spectrum-histogram'),
    ], className='twelve columns spectrum-histogram')
], style={'padding': '0px 10px 15px 10px',
              'marginLeft': 'auto', 'marginRight': 'auto', "width": "900px",
              'boxShadow': '0px 0px 5px 5px rgba(204,204,204,0.4)'})
        # html.Div([
        #     html.Div([
        #         html.H3("WIND DIRECTION")
        #     ], className='Title'),
        #     dcc.Graph(id='wind-direction'),
        # ], className='five columns wind-polar')
    # ], className='row wind-histo-polar')



@app.callback(Output('gamma-cps', 'figure'), [Input('gamma-cps-update', 'n_intervals')])
def gen_wind_speed(interval):
    global start_clock
    # now = dt.datetime.now()
    # sec = now.second
    # minute = now.minute
    # hour = now.hour
    now = int(time.time() * 1000)
    # total_time = (hour * 3600) + (minute * 60) + (sec)

    con = sqlite3.connect("./CVRS_local.sqlite3")
    # df = pd.read_sql_query('SELECT Speed, SpeedError, Direction from Wind where\
    #                         rowid > "{}" AND rowid <= "{}";'
    #                         .format(total_time-200, total_time), con)
    # NOTE: This is all sorts of  fucked up right now. Index is in some weird time.

    df = pd.read_sql_query('SELECT Time, CPS from det_0 where Time > "{}" AND Time <= "{}";'
                           .format(now - 120000, now), con)

    trace = Scatter(
        x=(df['Time'] - start_clock) / 1000,
        y=df['CPS'],
        # line=Line(
        #     color='#42C4F7'
        # ),
        marker=Marker(
            color='#42C4F7'
            ),
        hoverinfo='skip',
        # mode='lines'
        mode='markers'
    )

    layout = Layout(
        height=450,
        xaxis=dict(
            # range=[0, 120000],
            # showgrid=False,
            # showline=False,
            # zeroline=False,
            # fixedrange=True,
            # tickvals=[0, 10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 110000, 120000],
            # ticktext=['120000', '110000', '100000', '90000', '80000', '70000', '60000', '50000', '40000', '30000', '20000', '10000', '0'],
            title='Time Elapsed (sec)'
        ),
        yaxis=dict(
            range=[min(0, min(df['CPS'])),
                   max(75, max(df['CPS']))],
            showline=False,
            fixedrange=True,
            zeroline=False,
            nticks=max(6, round(df['CPS'].iloc[-1] / 10)),
            title='Counts'
        ),
        margin=Margin(
            t=45,
            l=50,
            r=50
        )
    )

    return Figure(data=[trace], layout=layout)


# @app.callback(Output('wind-direction', 'figure'), [Input('gamma-cps-update', 'n_intervals')])
# def gen_wind_direction(interval):
#     now = dt.datetime.now()
#     sec = now.second
#     minute = now.minute
#     hour = now.hour
#     global start_clock
#     now = dt.datetime.now() - start_clock
#     now = now.seconds
#
#     total_time = (hour * 3600) + (minute * 60) + (sec)
#
#     con = sqlite3.connect("./data/ptu_db.sqlite3")
#     df = pd.read_sql_query("SELECT * from Wind where rowid = " +
#                                          str(total_time) + ";", con)
#     val = df['Speed'].iloc[-1]
#     direction = [0, (df['Direction'][0]-20), (df['Direction'][0]+20), 0]
#
#     trace = Scatterpolar(
#         r=[0, val, val, 0],
#         theta=direction,
#         mode='lines',
#         fill='toself',
#         fillcolor='rgb(242, 196, 247)',
#         line=dict(
#             color='rgba(32, 32, 32, .6)',
#             width=1
#         )
#     )
#     trace1 = Scatterpolar(
#         r=[0, val*0.65, val*0.65, 0],
#         theta=direction,
#         mode='lines',
#         fill='toself',
#         fillcolor='#F6D7F9',
#         line=dict(
#             color = 'rgba(32, 32, 32, .6)',
#             width = 1
#         )
#     )
#     trace2 = Scatterpolar(
#         r=[0, val*0.3, val*0.3, 0],
#         theta=direction,
#         mode='lines',
#         fill='toself',
#         fillcolor='#FAEBFC',
#         line=dict(
#             color='rgba(32, 32, 32, .6)',
#             width=1
#         )
#     )
#
#     layout = Layout(
#         autosize=True,
#         width=275,
#         margin=Margin(
#             t=10,
#             b=10,
#             r=30,
#             l=40
#         ),
#         polar=dict(
#             bgcolor='#F2F2F2',
#             radialaxis=dict(range=[0, 45],
#                             angle=45,
#                             dtick=10),
#             angularaxis=dict(
#                 showline=False,
#                 tickcolor='white',
#             )
#         ),
#         showlegend=False,
#     )
#
#     return Figure(data=[trace, trace1, trace2], layout=layout)


@app.callback(Output('spectrum-histogram', 'figure'),
              [Input('gamma-cps-update', 'n_intervals')],
              [State('gamma-cps', 'figure'),
               State('bin-slider', 'value'),
               State('bin-auto', 'values')])
# @app.callback(Output('wind-speed', 'figure'), [Input('wind-speed-update', 'n_intervals')])
def gen_wind_histogram(interval, wind_speed_figure, sliderValue, auto_state):
    # now = dt.datetime.now()
    # sec = now.second
    # minute = now.minute
    # hour = now.hour
    now = int(time.time() * 1000)

    # total_time = (hour * 3600) + (minute * 60) + (sec)

    con = sqlite3.connect("./CVRS_local.sqlite3")
    # df = pd.read_sql_query('SELECT Speed, SpeedError, Direction from Wind where\
    #                         rowid > "{}" AND rowid <= "{}";'
    #                         .format(total_time-200, total_time), con)
    # NOTE: This is all sorts of  fucked up right now. Index is in some weird time.

    df = pd.read_sql_query('SELECT Spectrum_Array from det_0 where Time > "{}" AND Time <= "{}";'
                           .format(now - 120000, now), con)

    spectrum = df['Spectrum_Array'].apply(lambda x: np.array([float(lol) for lol in x.split(',')]))
    spectrum = spectrum.sum()

    trace = Scatter(
        y=spectrum,
        line=Line(
            color='#42C4F7'
        ),
        hoverinfo='skip',
        mode='lines'
    )

    layout = Layout(
        height=450,
        xaxis=dict(
            range=[0, 1024],
            showgrid=False,
            showline=False,
            zeroline=False,
            fixedrange=True,
            title='Channel Number'
        ),
        yaxis=dict(
            range=[min(0, min(spectrum)),
                   max(45, max(spectrum))],
            showline=False,
            fixedrange=True,
            zeroline=False,
            title='Counts'
        ),
        margin=Margin(
            t=45,
            l=50,
            r=50
        )
    )

    return Figure(data=[trace], layout=layout)


@app.callback(Output('bin-auto', 'values'), [Input('bin-slider', 'value')],
              [State('gamma-cps', 'figure')],
              [Event('bin-slider', 'change')])
def deselect_auto(sliderValue, wind_speed_figure):
    if (wind_speed_figure is not None and
       len(wind_speed_figure['data'][0]['y']) > 5):
        return ['']
    else:
        return ['Auto']

@app.callback(Output('bin-size', 'children'), [Input('bin-auto', 'values')],
              [State('bin-slider', 'value')],
              [])
def deselect_auto(autoValue, sliderValue):
    if 'Auto' in autoValue:
        return '# of Bins: Auto'
    else:
        return '# of Bins: ' + str(int(sliderValue))


external_css = ["https://cdnjs.cloudflare.com/ajax/libs/skeleton/2.0.4/skeleton.min.css",
                "https://cdn.rawgit.com/plotly/dash-app-stylesheets/737dc4ab11f7a1a8d6b5645d26f69133d97062ae/dash-wind-streaming.css",
                "https://fonts.googleapis.com/css?family=Raleway:400,400i,700,700i",
                "https://fonts.googleapis.com/css?family=Product+Sans:400,400i,700,700i"]


for css in external_css:
    app.css.append_css({"external_url": css})

if 'DYNO' in os.environ:
    app.scripts.append_script({
        'external_url': 'https://cdn.rawgit.com/chriddyp/ca0d8f02a1659981a0ea7f013a378bbd/raw/e79f3f789517deec58f41251f7dbb6bee72c44ab/plotly_ga.js'
    })


if __name__ == '__main__':
    ################## Visualization and Control stuff ##################
    # Interacts directly with the database buffer.
    print('Starting web server.')
    app.run_server(host='0.0.0.0', debug=True)
    print('Done.')

# main()