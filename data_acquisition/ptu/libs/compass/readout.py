import os
import datetime
import glob
import re
import pytz
import numpy as np


class CompassReadout:
    def __init__(self, dataDir):
        self.current_counts = None
        self.current_histogram = None
        self.readout_dir = dataDir
        self.archive_dir = os.path.join(self.readout_dir, "measurement_archive")
        self.initialize_dir_structure()
        return

    def initialize_dir_structure(self):
        if not os.path.isdir(self.archive_dir):
            os.mkdir(self.archive_dir)
        return

    def process_filenames(self, list_of_filenames):
        channels = []
        times = []
        outdict = {}  # dict keyed with the filename. This will be a dict of dict
        for filename in list_of_filenames:
            # remove the fileextension off the filename
            filename_path = os.path.split(filename)[-1]
            just_filename = os.path.splitext(filename_path)[0]

            answers = {'datetime_obj': None,
                       'epochtime': None,
                       'channel': None}
            
            channel_string = re.split('@', just_filename)[0]
            channel_number = re.findall(r'\d+', channel_string)[0]  # guaranteed to be one number
            answers['channel'] = channel_number

            split_string = re.split('_', just_filename)
            date_string = split_string[-2]
            time_string = split_string[-1]
            datetime_string = '_'.join([date_string, time_string])
            datetime_obj = datetime.datetime.strptime(datetime_string, '%Y%m%d_%H%M%S')  # time input is probably in EST
            EST = pytz.timezone('US/Eastern')
            datetime_obj = EST.localize(datetime_obj)

            answers['datetime_obj'] = datetime_obj

            # answers['epochtime'] = (datetime_obj - datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)).total_seconds()
            answers['epochtime'] = (datetime_obj - datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)).total_seconds()
            answers['datestr'] = date_string
            answers['timestr'] = time_string

            outdict[filename] = answers
        return outdict

    def readout_files(self, filelist):
        outdict = {}
        for filename in filelist:
            f = open(filename , 'r')
            a = f.readlines()
            total_counts = int(a[0].strip())
            histogram = []
            for line in a:
                meh = line.strip()
                histogram += [int(meh)]
            outdict[filename] = {'counts': total_counts,
                                 'energy_spectrum': np.array(histogram)}
        return outdict

    def archive_files(self, filelist):
        for filename in filelist:
            just_filename = os.path.split(filename)[-1]
            absolute_path_before = os.path.join(self.readout_dir, just_filename)
            absolute_path_after = os.path.join(self.archive_dir, just_filename)
            os.rename(absolute_path_before, absolute_path_after)
        return

    def pair_data(self, filelist, meta, measurement):
        out_dict = {}
        print(filelist)
        for filename in filelist:
            metadata = meta[filename]
            if metadata['datetime_obj'] != self.latest_datetimes[metadata['channel']]:
                continue
            print('here')
            measured_data = measurement[filename]
            out_dict[metadata['channel']] = {
                    'counts': measured_data['counts'],
                    'energy_spectrum': measured_data['energy_spectrum'],
                    'epochtime': metadata['epochtime'],
                    'datetime_obj': metadata['datetime_obj'],
                    'datestr': metadata['datestr'],
                    'timestr': metadata['timestr']}
        return out_dict

    def get_latest_datetime_per_channel(self, meta_data):
        timedict = {}  # by channel
        chosen_datetime = {}
        for key in meta_data:
            current = meta_data[key]
            channel = current['channel']
            if channel not in timedict:
                timedict[channel] = []
            datetime_obj = current['datetime_obj']
            timedict[channel] += [datetime_obj]

        # now the entire timedict by channel is populated.
        # Now to yank the latest one for each channel and tie it back to the filename
        for channel in timedict:
            chosen_datetime[channel] = max(timedict[channel])
            
        return chosen_datetime

    def update_measurement(self):
        self.current_measurement = None
        # analyze files in the default location
        filelist = glob.glob(os.path.join(self.readout_dir, '*.txt'))
        print(filelist)
        # meta_data and measurement_data are keyed with filename
        meta_data = self.process_filenames(filelist)
        # TODO: choose latest file for each channel
        self.latest_datetimes = self.get_latest_datetime_per_channel(meta_data)

        measurement_data = self.readout_files(filelist)
        output_data = self.pair_data(filelist, meta_data, measurement_data)  # reorganize to make a dict keyed by channel.
        self.archive_files(filelist)  # last step
        self.current_measurement = output_data  # organized by channel
        return
