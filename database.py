import numpy as np
import sqlite3

class DatabaseOperations(object):
    def __init__(self, filename):
        # check if the file exists:
        self.conn = sqlite3.connect(filename)
        self.c = self.conn.cursor()

    def initialize_structure(self, config=None, numdetectors=None):
        # with unit configuration, initialize the entire configuration
        # I may need much more than config to be fair.

        if config is not None:
            for i in range(len(config.energyCalibration)):
                self.c.execute('''CREATE TABLE det_{} (Time real, PositionId integer, Spectrum_Array text, CPS real,
                                  isAlive integer, Energy_cubic_a  real, Energy_cubic_b real, Energy_cubic_c real,
                                  DataType text, Timestamp real, Upper_ROI_Counts real, live_time real,
                                  Lower_ROI_Counts real, fine_gain real, response_time real, high_voltage integer,
                                  real_time real, tickdelta integer, tick integer, coarse_gain integer,
                                  energy_cubic_d integer)'''.format(i))
        elif numdetectors is not None:
            for i in range(numdetectors):
                self.c.execute('''CREATE TABLE det_{} (Time real, PositionId integer, Spectrum_Array text, CPS real,
                                  isAlive integer, Energy_cubic_a  real, Energy_cubic_b real, Energy_cubic_c real,
                                  DataType text, Timestamp real, Upper_ROI_Counts real, live_time real,
                                  Lower_ROI_Counts real, fine_gain real, response_time real, high_voltage integer,
                                  real_time real, tickdelta integer, tick integer, coarse_gain integer,
                                  energy_cubic_d integer)'''.format(i))

    def fake_stack_datum(self, stuff):
        self.c.execute("INSERT INTO det_0 VALUES {}".format(stuff))
        self.conn.commit()
        return

    def stack_datum(self, datum, config):
        for i in range(len(datum.gammaSpectrumData)):
            extracted_array = [float(x) for item in datum.gammaSpectrumData[i].intSpectrum]
            desired_outputs = (datum.gammaSpectrumData[i].timeStamp,
                               0,  # PositionID (I'm going off of the train dataset
                               datum.gammaSpectrumData[i].spectrum.intSpectrum,
                               np.sum(np.array(extracted_array)) /
                               datum.gammaSpectrumData[i].liveTime,  # cps
                               1,  # isAlive
                               0,  # Energy Cubic a  Not sure where to get this data from. Fuck it.
                               0,  # Energy Cubic b
                               0,  # Energy Cubic c
                               'Spectrum',  # Data type
                               datum.gammaSpectrumData[i].timeStamp,
                               0,  # Upper ROI counts
                               datum.gammaSpectrumData[i].liveTime,
                               0,  # Lower ROI counts
                               0,  # Fine Gain
                               0.1,  # Response Time
                               600,  # High voltage
                               datum.gammaSpectrumData[i].realTime,
                               -2,  # TickDelta
                               0,  #  TickNumber
                               0,  # Coarse gain
                               0)  # Energy cubic d
            self.c.execute("INSERT INTO det_{} VALUES {}".format(i, desired_outputs))
            self.conn.commit()
        return

    def get_dataSince(self, start, stop):
        # NOTE: This is messed up because I'm not quite sure how to pick detectors yet.
        desired = self.c.execute("SELECT * FROM det_0 WHERE Time <= {} AND Time > {}".format(stop, start))
        return desired