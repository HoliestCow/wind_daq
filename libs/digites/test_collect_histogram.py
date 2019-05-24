
import numpy as np
from caenlib import get_histograms
import threading
import time
import matplotlib.pyplot as plt


def main():
    # nbin = 2 ** 12
    nbin = 2**14
    measurement_time = 60  # 60 seconds.
    # measurement_spool(state, short_data, long_data, size)
    state = np.array([1], dtype=np.int32)
    # I have to check if it's always 4. I'm not sure if enabled output will make this fit the number of active channels.
    array_size = (4, nbin)
    long_data = np.zeros(array_size, dtype=np.uint32)

    time_array_size = (4, nbin, 1)

    long_data_thru_time = np.zeros(time_array_size, dtype=np.uint32)
    try:
        t1 = threading.Thread(target=measurement_spool,
                              args=(state))
        t1.start()
        time.sleep(15)  # sleep for ten seconds so the thing initializes properly.
        # time.sleep(measurement_time)
        for i in range(measurement_time):
            time.sleep(1)
            get_histograms(long_data)
            long_data_thru_time = np.concatenate((long_data_thru_time, long_data.reshape((long_data.shape[0], long_data.shape[1], 1))), axis=2)
        state[0] = 2
        time.sleep(2)
        state[0] = 3
        time.sleep(2)
        state[0] = 0
        print('stopping measurement from python')
        long_data_summed = np.sum(long_data_thru_time, axis=2)
        for i in range(long_data_summed.shape[0]):
            fig = plt.figure()
            plt.plot(long_data_summed[i, :])
            plt.xlabel('Channel Number')
            plt.ylabel('Counts')
            fig.savefig('longdata_histogram_ch{}.png'.format(i))
        print("quitting main.py")
        state[0] = 3
        time.sleep(2)
        t1.join()
        state[0] = 0
    except:
        state[0] = 3
        time.sleep(2)

    return

main()