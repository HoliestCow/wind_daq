
import time
import io
import numpy as np

import sys
sys.path.append('../')

from database import DatabaseOperations


def initialize_measurement_threads(database_filename, gamma_filenames, video_filename):
    dbo = DatabaseOperations(database_filename)
    gammaFileHandles = initialize_gammaFiles(gamma_filenames)
    videoFileHandle = initialize_video(video_filename)  # There's another argument that's returned here that I forgot about.
    # No need for GPS or environment stuff, because I have libraries to handle that.
    q = Queue()
    sqldump_thread = threading.Thread(group=None, target=thread_handler, name='thread_handler', args=(dbo, gammaFilehandles, videoFileHandle))
    thriftpackage_thread = threading.Thread(group=None, target=
    thread.start()
    return dbo, thread

# There needs to be a thread to handle the packaging, that was the whole fucking point.

def thread_handler(dbo, gammaFilehandles, videoFilehandles):
    time.sleep(1.0)
    rad_data = radiation_catch(gammaFilehandles)
    gps_data = gps_catch()
    temp_data = environment_catch()
    video_catch(videoFilehandles)
    dump_data(dbo, rad_data, gps_data, temp_data)  # formerly known as dumptodb
    return

def radiation_catch(dbo, targetFile):
    # Input: database object and csv filename
    # NOTE: This will change once I implement the CAEN libraries where I'll basically be polling the catch_measurements
    # NOTE: This function will never return. Not sure if this is shitty or not.

    ### CSV seeker object calls go here. Somehow. ###
    return


def gps_catch(dbo):
    # Input: database object

    return


def environment_catch(dbo):
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
    targetFilePattern = '../data/bullshit_'
    isInitialize = True
    db = DatabaseOperations('./PTU_local.sqlite3')
    db.initialize_structure(numdetectors=4)

if __name__ == "__main__":
    main()
