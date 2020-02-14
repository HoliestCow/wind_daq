
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

def plot_cps(con, start_time, end_time):
    colors = ['r.', 'g.', 'b.', 'm.']
    fig = plt.figure()
    for i in range(4):
        df = pd.read_sql_query('SELECT CPS,Time from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, start_time, end_time), con)
        plt.plot(df['Time'], df['CPS'], colors[i])
    plt.xlabel('Time')
    plt.ylabel('Count rate (cps')
    fig.savefig('plot_cps.png')
    plt.close()
    return

def plot_spectra(con, start_time, end_time):
    bin_number = 2**14

    con = sqlite3.connect("./CVRS_local.sqlite3")
    colors = ['r.', 'g.', 'b.', 'm.']
    fig = plt.figure()
     
    for i in range(4):
        df = pd.read_sql_query('SELECT Spectrum_Array from det_{} where Time > "{}" AND Time <= "{}";'
                               .format(i, start_time, end_time), con)
        spectrum = df['Spectrum_Array'].apply(lambda x: np.array([float(lol) for lol in x.split(',')]))
        spectrum = spectrum.sum()
        # stuff = rebin(spectrum, 4096)
        if i < 2:
            plt.plot(spectrum[:4096], colors[i])
        else:
            stuff = rebin(spectrum, 8192)
            plt.plot(stuff[:4096], colors[i])
    plt.xlabel('Channel number')
    plt.ylabel('Counts')
    fig.savefig('plot_spectra.png')
    plt.close()
    return

def rebin(data, new_bin_number):
    [counts, bin_edges] = np.histogram(data, bins=new_bin_number)
    return counts

def plot_gps(con, start_time, end_time):
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
    fig.savefig('plot_gps.png')
    plt.close()

    fig = plt.figure()
    plt.plot(time, x, 'r.')
    plt.plot(time, y, 'b.')
    plt.xlabel('Time')
    plt.ylabel('Position')
    fig.savefig('plot_gpsvstime.png')
    plt.close()
    return 

def main():
    con = sqlite3.connect("./CVRS_local.sqlite3")
    start_time, end_time = get_alltime(con)
    plot_cps(con, start_time, end_time)
    plot_spectra(con, start_time, end_time)
    plot_gps(con, start_time, end_time)
    return

main()
