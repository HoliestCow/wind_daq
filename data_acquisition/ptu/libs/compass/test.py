from readout import CompassReadout 
def main():

    # readout = CompassReadout('../../testing_data/')
    readout = CompassReadout('/home/holiestcow/Documents/winds/thrift/wind_daq/data_acquisition/ptu/testing_data/run/UNFILTERED/')
    readout.update_measurement()
    return

main()
