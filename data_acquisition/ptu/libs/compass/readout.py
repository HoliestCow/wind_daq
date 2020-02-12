import os
import datetime
import glob
import re
import pytz
import numpy as np
import matplotlib.pyplot as plt


class CompassReadout:
    def __init__(self, dataDir):
        self.current_counts = None
        self.current_histogram = None
        self.readout_dir = dataDir
        self.archive_dir = os.path.join(self.readout_dir, "measurement_archive")

        self.reference_datetime = {}
        self.current_datetime = {}

        self.data = {}

        self.initialize_dir_structure()
        return

    def initialize_dir_structure(self):
        if not os.path.isdir(self.archive_dir):
            os.mkdir(self.archive_dir)
        return

    def process_files(self, list_of_filenames):
        channels = []
        times = []
        for filename in list_of_filenames:
            # remove the fileextension off the filename
            filename_path = os.path.split(filename)[-1]
            just_filename = os.path.splitext(filename_path)[0]
            channel_string = re.split('@', just_filename)[0]
            channel_number = re.findall(r'\d+', channel_string)[0]  # guaranteed to be one number

            if channel_number not in self.data:
                self.data[channel_number] = {}

            split_string = re.split('_', just_filename)
            date_string = split_string[-2]
            time_string = split_string[-1]
            datetime_string = '_'.join([date_string, time_string])
            datetime_obj = datetime.datetime.strptime(datetime_string, '%Y%m%d_%H%M%S')  # time input is probably in EST
            EST = pytz.timezone('US/Eastern')
            datetime_obj = EST.localize(datetime_obj)
            
            self.data[channel_number][datetime_obj] = {}
            answers = self.data[channel_number][datetime_obj]
            answers['datetime_obj'] = datetime_obj

            # answers['epochtime'] = (datetime_obj - datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)).total_seconds()
            answers['epochtime'] = (datetime_obj - datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)).total_seconds()
            answers['datestr'] = date_string
            answers['timestr'] = time_string

            answers['filename'] = filename

            f = open(filename , 'r')
            a = f.readlines()
            # total_counts = int(a[0].strip())
            histogram = np.zeros((2**14, ), dtype=int)
            counter = 0
            for line in a:
                meh = line.strip()
                histogram[counter] = int(meh)
                counter += 1
            total_counts = np.sum(histogram[100:])
            print('================')
            print(total_counts)
            print('===============')

            answers['counts'] = total_counts
            answers['energy_spectrum'] = np.array(histogram)

            self.data[channel_number][datetime_obj] = answers
        return

    def archive_files(self, filelist):
        for filename in filelist:
            just_filename = os.path.split(filename)[-1]
            absolute_path_before = os.path.join(self.readout_dir, just_filename)
            absolute_path_after = os.path.join(self.archive_dir, just_filename)
            os.rename(absolute_path_before, absolute_path_after)
        return

    def pair_data(self):
        # Take the meta data, and pair the current and previous timestamps.
        out_dict = {}
        for channel in self.current_datetime:
            current_datetime = self.current_datetime[channel]
            data = self.data[channel][current_datetime]
            reference_datetime = self.reference_datetime[channel]
            print(reference_datetime)
            print(current_datetime)
            reference_data = self.data[channel][reference_datetime]
            counts = data['counts'] - reference_data['counts']
            energy_spectrum = data['energy_spectrum'] - reference_data['energy_spectrum']
            out_dict[channel] = {
                    'counts': counts,
                    'energy_spectrum': energy_spectrum,
                    'epochtime': data['epochtime'],
                    'datetime_obj': data['datetime_obj'],
                    'datestr': data['datestr'],
                    'timestr': data['timestr']}
            self.reference_datetime[channel] = current_datetime
        self.current_measurement = out_dict
        return

    def get_latest_datetime_per_channel(self):
        timedict = {}  # by channel
        for key in self.data:
            # these keys are datetime_obj
            current = self.data[key]
            channel = key
            if channel not in timedict:
                timedict[channel] = []
            timedict[channel] += list(current.keys())

        # now the entire timedict by channel is populated.
        # Now to yank the latest one for each channel and tie it back to the filename
        chosen_datetime = {}
        previous_datetime = {}
        for channel in timedict:
            # have to yank the max time and the second to max time.
            # This is so I can get that seconds worth of data via subtraction.
            timedict[channel].sort()
            if len(timedict[channel]) > 1:
                self.reference_datetime[channel] = timedict[channel][-2]
                chosen_datetime[channel] = timedict[channel][-1]
            else:
                # previous_datetime[channel] = timedict[channel][-1]
                chosen_datetime[channel] = timedict[channel][-1]
        self.current_datetime = chosen_datetime
        return 

    def update_measurement(self):
        self.current_measurement = None
        # analyze files in the default location
        filelist = glob.glob(os.path.join(self.readout_dir, '*.txt'))
        # meta_data and measurement_data are keyed with filename
        self.process_files(filelist)  # indexed by datetime_obj now
        # TODO: choose latest file for each channel
        self.get_latest_datetime_per_channel()

        self.pair_data()  # reorganize to make a dict keyed by channel.
        self.archive_files(filelist)  # last step
        return

    def plotvstime(self):
        for channel in self.data:
            time = []
            counts = []
            for datetime_obj in self.data[channel]:
                time += [datetime_obj]
                counts += [self.data[channel][datetime_obj]['counts']]
            fig = plt.figure()
            sorted_counts = [x for _,x in sorted(zip(time, counts))]
            time = sorted(time)
            plt.plot(time[1:], np.diff(sorted_counts), '.')
            fig.savefig('channel{}_countsvstime.png'.format(channel))
            plt.close()
        return

    def plot_all_measurements(self):
        filelist = glob.glob(os.path.join(self.readout_dir, '*.txt'))
        self.process_files(filelist)
        self.plotvstime()
        return
