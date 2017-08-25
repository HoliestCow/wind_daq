# This code looks for the file that the digitizer is writing to and pulls data from it generating arrays of list mode data
# checks pipe for new data every x number of seconds (bufferTime variable)
# outputs:
# array of triggerTime which is number of clock ticks (4ns/click) since acquisition started
# array of qlong which the total charge in counts deposited into the ADC for each event
# an array of "extra" which is something from CAEN (who knows)
# an array of qshort which is the total charge in counts deposited in the short integration window used for PSD
# The digitizer MUST be running before executing this code


#import the things
import logging
import os
import time


def main():
    # Turn on logging so can get some debugging action
    logging.basicConfig(format='%(asctime)s %(message)s')
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)
    log.debug('Logging has started')


    # Name of pipe (e.g. inputFile or inputFile.txt or inputFile.dat)
    FIFO = 'inputFile'

    # Amount of time in seconds want to wait before checking pipe for more data
    bufferTime = 2

    #check if the acquisition is writing to the pipe
    if not os.path.exists(FIFO):
        log.debug('Start acquistion please, quitting now')
        # got rid of this because was seeing weird behavior if python creates a pipe that something else tries to write to later
        #os.mkfifo(FIFO) #make pipe
        #log.debug('Pipe is made')
    else:
        log.debug('Acquistion running, pipe present: OK')

    log.debug('Pipe is open for reading')
    with open(FIFO, 'r') as fifo:
        newPipe= True # init boolean for checking whether pipe is new

        # initialize empty arrays
        triggerTimeTag = []
        qlong = []
        extras = []
        qshort = []

        while True:
            data = fifo.read().splitlines() #split incoming data into lines based on carriage return


            # CAEN writes files with 6 header lines, checks if file is new and if so discard headers
            if newPipe is True:
                data = data[6:]
                newPipe = False

            #uncomment for debugging (check whether data coming in)
            #print("Received Data: " + str(data))
            #print('lololololololol')

            for line in data:
                words = line.split()  # split line into space delimited words

                #build 1D arrays with list mode data
                triggerTimeTag.append(int(words[0]))  # trigger time in clock units (4ns/tick)
                qlong.append(int(words[1]))  # total integrated charge of event in counts
                extras.append(int(words[2]))  # have no idea what this is but CAEN puts it in output file
                qshort.append(int(words[3]))  # integrated charge in counts of short window, useful for PSD

                #uncomment for debugging
                #print("Qlong is: " + str(qlong))
            time.sleep(bufferTime) # wait x number of seconds to check for data coming in on pipe
    return

main()
