
import time
import io
import numpy as np

import sys
sys.path.append('../')

from database import DatabaseOperations


def initialize(database_filename, gamma_filenames, video_filename):
    dbo = DatabaseOperations(database_filename)
    gammaFileHandles = initialize_gammaFiles(gamma_filenames)
    videoFileHandle = initialize_video(video_filename)  # There's another argument that's returned here that I forgot about.
    # No need for GPS or environment stuff, because I have libraries to handle that.
    q = Queue()
    sqldump_thread = threading.Thread(group=None, target=thread_handler, name='thread_handler', args=(dbo, gammaFilehandles, videoFileHandle))
    # thriftpackage_thread = threading.Thread(group=None, target=
    sqldump_thread.start()
    return dbo, thread

# There needs to be a thread to handle the packaging, that was the whole fucking point.

def thread_handler(dbo, gammaFilehandles, videoFilehandles):
    time.sleep(1)
    rad_data = db_pull()
    current = []
    dump_data(dbo, rad_data, gps_data, temp_data)  # formerly known as dumptodb
    return goodies

def radiation_catch(dbo, targetFile):
    # Input: database object and csv filename
    # NOTE: This will change once I implement the CAEN libraries where I'll basically be polling the catch_measurements
    # NOTE: This function will never return. Not sure if this is shitty or not.

    now = int(time.time() * 1000)

    out = []
    desired = db.get_dataSince(now - lasttime * 1000, now)
    # current = desired[-1]
    current = desired
    if current is not None:
        # Only accept the last row, see the note above.
        for row in current:
            spectrum = row[2].split(',')
            intSpectrum = [int(x) for x in spectrum]
            timestamp = int(row[0])
            # realtime = int(current[-5])
            # livetime = int(current[-10])
            realtime = 1
            livetime = 1

            integerSpectrum = Spectrum(
                spectrumInt=intSpectrum,
                format=SpectrumFormat.ARRAY,
                channelCount=len(intSpectrum),
                liveTime=livetime)

            spectrumResult = SpectrumResult(
                intSpectrum=integerSpectrum)

            out += [GammaSpectrumData(
                componentId=uuid_dict['GammaDetector'],
                timeStamp=timestamp,
                health=Health.Nominal,
                spectrum=spectrumResult,
                liveTime=livetime,
                realTime=realtime)]
    else:
        out += [GammaSpectrumData(
            componentId=uuid_dict['GammaDetector'],
            timeStamp=int(time.time() * 1000),
            health=Health.Nominal)]
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
