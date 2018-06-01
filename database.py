
import numpy as np
import sqlite3
import time

class DatabaseOperations(object):
    def __init__(self, filename):
        # check if the file exists:
        self.filename = filename
        self.conn = sqlite3.connect(self.filename)
        self.c = self.conn.cursor()

    def initialize_structure(self, config=None, numdetectors=None):
        # with unit configuration, initialize the entire configuration
        # I may need much more than config to be fair.

        self.c.execute('''CREATE TABLE IF NOT EXISTS det_0(Time integer, PositionId integer, Spectrum_Array text, CPS real)''')
        # if config is not None:
        #     for i in range(len(config.energyCalibration)):
        #         self.c.execute('''CREATE TABLE IF NOT EXISTS det_{} (Time integer, PositionId integer, Spectrum_Array text, CPS real)'''.format(i))
        # elif numdetectors is not None:
        #     for i in range(numdetectors):
        #         self.c.execute('''CREATE TABLE IF NOT EXISTS det_{} (Time integer, PositionId integer, Spectrum_Array text, CPS real)'''.format(i))

    def fake_stack_datum(self, stuff):
        self.c.execute("INSERT INTO det_0(Time, PositionId, Spectrum_Array, CPS) VALUES ({}, {}, {}, {});".format(int(time.time() * 1000), stuff[0], stuff[1], stuff[2]))
        self.conn.commit()
        return

    def stack_datum(self, datum, config):
        print('stacking datum')
        if datum.gammaSpectrumData is None:
            print('no data')
            return
        c = time.time()
        for i in range(len(datum.gammaSpectrumData)):
        # print(datum.gammaSpectrumData[i])
            extracted_array = [str(item) for item in datum.gammaSpectrumData[i].spectrum.intSpectrum.spectrumInt]
            extracted_array = ','.join(extracted_array)
            extracted_array = '\"' + extracted_array + '\"'
            timestamp = float(datum.gammaSpectrumData[i].timeStamp)
            positionid = 1  # PositionID (I'm going off of the train dataset
            cps = datum.gammaGrossCountData[i].counts
            # cps = float(np.sum(np.array(extracted_array)) / \
            #     float(datum.gammaSpectrumData[i].liveTime))  # CPS
            # print(cps)
            # self.c.execute("INSERT INTO det_{}(Time, Spectrum_Array, CPS) \
            #                 VALUES({}, {}, {});".format(
            # self.c.execute("INSERT INTO det_{}(Time, PositionId, CPS, Spectrum_Array) VALUES ({}, {}, {}, {});".format(i, timestamp, positionid, cps, text_array))
            self.c.execute("INSERT INTO det_0(Time, PositionId, CPS, Spectrum_Array) VALUES ({}, {}, {}, {});".format(timestamp, positionid, cps, extracted_array))
        self.conn.commit()
        d = time.time()
        print('DB write: {}s'.format(d-c))
        return

    def get_dataSince(self, start, stop):
        # NOTE: This is messed up because I'm not quite sure how to pick detectors yet.
        command = "SELECT * FROM det_0 WHERE Time <= {} AND Time >= {}".format(stop, start)
        desired = self.c.execute(command)
        # current = desired.fetchall()  # note sure if fetchall or fetchone
        current = desired.fetchall()
        return current