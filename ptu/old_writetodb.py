
import time
import io
import numpy as np

import sys
sys.path.append('../')

from database import DatabaseOperations

def initialize_database(database_filename='test.sqlite3'):
    dbo = DatabaseOperations(database_filename)
    return dbo


def initialize_threads(dbo):
    threads = {}
    radiation_thread = threading.Thread(group=None, target=catch_radiation, name='radiation_thread', args=(dbo, ))
    gps_thread = threading.Thread(group=None, target=catch_gps, name='gps_thread', args=(dbo, ))
    environment_thread = threading.Thread(group=None, target=catch_environment, name='environment_thread', args=(dbo, ))

    threads['radiation'] = radiation_thread
    threads['gps'] = gps_thread
    threads['environment'] = environment_thread

    threads['radiation'].start()
    threads['gps'].start()
    threads['environment'].start()
    return threads


def catch_radiation(dbo):
    # Input: database object

    return


def catch_gps(dbo):
    # Input: database object

    return


def catch_environment(dbo):
    # NOTE: This is actually the GPS sensor since it has a thermometer on there already.
    # Input: database object

    return


def catch_video(video_filename='test.avi'):

    return


def dumptodb(db, datum):
    hacked_spectrum = datum.tolist()
    hacked_spectrum = [str(x) for x in hacked_spectrum]
    hacked_spectrum = ','.join(hacked_spectrum)
    hacked_spectrum = '\"' + hacked_spectrum + '\"'
    #  (I'm going off of the train dataset)
    # desired_outputs = (int(time.time() * 1000),  # this is timestamp
    desired_outputs = (1,  # Position ID
                       hacked_spectrum,
                       np.sum(datum))  # cps
    db.fake_stack_datum(desired_outputs)
    print('dumpingtodb')
    return


def main():
    targetFile = './data/raw_stream_data.dat'
    isInitialize = True
    db = DatabaseOperations('./PTU_local.sqlite3')
    db.initialize_structure(numdetectors=1)
    counter = 0
    with io.open(targetFile, 'r', buffering=1) as f:
        f.seek(0, 2)  # Go to the end of the file
        t = int(time.time())
        juice = []
        while True:
            line = f.readline().strip()
            if not line:
                continue
            if isInitialize:
                isInitialize = False
                words = line.split()
                prev_time = float(words[0])
            print(line)
            words = line.split()
            if len(words) != 2:
                # I'm throwing away data here. The trouble with stream data
                continue
            x = float(words[0])
            counter += x
            charge = float(words[1])
            if counter >= 1:
                counts, bin_edges = np.histogram(np.array(juice), bins=1024, range=(0, 28000))
                # bin_edges = bin_edges[1:]
                # dumptodb(db, t * 1000, counts)
                dumptodb(db, counts)
                juice = []
                t += 1
                counter = 0
            juice += [float(charge)]

if __name__ == "__main__":
    main()
