
import time
import numpy as np


class CSVSeeker:
    def __init__(self, filetotrack, file_index):
        self.filename = filetotrack
        self.fh = open(self.filename, 'r', buffering=1)
        self.fh.seek(0, 2)
        self.start_seek = self.fh.tell()
        self.end_seek = self.current_seek
        self.file_index = file_index
        self.database_object = database_object

    def closeout(self):
        self.fh.close()

    # def dbwrite(self, spectra):
    #     hacked_spectrum = spectra.tolist()
    #     hacked_spectrum = [str(x) for x in hacked_spectrum]
    #     hacked_spectrum = ','.join(hacked_spectrum)
    #     hacked_spectrum = '\"' + hacked_spectrum + '\"'
    #     #  (I'm going off of the train dataset)
    #     # desired_outputs = (int(time.time() * 1000),  # this is timestamp
    #     # desired_outputs = (1,  # Position ID
    #     #                    hacked_spectrum,
    #     #                    np.sum(spectra))  # cps
    #     desired_outputs = (int(time.time()), self.file_index, hacked_spectrum, np.sum(spectra))
    #     self.database_object.stack_datum(desired_outputs)
    #     # db.fake_stack_datum(desired_outputs)
    #     return

    def get_updates(self):
        # NOTE: This doesn't hold anymore. I need a seek tracker to change the first argument.
        # f.seek(0, 2)  # Go to the end of the file
        juice = []

        # Mark the end of the file.
        self.fh.seek(0, 2)
        self.end_seek = self.fh.tell()
        self.fh.seek(self.start_seek)
        # while len(self.fh.readline()) >= 2:
        while self.fh.tell() < self.end_seek:
            line = self.fh.readline().strip()
            if not line:
                return
            words = line.split(';')  # Or whatever the compass bullshit spits out now.
            if len(words) != 4:
                # I'm throwing away data here. The trouble with stream data.
                continue
            charge = float(words[1])
            juice += [float(charge)]
        counts, bin_edges = np.histogram(np.array(juice), bins=1024, range=(0, 28000))
        return counts
