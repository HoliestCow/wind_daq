#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon May  6 15:49:47 2019

@author: root
"""

from radiation_measurement import radiation_measurement
import time

rad = radiation_measurement()

measurement_time = 60
digitizer_location = '/home/kgoetz/Documents/digiTES_4.5.13/bin'
save_location = '/home/kgoetz/Documents/test_run'
run_name = 'test_0'

filename = digitizer_location+'/DataFiles/Run0_List_0_0.txt'

rad.run_digitizer(measurement_time,digitizer_location,save_location,run_name)
time.sleep(5)
rad.fifo(filename,0)