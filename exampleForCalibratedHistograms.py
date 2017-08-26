# reads in list mode data for offline (not live) file
# takes calibration coefficients m and b for linear calibration
# converts time from clock ticks to ns
# makes calibrated 1 and 2D (time evolved) histograms and plots them if uncomment lines (starts at 102)


import logging
logging.basicConfig(format='%(asctime)s %(message)s')
log = logging.getLogger()
log.setLevel(logging.DEBUG)
log.debug('Logging has started')

import os
#import struct
import numpy as np
import matplotlib.pyplot as plt
import time
import math
import csv



def main():

    # file want to read in
    FIFO='nick137_004_ls_1.dat'#'testerino_005_ls_0.dat'#'inputFile'

    # number of bins in histogram, because using calibration its 3000 keV, if uncalibrated use 2**16
    num_bins = 3000 #3MeV

    #calibration factors, assuming linear calibration y=mx+b
    m = 0.2
    b = 0


    # read in the file as pipe
    if not os.path.exists(FIFO):
        log.debug('File not present...quitting')
        #os.mkfifo(FIFO)
        #log.debug('Pipe is made')
    else:
        log.debug('File located!')


    #make some empty arrays to fill
    triggerTimeTag = []
    qlong = []
    extras = []
    qshort = []

    # initialize variable for starting time of acquistion= 0
    initTime = 0

    with open(FIFO,'r') as fifo:
        log.debug('File is open for reading')

        #dont seem to be explicit newline characters so have to split again
        #data is split into individual lines and first 6 lines are skipped because those are headers
        data=fifo.read().splitlines()
        data = data[6:]

        for line in data:
            words = line.split()

            if len(triggerTimeTag) == 0:
                initTime = float(words[0])
                print(initTime)

            # trigger time is in clock clicks 4ns/tick, seems to start as some insanely large number so subtracting out as initial time
            # also putting trigger time in seconds instead of clock ticks
            triggerTimeTag.append((float(words[0])-initTime)/4)  # trigger time in ns (4ns/clock tick)
            qlong.append((float(words[1])*m) + b)  # total integrated energy of event in rough keV
            extras.append(int(words[2]))  # have no idea what this is but CAEN puts it in output file
            qshort.append(int(words[3]))  # integrated charge in counts of short window, useful for PSD
            #print(qlong) #uncomment for debugging
        # close the file because reasons
        fifo.close()
        log.debug('File fully read') #for checking to make sure that it doesnt take obscenely long to read the file
        #print(triggerTimeTag[3]) #uncomment for debugging


    #buid a histogram that can be called later
    histQlong, bin_edges = np.histogram(qlong, bins = range(num_bins))
    #print(histQlong) #uncomment for debugging


    # make max time want to plot in 100's of ms for proper binning
    maxTime = int(math.ceil(triggerTimeTag[len(triggerTimeTag)-1])/100000)
    #print(maxTime) #uncomment for debugging

    #build 2D histogram with energy, time, counts (x,y,z)
    # make 2D histogram, bin (with xedges and yedges) to 100ms/division for time
    histTime, xedges, yedges = np.histogram2d(qlong,triggerTimeTag, bins = [num_bins, maxTime])
    #print(histTime[11]) #uncomment for debugging
    #what (i think) this outputs its the energy histogram for each time bin

    #uncomment write the 2D histogram to file to check it out
    #writer = csv.writer(open("histFile", 'w'))
    #for row in histTime:
    #    writer.writerow(row)

    # uncomment to plot 1D histogram
    #plot histogram from qlong
    #plt.hist(qlong, bins = range(num_bins))
    #plt.show()

    # this doesnt work i'm not sure why
    #plt.hexbin(qlong,triggerTimeTag, bins =  [num_bins, maxTime])
    #plt.show()

    #save png of 1D histogram if want to
    #plt.savefig('qlong.png')
    return

main()
