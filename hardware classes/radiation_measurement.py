#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue May  7 11:33:50 2019

@author: callie_macbookair


NOTE: You MUST have a sleep time in between starting the digitizer and starting fifo. The digitizer needs time start up and fifo will time out if you 
do not wait. I suggest something like 5-7 seconds.
"""




import logging
import os
import time
import subprocess
from makeHistogram import makeHistogram_noTable 
import matplotlib.pyplot as plt 
import numpy as np
import threading

class radiation_measurement():
    #On init start logging and set the buffer time (how often access shared memory)
     def __init__(self):
         # Turn on logging so can get some debugging action
         logging.basicConfig(format='%(asctime)s %(message)s')
         self.log = logging.getLogger()
         self.log.setLevel(logging.DEBUG)
         self.log.debug('Logging has started')
         
         #Time to wait before grabbing data from shared memory again, used in FIFO 
         self.bufferTime = 0.1
    
    
#         digitizer_location = '/home/kgoetz/Documents/digiTES_4.5.13/bin'
#         save_location = '/home/kgoetz/Documents'
#         run_name = 'test_run'
         
    
    #This method starts the digitizer running independantly in its own terminal 
    #It requires a measurement time, the location the digitizer is writing to, the location where you want to save you data from the run and the run name
     def run_digitizer(self,measurement_time,digitizer_location,save_location,run_name):
         
         
         
         #Autogenerate a digitizer command file that will be used to pipe commands into digites  
         f = open(digitizer_location+'/digitizer_command.txt', 'w') #put script in folder with digitizer 
         f.write('\n')  
         f.write('\n') 
         f.write('s\n')
         f.write('\n')
         f.write('l\n')
         for n in range(0,measurement_time-2): 
             f.write('\n')
         f.write('h\n')
         f.write('\n')
         f.write('s\n')
         f.write('\n')
         f.write('q\n')
         f.close()
         
         #Wait 1 second to let it finish writing file
         time.sleep(1)
         
         
         #Autogenerate script that runs digitizer and moves data safely to named folder when digitizer is finished 
         d = open('run_digiTES.bash', 'w') #put script in current directory 
         d.write('#!/bin/bash \n')
         d.write('cd '+digitizer_location+'/ \n') #change to digitizer location
         d.write('\n') 
         d.write('while read input\n')
         d.write('do echo "$input"\n')
         d.write('\t sleep 1\n')
         d.write('done < digitizer_command.txt | ./digiTES\n')
         d.write('\n')
         d.write('cd '+save_location+'/ \n') #change to digitizer location
         d.write('mkdir '+run_name+'\n') #make a folder with the run name 
         d.write('cp '+digitizer_location+'/DataFiles/* '+save_location+'/'+run_name+'/ \n') #move all data from folder digites writes to folder with run name 
         d.write('\n')
         d.close()
         
         #Wait 1 second to let it finish writing file
         time.sleep(1)
         
         #Start digitizer process 
         self.proc = subprocess.call(['gnome-terminal','-x','./run_digiTES.bash'])
         
         self.log.debug('Digitizer Started')
         
     def acquire_data(self,digitizer_location, channels_activated=[True,True,True,True]):
         ## CARL #################
         # This method is untested and I'm not sure if its right but you said you are familiar with it and it should give you an idea of how I was thinking about starting up a new thread
         #I think we want to use a channel mask because enabling/disabling channels will be easy
         
         #Wait 7 seconds after start acquiring to let digitizer have some time to start up
         time.sleep(7)
         
         
         
         #If channel 0 is enabled, start reading in
         if channel_mask[0]:
             filename = digitizer_location+'/DataFiles/Run0_List_0_0.txt'
             #Start new thread with fifo
             self.ch0 = threading.Thread(target=self.fifo,args=(filename,0))
             self.ch0.start()
         
         #If channel 1 is enabled, start reading in
         if channel_mask[1]:
             filename = digitizer_location+'/DataFiles/Run0_List_0_1.txt'
             #Start new thread with fifo
             self.ch1 = threading.Thread(target=self.fifo,args=(filename,1))
             self.ch1.start()

         #If channel 2 is enabled, start reading in
         if channel_mask[2]:
             filename = digitizer_location+'/DataFiles/Run0_List_0_2.txt'
             #Start new thread with fifo
             self.ch2 = threading.Thread(target=self.fifo,args=(filename,2))
             self.ch2.start()

         #If channel 3 is enabled, start reading in
         if channel_mask[3]:
             filename = digitizer_location+'/DataFiles/Run0_List_0_3.txt'
             #Start new thread with fifo
             self.ch3 = threading.Thread(target=self.fifo,args=(filename,3))
             self.ch3.start()

# THIS METHOD OPENS A PIPE TO SHARED MEMORY, GRABS DATA COMING IN EVERY X NUMBER OF SECONDS (DEFINED BY BUFFER_TIME) AND OUTPUTS NUMPY ARRAYS FOR TIMESTAMP (PICOSECONDS)
# EVENT ENERGY(ADC COUNTS), PSD VALUE AND ENERGY HISTOGRAM
# IT NEEDS A FILE NAME AND A CHANNEL NUMBER 
             
             
     def fifo(self,filename,channel_number):
        
        #Check that digitizer is running 
        if not os.path.exists(filename):
            self.log.debug('Start acquistion please')
            #If its not running wait 5 seconds 
            time.sleep(5)
        else:
            self.log.debug('Acquistion running, pipe present: OK')
            
        self.log.debug('Pipe to data for channel '+str(channel_number)+' is open for reading')
        
        #set new pipe variable to true at beginning of run
        newPipe=True
        
        #Initialize wait count to 0 
        wait_count = 0
        
        
        #Open the pipe to shared memory  
        with open(filename, 'r') as fifo:
            # initialize empty arrays
            tr = []
            l = []
            psd = []
            
            while True:
    
                data = fifo.read().splitlines() #split incoming data into lines based on carriage return
                
                
                #Set up a time out so the code stops when the digitizer does 
                if not data and wait_count < 11:
                    self.log.debug('Waiting for data from digitizer on channel '+str(channel_number))
                    time.sleep(0.5)
                    wait_count = wait_count+1
                    continue
                elif not data and wait_count == 11:
                    self.log.debug('Digitizer has stopped, time out criteria reached, quitting shared memory access on channel '+str(channel_number))
                    return
                    
                #Reset wait count to 0 every time get new data
                wait_count = 0
                
                
                ############################ NOTE ############################
                # Sometimes fifo accessses shared memory and grabs data when the digitizer is still writing a line, most of the things below are for dealing with that 
                
                
                #Grab the first line to fill first 
                first = data[0]
                first_words = first.split(' ')
                while '' in first_words: #get rid of extra spaces
                    first_words.remove('')
    
                #if this is a new file, no need to worry if things have been cut off, but do fill last
                if newPipe is True:
                    for line in data[-1]:
                        words = line.split(' ')  # split line into space delimited words
                        while '' in words: #get rid of extra spaces
                            words.remove('')
    #                print("Received Data: " + str(words))
                    #build 1D arrays with list mode data
                        if len(words) == 3:
                            tr.append(words[0])  # trigger time in ps
                            l.append(words[1])  # total integrated charge of event in counts
                            psd.append(words[2])  # psd value
                        else: 
                            #Just a bit of error checking
                            print("Read error on channel "+str(channel_number)+", skipping line")
                            print("Data line on channel "+str(channel_number)+" is:")
                            print(words)
                        
                   #uncomment for debugging
    #                print("Long gate is: " + str(l))
                    time.sleep(self.bufferTime) # wait x number of seconds to check for data coming in on pipe
                    last = data[-1]
                    #new pipe is now false
                    newPipe = False
                #if its not a new pipe, proceed as normal
                else: 
                    for line in data[1:-1]:
                        words = line.split(' ')  # split line into space delimited words
                        while '' in words:
                            words.remove('')
                    #build 1D arrays with list mode data
                        tr.append(float(words[0])*(10**-12))  # trigger time in ns
                        l.append(float(words[1]))  # total integrated charge of event in counts
                        psd.append(float(words[2]))  # psd value
                        
                    newline = []
                    last_words = last.split(' ')
                    while '' in last_words: #get rid of extra spaces
                            last_words.remove('')
                            
                            
                    
                    #if both the last line of the old data file and the first line are fine then append them as expected
                    if len(last_words) == 3 and len(first_words) ==3:
                        tr.append(last_words[0])  # trigger time in ns
                        l.append(last_words[1])  # total integrated charge of event in counts
                        psd.append(last_words[2])  # psd value
                        
                        tr.append(first_words[0])  # trigger time in ns
                        l.append(first_words[1])  # total integrated charge of event in counts
                        psd.append(first_words[2])  # psd value
                        
                    #if last words and first words have the length of 2 then they split in the long gate
                    elif len(last_words) == 2 and len(first_words) ==2:
                        print('Read split in long gate value in channel '+str(channel_number)+'. Fixing.')
                        newline = last+first
                        print("Whole line in channel "+str(channel_number)+" is: " + newline)
                        new_words = newline.split(' ')
                        while '' in new_words: #get rid of extra spaces
                            new_words.remove('')
                        tr.append(new_words[0])  # trigger time in ps
                        l.append(new_words[1])  # total integrated charge of event in counts
                        psd.append(new_words[2])  # psd value
    #                    self.log.debug('Read split in long gate value. Fixing.')
                        
                    #if the last word is 3 but the first word is 1 then split on the PSD value
                    elif len(last_words) == 3 and len(first_words) ==1:
                        print('Read split in PSD value in channel '+str(channel_number)+'. Fixing.')
                        newline = last+first
                        print("Whole line in channel "+str(channel_number)+" is: " + newline)
                        new_words = newline.split(' ')
                        while '' in new_words: #get rid of extra spaces
                            new_words.remove('')
                        tr.append(new_words[0])  # trigger time in ps
                        l.append(new_words[1])  # total integrated charge of event in counts
                        psd.append(new_words[2])  # psd value
    #                    self.log.debug('Read split in PSD value. Fixing.')
                        
                    #if the last word is 1 and the first word is 3 then split on the time stamp
                    elif len(last_words) == 1 and len(first_words) ==3:
                        print("Read split in time stamp in channel "+str(channel_number)+", fixing.")
                        newline = last+first
                        print("Whole line in channel "+str(channel_number)+" is: " + newline)
                        
                        new_words = newline.split(' ')
                        while '' in new_words: #get rid of extra spaces
                            new_words.remove('')
                        tr.append(new_words[0])  # trigger time in ps
                        l.append(new_words[1])  # total integrated charge of event in counts
                        psd.append(new_words[2])  # psd value
    #                    self.log.debug('Read split in timestamp. Fixing.')
    #                
                    #Use this to catch if it splits in unexpected way 
                    else:
                        #Write to log file here! 
    #                    self.log.warning('Something very weird just happened. Read is split in an unknown way. This event is being discarded.')
                        print("Read split in unknown way in channel "+str(channel_number)+", fixing")
                        print("Last line in channel "+str(channel_number)+" is:")
                        print(last)
                        print("First line in channel "+str(channel_number)+" is:")
                        print(first)
                        print("Whole line in channel "+str(channel_number)+" is:" + last+first)
                        newline = last+first
                        new_words = newline.split(' ')
                        while '' in new_words: #get rid of extra spaces
                            new_words.remove('')
    #                    self.log.debug(first) 
                        tr.append(new_words[0])  # trigger time in ps
                        l.append(new_words[1])  # total integrated charge of event in counts
                        psd.append(new_words[2])  # psd value
                                        
                    time.sleep(self.bufferTime) # wait x number of seconds to check for data coming in on pipe   
                    last = data[-1] #fill last line of data for comparison in next batch
                    
                    
                    
                #uncomment for debugging (check whether data coming in)
                #print("Received Data: " + str(data))
                #print('lololololololol')
                
                #Make nice numpy arrays out of the long gate and time stamp, these arrays update with every grab from shared memory
                    self.timestamp = np.array(tr,dtype=np.float)*(10**-12) #put the time stamp in ns 
                    self.event = np.array(l,dtype=np.float) #event is the total integrated value in bin# for a given event
                    self.psd = np.array(l,dtype=np.float) #PSD value
                    print("Number of Events in channel "+str(channel_number)+": " + str(len(self.event)))
                    
                    if len(self.event > 0):
                        [self.hist,self.bins] = makeHistogram_noTable(self.event)
    #                    plt.plot(bins,hist)
    #                    plt.xlim(0,2**14)
    #                    plt.show()



         
         
         
         
         
         
         
         
         
         
         
         
         
