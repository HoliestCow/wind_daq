
import time
import numpy as np
import io

# This is actually harder than I was envisioning since I need nanosecond precision in the time.sleeps()

# But the idea is to read the entire static file, populate the sleep times in nanoseconds, correct
#   for any overflows (or rollovers), then write the file with the appropriate timescales into
#   the buffer file.


def main():
    targetFile = 'preprocessed_nick137_004_ls_1.dat'
    outputFile = 'raw_stream_data.dat'
    # Purpose is to take data that was collected and make another seem like its populated in realtime
    # fr = open(targetFile, 'r')

    # First pass to get the time differences
    x = []
    payload = []
    with open(targetFile, 'r') as fr:
        lines = fr.readlines()
        # while line:
        for line in lines:
            words = line.split()
            x += [float(words[0])]
            payload += [float(words[1])]
    payload = np.array(payload)
    x = np.array(x)
    timediff = np.diff(x)

    x = np.concatenate((np.array([0]), timediff))
    x = x * 4  # convert to nanoseconds (4 ns / tick)
    x = x * 1E-6  # convert to microseconds

    print(np.max(x))
    print(np.max(payload))
    print(np.median(payload))
    print(np.std(payload))

    # with open(outputFile, 'w', buffering=0) as fw:
    with io.open(outputFile, 'w', buffering=1) as fw:
        for i in range(len(x)):
            time.sleep(x[i])  # make this an asynchronous process.
            message = '{} {}\n'.format(x[i], payload[i])
            fw.write(message)
    return

main()
