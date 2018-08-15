
import numpy as np
from caenlib import measurement_spool
import threading
import time


def main():

    measurement_time = 60  # 60 seconds.

    # measurement_spool(state, short_data, long_data, size)
    state = np.array([1], dtype=np.int)
    array_size = (1, 4096)
    short_data = np.zeros(array_size, dtype=np.uint32)
    long_data = np.zeros(array_size, dtype=np.uint32)
    t1 = threading.Thread(target=measurement_spool,
                          args=(state, short_data, long_data, short_data.size))
    t1.start()
    time.sleep(measurement_time)
    state[0] = 0
    print('stopping measurement from python')
    t1.join()
    print('saving data')
    np.savetxt('shortdata.csv', short_data, delimiter=',')
    np.savetxt('longdata.csv', long_data, delimiter=',')
    print("quitting main.py")
    return

main()