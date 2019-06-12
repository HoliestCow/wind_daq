
import matplotlib.pyplot as plt
from glob import glob
import numpy as np

def main():
    filelist = glob('./*det0*.npy')
    for file in filelist:
        data = np.load(file)
        fig = plt.figure()
        plt.plot(data)
        fig.savefig(file[:-4] + '.png')
        plt.close()
    return

main()