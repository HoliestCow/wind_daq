
import numpy as np
import sqlite3
import time
from wind_daq.windthrift.PTUPayload.ttypes import SystemDefinition


class DatabaseOperations(object):
    def __init__(self, filename):
        # check if the file exists:
        self.filename = filename
        self.conn = sqlite3.connect(self.filename)
        self.c = self.conn.cursor()

    # def initialize_structure(self, numdetectors=None):
    def initialize_structure(self, systemdefinition):
        self.uuid2table = {}
        # with unit configuration, initialize the entire configuration
        # I may need much more than config to be fair.

        # QUESTION: Should CPS be real or int?
        # QUESTION: Should long and lat be real or int?
        # if numdetectors is not None:
        #     for i in range(numdetectors):
        #         self.c.execute('''CREATE TABLE IF NOT EXISTS det_{}(Time
        #                        integer, PositionId integer, Spectrum_Array
        #                        text, CPS real)'''.format(i))
        # else:
        #     self.c.execute('''CREATE TABLE IF NOT EXISTS det_0(Time integer,
        #                    PositionId integer, Spectrum_Array text,
        #                    CPS real)'''.format(i))
        # Here I am creating the spectrum tables using the system definition.
        for i in range(len(systemdefinition.gammaSpectrumDefinitions)):
            self.c.execute('''CREATE TABLE IF NOT EXISTS det_{}(Time integer,
                           PositionId integer, Spectrum_Array text, CPS
                           real)'''.format(i))

        self.c.execute('''CREATE TABLE IF NOT EXISTS gps(Time integer,
                       Longitude real, Latitude real,
                       NumberofSatellites integer)''')

        self.c.execute('''CREATE TABLE IF NOT EXISTS videofusion(Time integer,
                       detection_image_filename string,
                       tracking_image_filename_prefix string,
                       tracking_image_startindex integer,
                       tracking_image_stopindex integer,
                       fusion_result_image_filename string)''')
        # There are two videostreams, how do I know which one is siamfc and the
        # other is deteection
        if systemdefinition.contextStreamDefinitions is not None:
            for i in range(len(systemdefinition.contextStreamDefinitions)):
                piece = systemdefinition.contextStreamConfigurations[i]
                self.uuid2table[piece.componentId] = {
                    'streamAddress': piece.streamAddress,
                    'configuration': piece.configuration,
                    'component': piece.component,
                    'start_time': time.time()}
        self.lasttime = {}
        self.lasttime['siamfc_stream'] = 0
        self.lasttime['yolo_stream'] = 0

    def stack_datum(self, datum):
        gamma_flag = True
        gps_flag = True
        camera_flag = False
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
        elif datum.streamIndexData is None:
            print('no videostreaming data in datum')
            camera_flag = False
        # print(datum)
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

        if camera_flag:
            print('========\n======\nIHAVECAMERAFOTO\n===========\n======')
            for i in range(len(datum.streamIndexData)):
                piece = datum.streamIndexData[i]
                # determine if this is siamfc or yolo
                address = self.uuid2table[piece.componentId]['streamAddress']
                if re.search('siam', address):
                    # figure out how many photos have been taken in this
                    # period.
                    tracking_image_startindex = self.lasttime['siamfc_stream']
                    tracking_image_stopindex = int(piece.streamTimeStamp)
                    self.lasttime['siamfc_stream'] = piece.streamTimeStamp + 1
                    tracking_image_prefix = address
                elif re.search('yolo', address):
                    detection_image = '{}_{}.jpg'.format(address,
                                                         int(piece.streamTimeStamp))
            timestamp = int(time.time())
            fusion_result_image = '{}_{}_{}.jpg'.format(
                'fusionresult',
                tracking_image_prefix,
                tracking_image_stopindex)

            self.c.execute("INSERT INTO videofusion(Time,\
                           detection_image_filename, tracking_image_filename_prefix, tracking_image_startindex, tracking_image_stopindex, fusion_result_image_filename) VALUES ({}, {}, {}, {}, {}, {});".format(timestamp,
                                           detection_image,
                                           tracking_image_prefix,
                                           tracking_image_startindex,
                                           tracking_image_stopindex,
                                           fusion_result_image))

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
