
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from wind_daq.data_acquisition.utils.thrift_uuid import Thrift_UUID
from wind_daq.data_acquisition.utils.database import DatabaseOperations
import sqlite3

def get_alltime(con):
    df = pd.read_sql_query('SELECT Time from det_0;', con)
    time_before = min(df['Time'])
    time_after = max(df['Time'])
    return time_before, time_after

def plot_cps(con, start_time, end_time, output_prefix):
    colors = ['r.', 'g.', 'b.', 'm.']
    fig = plt.figure()
    for i in range(4):
        df = pd.read_sql_query('SELECT CPS,Time from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, start_time, end_time), con)
        plt.plot(df['Time'][100:], df['CPS'][100:], colors[i])
    plt.xlabel('Time')
    plt.ylabel('Count rate (cps')
    fig.savefig('{}_plot_cps.png'.format(output_prefix))
    plt.close()
    return

def plot_spectra(con, start_time, end_time, output_prefix):
    bin_number = 2**14

    colors = ['r.', 'g.', 'b.', 'm.']
    fig = plt.figure()
     
    for i in range(4):
        df = pd.read_sql_query('SELECT Spectrum_Array from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, start_time, end_time), con)
        spectrum = df['Spectrum_Array'].apply(lambda x: np.array([float(lol) for lol in x.split(',')]))
        spectrum = spectrum.sum()
        # stuff = rebin(spectrum, 4096)
        if i < 2:
            # plt.plot(spectrum[:4096], colors[i])
            plt.plot(spectrum, colors[i])
        else:
            # stuff = rebin(spectrum, 4096)
            plt.plot(spectrum, colors[i])
    plt.xlabel('Channel number')
    plt.ylabel('Counts')
    fig.savefig('{}_plot_spectra.png'.format(output_prefix))
    plt.close()
    return

def rebin(data, new_bin_number):
    [counts, bin_edges] = np.histogram(data, bins=new_bin_number)
    return counts

def plot_gps(con, start_time, end_time, output_prefix):
    # now = int(time.time())
    df = pd.read_sql_query('SELECT Time, Latitude, Longitude FROM gps WHERE Time > "{}" AND Time <= "{}";'
                           .format(start_time, end_time), con)
    x = list(df['Latitude'])
    y = list(df['Longitude'])
    time = list(df['Time'])

    fig = plt.figure()
    plt.plot(x, y)
    plt.xlabel('Latitude')
    plt.ylabel('Longitude')
    fig.savefig('{}_plot_gps.png'.format(output_prefix))
    plt.close()

    fig = plt.figure()
    plt.plot(time, x, 'r.')
    plt.plot(time, y, 'b.')
    plt.xlabel('Time')
    plt.ylabel('Position')
    fig.savefig('{}_plot_gpsvstime.png'.format(output_prefix))
    plt.close()
    return 

def main():
    con = sqlite3.connect("./cvrs/CVRS_local.sqlite3")
    output_prefix = 'cvrs'
    start_time, end_time = get_alltime(con)
    # start_time = 89E9
    plot_cps(con, start_time, end_time, output_prefix)
    plot_spectra(con, start_time, end_time, output_prefix)
    plot_gps(con, start_time, end_time, output_prefix)

    con = sqlite3.connect("./ptu/PTU_local.sqlite3")
    output_prefix = 'ptu'
    start_time, end_time = get_alltime(con)
    # start_time = 89E9
    plot_cps(con, start_time, end_time, output_prefix)
    plot_spectra(con, start_time, end_time, output_prefix)
    plot_gps(con, start_time, end_time, output_prefix)
    return

main()
