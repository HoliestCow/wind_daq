
import numpy as np
import matplotlib.pyplot as plt
import os
from readout import CompassReadout
import glob

def main():
    readout = CompassReadout('../../../../../caen_drivers_programs/CoMPASS-1.3.0/projects/wind/DAQ/run/UNFILTERED/')
    readout.process_files(glob.glob(os.path.join(readout.readout_dir, '*.txt')))
    for key in readout.data:
        channel_data = readout.data[key]
        timeline = []
        counts = []
        for time in channel_data:
            counts += [channel_data[time]['counts']]
            timeline += [time]
        fig = plt.figure()
        plt.plot(timeline, counts, '.')
        fig.savefig('channel{}.png'.format(key))
        plt.close()
    return

main()
