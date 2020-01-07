from readout import CompassReadout

def main():
    readout = CompassReadout('../../testing_data/')
    readout.update_measurement()
    print(readout.current_measurement)
    print(readout.latest_datetimes)
    return

main()
