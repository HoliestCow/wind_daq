import logging
import os
# import struct
import time

def main():
    logging.basicConfig(format='%(asctime)s %(message)s')  # filename='DocTestLog')
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)
    log.debug('Logging has started')

    FIFO = 'inputFile'

    if not os.path.exists(FIFO):
        log.debug('Pipe not present...Creating...')
        os.mkfifo(FIFO)
        log.debug('Pipe is made')
    else:
        log.debug('Pipe already present')

    # os.mkfifo(FIFO) #make the named pipe

    # with open(FIFO,'r') as fifo: # rb for a binary file, r just for an asci
    #    data=fifo.read()

    with open(FIFO, 'r') as fifo:
        log.debug('Pipe is open for reading')
        tryCount = 0
        while True and tryCount < 30:
            try:
                # TODO: slip in read_fifo()
                # data = fifo.readlines()#[5:] #first six lines are headers, ignore
                data = fifo.read()
                for line in data:
                    print("Received: " + data)
                    words = line.split()  # split line into space delimited words
                    triggerTimeTag = words[0]  # trigger time in clock units (4ns/tick)
                    qlong = words[1]  # total integrated charge of event in counts
                    extras = words[2]  # have no idea what this is but CAEN puts it in output file
                    qshort = words[3]  # integrated charge in counts of short window, useful for PSD
                    print("Qlong is: " + qlong)
            except:
                # TODO: slip in time.sleep(2), erase everything in the brackets
                # looks for data in pipe for one minute before quitting
                # {
                tryCount = 0
                while (tryCount < 30):
                    time.sleep(2)  # waits for 2 seconds to see if more data is coming in
                    # tries again to find data
                    try:
                        # data = fifo.readlines()#[5:] #first six lines are headers, ignore
                        data = fifo.read()
                        for line in data:
                            print("Received: " + data)
                            words = line.split()  # split line into space delimited words
                            triggerTimeTag = words[0]  # trigger time in clock units (4ns/tick)
                            qlong = words[1]  # total integrated charge of event in counts
                            extras = words[2]  # have no idea what this is but CAEN puts it in output file
                            qshort = words[3]  # integrated charge in counts of short window, useful for PSD
                            print("Qlong is: " + qlong)
                    except:
                            log.debug('Waiting for more data in pipe')
                            continue
                    tryCount += 1

                log.debug('Pipe no longer active...quitting')
                break
                # }
    return


def read_fifo(fifo_handle, logger):
    tryCount = 0
    while (tryCount < 30):
        time.sleep(2)  # waits for 2 seconds to see if more data is coming in
        # tries again to find data
        try:
            # data = fifo.readlines()#[5:] #first six lines are headers, ignore
            data = fifo_handle.read()
            for line in data:
                print("Received: " + data)
                words = line.split()  # split line into space delimited words
                triggerTimeTag = words[0]  # trigger time in clock units (4ns/tick)
                qlong = words[1]  # total integrated charge of event in counts
                extras = words[2]  # have no idea what this is but CAEN puts it in output file
                qshort = words[3]  # integrated charge in counts of short window, useful for PSD
                print("Qlong is: " + qlong)
        except:
                logger.debug('Waiting for more data in pipe')
                continue
        tryCount += 1

    logger.debug('Pipe no longer active...quitting')
    return  # return data here.

main()
