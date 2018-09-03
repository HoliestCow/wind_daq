
import numpy as np
import sqlite3
import time


class DatabaseOperations(object):
    def __init__(self, filename):
        # check if the file exists:
        self.filename = filename
        self.conn = sqlite3.connect(self.filename)
        self.c = self.conn.cursor()

    def initialize_structure(self, numdetectors=None):
        # with unit configuration, initialize the entire configuration
        # I may need much more than config to be fair.

        # QUESTION: Should CPS be real or int?
        # QUESTION: Should long and lat be real or int?
        if numdetectors is not None:
            for i in range(numdetectors):
                self.c.execute('''CREATE TABLE IF NOT EXISTS det_{}(Time integer, PositionId integer, Spectrum_Array text, CPS real)'''.format(i))
        else:
            self.c.execute('''CREATE TABLE IF NOT EXISTS det_0(Time integer, PositionId integer, Spectrum_Array text, CPS real)'''.format(i))



        self.c.execute('''CREATE TABLE IF NOT EXISTS gps(Time integer, Longitude real, Latitude real, NumberofSatellites integer)''')

    def fake_stack_datum(self, stuff, tablename):
        self.c.execute("INSERT INTO {}(Time, PositionId, Spectrum_Array, CPS) \
            VALUES ({}, {}, {}, {});".format(
            # tablename, int(time.time() * 1000), stuff[0], stuff[1], stuff[2]))
            tablename, stuff[0], stuff[1], stuff[2], stuff[3]))
        self.conn.commit()
        return

    def stack_datum(self, datum):
        gamma_flag = True
        gps_flag = True
        print('stacking datum')
        if datum is None:
            print('no data')
            gamma_flag = False
            gps_flag = False
        elif datum.gammaSpectrumData is None:
            print('no data in gammaSpec')
            gamma_flag = False
        elif datum.navigationData is None:
            print('no gps data in datum')
            gps_flag = False
        print(datum)
        c = time.time()
        if gamma_flag:
            for i in range(len(datum.gammaSpectrumData)):
            # print(datum.gammaSpectrumData[i])
                # temp_array = np.array(datum.gammaSpectrumData[i].spectrum.intSpectrum.spectrumInt)
                # np.save('spectra_det{}_t{}'.format(i, time.time()), temp_array)
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
                self.c.execute("INSERT INTO det_{}(Time, PositionId, CPS, Spectrum_Array) VALUES ({}, {}, {}, {});".format(i, timestamp, positionid, cps, extracted_array))
        if gps_flag:
            print('======\n======\nIHAVEGPSDATA\n========\n=======')
            for i in range(len(datum.navigationData)):
                piece = datum.navigationData[i]
                timestamp = piece.timeStamp
                # NOTE: eventually number of sats
                # numSatellites = piece.numberOfSatellites

                location = piece.location
                latitude = location.latitude  # double
                longitude = location.longitude  # double

                self.c.execute("INSERT INTO gps(Time, Latitude, Longitude) VALUES ({}, {}, {});".format(
                    timestamp, latitude, longitude))

        self.conn.commit()
        d = time.time()
        print('DB write: {}s'.format(d - c))
        return

    def get_dataSince(self, start, stop):
        # NOTE: This is messed up because I'm not quite sure how to pick detectors yet.
        command = "SELECT * FROM det_0 WHERE Time <= {} AND Time >= {}".format(stop, start)
        desired = self.c.execute(command)
        # current = desired.fetchall()  # note sure if fetchall or fetchone
        current = desired.fetchall()
        return current