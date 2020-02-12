

import threading
import time
############### CVRS STUFF ###############
import sys
import glob
# sys.path.append('/home/holiestcow/Documents/winds/thrift/wind_daq/WIND-Thrift/gen-py')

# from tutorial import Calculator
# from tutorial.ttypes import InvalidOperation, Operation, Work
import CVRSServices.CVRSEndpoint
from CVRSServices.CVRSEndpoint import Iface
from CVRSServices.ttypes import (StatusCode, ControlType, Session, StartRecordingControlPayload,
                                 ControlPayloadUnion, ControlMessage, ControlMessageAck,
                                 RecordingUpdate, DefinitionAndConfigurationUpdate)
from Exceptions.ttypes import InvalidSession
from UUID.ttypes import UUID
from PTUPayload.ttypes import Status

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.server import TServer
from thrift.protocol import TBinaryProtocol

############ Website stuff ###############
import dash
import dash_core_components as dcc
import dash_html_components as html
import os

from dash.dependencies import Input, Output, State
# import plotly.plotly as py
from plotly.graph_objs import *
# from scipy.stats import rayleigh
# from flask import Flask
import numpy as np
import pandas as pd
import sqlite3

############ Database Stuff ##############

# from .thrift_uuid import *
from wind_daq.data_acquisition.utils.thrift_uuid import Thrift_UUID
from wind_daq.data_acquisition.utils.database import DatabaseOperations

# from thrift_uuid import Thrift_UUID


########## DONE IMPORTING ################

# start_clock = dt.datetime.now()
import datetime

colors = [
    '#1f77b4',  # muted blue
    '#ff7f0e',  # safety orange
    '#2ca02c',  # cooked asparagus green
    '#d62728',  # brick red
    '#9467bd',  # muted purple
    '#8c564b',  # chestnut brown
    '#e377c2',  # raspberry yogurt pink
    '#7f7f7f',  # middle gray
    '#bcbd22',  # curry yellow-green
    '#17becf'   # blue-teal
]

class CVRSHandler(Iface):
    """
    CVRSEndpoint

    This is the service implemented by WIND-compliant CVRS
    software, and provides a mechanism for the PTU to push
    data, status, and other information to the CVRS.

    This service defines a basic flow for connecting to a
    CVRS and sending it data. Each time a PTU re-establishes
    connection to the CVRS, the PTU MUST call these methods
    in the following order:

    1. registerPtu
    2. define
    loop (1hz):
        3. reportStatus
        4. pushData
        5. pushAcknowledgements

    The initial registration establishes a session between the PTU
    and the CVRS which can be used to track the state of the connection.
    The CVRS MAY determine that a session has become invalid if it has
    not received a message from the PTU within a reasonable period of time.

    The specific semantics of each method are described in the
    comments on the method definitions below.
    *
    """
    def __init__(self):
        self.log = {}
        self.sessions = {}
        self.registeredPTUs = {}
        self.PTUdefinitions = {}
        self.current_sessionId = None
        self.controlmessage_acknowledgements = []
        self.controlmessages = []
        self.db = None
        self.acknowledgements = []

    def _set_database(self, filename):
        self.db = DatabaseOperations(filename)
        return

    def ping(self):
        print('ping')
        return "pong"

    def registerPtu(self, unitDefinition):
        """
        Purpose: store PTU unitdefinition accessed by UUID.
        Parameters:
         - unitDefinition
        """
        self.registeredPTUs[unitDefinition.unitName] = unitDefinition
        x = Thrift_UUID.generate_thrift_uuid()
        newSessionId = UUID(
            leastSignificantBits=x[0],
            mostSignificantBits=x[1])
        self.current_sessionId = newSessionId
        return Session(status=StatusCode.OK, sessionId=newSessionId)

    def define(self, sessionId, status, systemDefinition, systemConfiguration, recordingUpdate):
        """
        Parameters:
         - sessionId
         - status
         - systemDefinition
         - systemConfiguration
         - recordingUpdate
        """
        # Check if the session matches the current session:
        # this may just be sessionId. I'm not sure if this is a Session object or not. (sessionId.sessionId)
        if sessionId == self.current_sessionId:
            ptu_message = []
            self.db.initialize_structure(systemDefinition)
            return ptu_message
        else:
            # Sessions ID does not match.
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            ptu_message = []
            return ptu_message

    def reportStatus(self, sessionId, status, definitionAndConfigurationUpdate):
        # throws (1: Exceptions.InvalidSession error);
        # return [controlmessage]
        print('reporting')
        if sessionId == self.current_sessionId:
            message = []
            return message
        else:
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            message = []
            return message

    def pushData(self, sessionId, datum, definitionAndConfigurationUpdate):
        print('pushingData')
        # throws (1: Exceptions.InvalidSession error);
        # Returns:
  #  	 		bool: true if the acnowledgements were successfully received; false otherwise.
  #  				  If the value is false, the PTU MUST re-send this list of acknowledgements again.
        if sessionId == self.current_sessionId:
            print('here')
            # Use private methods to store data into a local  SQL database
            self.db.stack_datum(datum)
            return [True]
        else:
            print('This  is gonna fail me hard.')
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            return [False]

    def pushAcknowledgements(self, sessionId, acknowledgements):
        print('pushingAcks')
        #) throws (1: Exceptions.InvalidSession error);
        if sessionId == self.current_sessionId:
            # maybe only acknowledgements. 2d list vs. 1d?
            self.acknowledgements += acknowledgements
            return [True]
        else:
            message = 'PTU Session ID {} does not match current session ID {}'.format(sessionId,
                self.current_sessionId)
            raise InvalidSession(sessionId=sessionId, message=message)
            return [False]

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
            html.H3("Gamma Detectors")
        ], className='Title'),
        html.Div([
            dcc.Graph(id='gamma-cps',
                      figure=Figure(
                          layout=Layout(
                              title='CPS'
                          )
                      )),
        # ], className='twelve columns wind-speed'),
        ], className='six columns wind-speed'), # twelve / six controls the width of the space.

        html.Div([
        dcc.Graph(id='spectrum-histogram',
                  figure=Figure(
                      layout=Layout(
                          title='Spectrum'
                      )
                  )),
    ], className='twelve columns spectrum-histogram')
], style={'padding': '0px 10px 15px 10px',
              'marginLeft': 'auto', 'marginRight': 'auto', "width": "900px",
              'boxShadow': '0px 0px 5px 5px rgba(204,204,204,0.4)'}),


        html.Div([
            # html.Div([
            #     html.H3("WIND DIRECTION")
            # ], className='Title'),
            dcc.Graph(id='gps-location',
                      figure=Figure(
                           layout=Layout(
                               title='GPS Location'
                           )
                      )),
        ], className='five columns wind-polar'),
        dcc.Interval(id='periodic-update', interval=1000, n_intervals=0),
    ], className='row gamma-row')
# ], className='row wind-histo-polar'),
# ], style={'padding': '0px 10px 15px 10px',
#           'marginLeft': 'auto', 'marginRight': 'auto', "width": "900px",
#           'boxShadow': '0px 0px 5px 5px rgba(204,204,204,0.4)'})

        # html.Div([
        #     html.Div([
        #         html.H3("WIND DIRECTION")
        #     ], className='Title'),
        #     dcc.Graph(id='wind-direction'),
        # ], className='five columns wind-polar')
    # ], className='row wind-histo-polar')


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


@app.callback(Output('gps-location', 'figure'), [Input('periodic-update', 'n_intervals')])
def get_gps(interval):
    # now = int(time.time())
    now = get_time() 
    time_before = now - 60
    time_after = now

    con = sqlite3.connect("./CVRS_local.sqlite3")
    df = pd.read_sql_query('SELECT Latitude, Longitude FROM gps WHERE Time > "{}" AND Time <= "{}";'
                           .format(time_before, time_after), con)

    trace = Scatter(
        x=list(df['Longitude']),
        y=list(df['Latitude']),
        # x=list(df['Latitude']),
        # y=list(df['Longitude']),
        # line=Line(
        #     color='#42C4F7'
        # ),
        hoverinfo='skip',
        mode='markers',
        marker=dict(
            size=4
        )
    )

    layout = Layout(
        height=450,
        xaxis=dict(
            range=[-83.9244, -83.9240],
            showgrid=True,
            showline=False,
            zeroline=False,
            fixedrange=True,
            # tickvals=[0, 10, 20, 30, 40, 50, 60],
            # ticktext=['60', '50', '40', '30', '20', '10', '0'],
            title='Latitude'
        ),
        yaxis=dict(
            range=[35.9565, 35.9569],
            showline=False,
            showgrid=True,
            fixedrange=True,
            zeroline=False,
            # nticks=max(6, round(df['CPS'].iloc[-1] / 10)),
            title='Longitude'
        ),
        margin=Margin(
            t=45,
            l=50,
            r=50
        )
    )
    # NOTE: This is for GPS Location
#     mapbox_access_token = 'pk.eyJ1IjoiY2JyaXR0MiIsImEiOiJjamxocGwyZzkwNTAwM3ZsYW45cGphaGVnIn0.7OFl5XmUtxSvZE0JWA_Oyw'
#
#     trace = Scattermapbox(
#         lat=list(df['Latitude']),
#         lon=list(df['Longitude']),
#         mode='markers',
#         marker=dict(
#             size=14
#         ),
#         # hoverinfo='text',
#     )
#
#     layout = Layout(
#         autosize=True,
#         hovermode='closest',
#         mapbox=dict(
#             accesstoken=mapbox_access_token,
#             bearing=0,
#             center=dict(
#                 lat=-83.9231033325195,
#                 lon=35.9570732116699
#                 ),
#             pitch=0,
#             zoom=5
#         ),
# )


    return Figure(data=[trace], layout=layout)

@app.callback(Output('gamma-cps', 'figure'), [Input('periodic-update', 'n_intervals')])
def gen_cps(interval):
    now = get_time()
    time_before = now - 60
    time_after = now

    con = sqlite3.connect("./CVRS_local.sqlite3")

    traces = []
    minval = []
    maxval= []

    for i in range(4):
        df = pd.read_sql_query('SELECT CPS from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, time_before, time_after), con)
        traces += [Scatter(
            y=df['CPS'],
            line=Line(
                color=colors[i]
            ),
            hoverinfo='skip',
            mode='lines'
        )]
        minval += [min(df['CPS'])]
        maxval += [max(df['CPS'])]

    layout = Layout(
        height=450,
        xaxis=dict(
            range=[0, 60],
            showgrid=False,
            showline=False,
            zeroline=False,
            fixedrange=True,
            tickvals=[0, 10, 20, 30, 40, 50, 60],
            ticktext=['60', '50', '40', '30', '20', '10', '0'],
            title='Time Elapsed (sec)'
        ),
        yaxis=dict(
            range=[min(minval) - 50,
                   max(maxval) + 50],
            showline=False,
            fixedrange=True,
            zeroline=False,
            nticks=6,
            title='Counts'
        ),
        margin=Margin(
            t=45,
            l=50,
            r=50
        )
    )

    return Figure(data=traces, layout=layout)


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
              [Input('periodic-update', 'n_intervals')])
              # [State('gamma-cps', 'figure')])
               # State('bin-slider', 'value'),
               # State('bin-auto', 'values')])
# @app.callback(Output('wind-speed', 'figure'), [Input('wind-speed-update', 'n_intervals')])
def gen_wind_histogram(interval):
    # now = int(time.time())
    now = get_time()
    time_before = now - 60
    time_after = now

    bin_number = 2**14

    con = sqlite3.connect("./CVRS_local.sqlite3")
    traces = []
    
    for i in range(4):
        df = pd.read_sql_query('SELECT Spectrum_Array from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, time_before, time_after), con)

        spectrum = df['Spectrum_Array'].apply(lambda x: np.array([float(lol) for lol in x.split(',')]))
        spectrum = spectrum.sum()
        # counts = rebin(spectrum, bin_number)

        traces += [Scatter(
            y=spectrum,
            line=Line(
                color=colors[i]
            ),
            hoverinfo='skip',
            mode='lines'
        )]

    layout = Layout(
        height=450,
        xaxis=dict(
            range=[0, bin_number-1],
            showgrid=False,
            showline=False,
            zeroline=False,
            fixedrange=True,
            title='Channel Number'
        ),
        yaxis=dict(
            range=[0,
                   50],
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
    return Figure(data=traces, layout=layout)


def rebin(data, new_bin_number):
    [counts, bin_edges] = np.histogram(data, bins=new_bin_number)
    return counts


def start_thrift_server():
    ################## CVRS STUFF ##################
    handler = CVRSHandler()
    handler._set_database('CVRS_local.sqlite3')
    processor = CVRSServices.CVRSEndpoint.Processor(handler)
    # transport = TSocket.TServerSocket(host='192.249.3.246', port=8080)
    transport = TSocket.TServerSocket(host='0.0.0.0', port=8080)
    # transport = TSocket.TServerSocket(host='0.0.0.0', port=9090)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()

    # server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)
    server = TServer.TSimpleServer(processor, transport, tfactory, pfactory)

    print('Starting thrift server.')
    server.serve()
    print('Thrift server started.')
    return


def start_webapp_server():
    print('Starting web server.')
    # app.run_server(host='0.0.0.0', port=9090, debug=True)
    app.run_server(port=9090, debug=True)
    return

def get_time():
    now = datetime.datetime.now()
    epoch = datetime.datetime(year=1970, month=1, day=1)
    timesinceepoch = int((now - epoch).total_seconds())
    return timesinceepoch


if __name__ == '__main__':

    # QUESTION: Where should the thread be run? The webapp or the thrift server?

    thrift_thread = threading.Thread(target=start_thrift_server)
    # webapp_thread = threading.Thread(target=start_webapp_server)  # this cannot work. Webapp has to be the calling thread.

    thrift_thread.start()
    # webapp_thread.start()
    # start_thrift_server()
    start_webapp_server()
# main()
