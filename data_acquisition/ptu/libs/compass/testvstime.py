from readout import CompassReadout 
def main():

    # readout = CompassReadout('../../testing_data/')
    # readout = CompassReadout('/home/holiestcow/Documents/winds/thrift/wind_daq/data_acquisition/ptu/testing_data/run/UNFILTERED/')
    readout = CompassReadout('../../../../../caen_drivers_programs/CoMPASS-1.3.0/projects/wind/DAQ/run/UNFILTERED/measurement_archive/')
    # readout.update_measurement()
    readout.plot_all_measurements()
    return

main()
