
import time
import io
import numpy as np

import sys
sys.path.append('../')

from database import DatabaseOperations


def dumptodb(db, t, datum):
    print('bathroom')
    hacked_spectrum = datum.tolist()
    hacked_spectrum = [str(x) for x in hacked_spectrum]
    hacked_spectrum = ','.join(hacked_spectrum)
    #  (I'm going off of the train dataset)
    desired_outputs = (t,  # this is timestamp
                       0,  # PositionID
                       hacked_spectrum,
                       np.sum(datum),  # cps
                       1,  # isAlive
                       0,  # Energy Cubic a  Not sure where to get this data from. Fuck it.
                       0,  # Energy Cubic b
                       0,  # Energy Cubic c
                       'Spectrum',  # Data type
                       t,
                       0,  # Upper ROI counts
                       t,
                       0,  # Lower ROI counts
                       0,  # Fine Gain
                       0.1,  # Response Time
                       600,  # High voltage
                       t,
                       -2,  # TickDelta
                       0,  #  TickNumber
                       0,  # Coarse gain
                       0)  # Energy cubic d
    db.fake_stack_datum(desired_outputs)
    print('dumped')
    print(t)
    return


def main():
    targetFile = 'raw_stream_data.dat'
    isInitialize = True
    db = DatabaseOperations('./ptu_db.sqlite3')
    db.initialize_structure(numdetectors=1)
    counter = 0
    with io.open(targetFile, 'r', buffering=1) as f:
        f.seek(0, 2) # Go to the end of the file
        t = 0
        juice = []
        while True:
            line = f.readline().strip()
            if not line:
                time.sleep(0.01)  # Sleep for 10 milliseconds
                continue
            if isInitialize:
                isInitialize = False
                words = line.split()
                prev_time = float(words[0])
            words = line.split()
            x = float(words[0])
            counter += x
            charge = float(words[1])
            if counter >= 1:
                counts, bin_edges = np.histogram(np.array(juice), bins=1024, range=(0, 28000))
                # bin_edges = bin_edges[1:]
                dumptodb(db, t, counts)
                juice = []
                t += 1
                counter = 0
            juice += [float(charge)]

if __name__ == "__main__":
    main()
