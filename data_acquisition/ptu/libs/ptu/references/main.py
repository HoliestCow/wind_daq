
from wrapper import running_process
import numpy as np
from time import sleep
import threading
import ctypes


def main():
    # running_process(x)
    # Pass an immutable object to running process. Maybe a list of [x]
    # Have the c function look at the last element of the list for commands.
    # x = np.array([1], dtype=np.uint32)
    x = np.array([1], dtype=np.int32)
    array_size = (10,)
    data = np.zeros(array_size, dtype=np.int32)
    t1 = threading.Thread(target=running_process, args=(x, data, data.size))
    t1.start()
    print('in python, x = {}'.format(x[0]))
    sleep(3)
    x[0] = 0
    print('in python, x = {}'.format(x[0]))
    t1.join()
    print(data)
    print("quitting main.py")
    print(np.sum(data))
    np.savetxt('yolo.csv', data, delimiter=',')
    return

main()
