#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Aug  1 14:24:23 2018

@author: Callie Goetz
"""

#This code watches a given folder for modifications (ie a digitizer is writing data to it) 
#and when modified grabs images with a camera and saves them with a timestamp at a given frame rate

#Folder watching and saving to and angle are looking at if applicable are determined by GUI dialog 

#import numpy as np
#import matplotlib as plt
import cv2
import time
from datetime import  datetime 
#import sys
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler
#import logging
#import os
import tkinter as tk
from tkinter import filedialog, simpledialog
import sys

#Event handler class for observer
class Event(LoggingEventHandler):
    
    # So can only initialize/terminate camera once 
    def __init__(self, camera_handle):
        self.cap_handler = camera_handle
    
    #When anything is modified in the folder we are watching, snap a picture, save with timestamp and wait some delay (to control frame rate of camera because digitizers update like crazy )
    def on_modified(self, event):
        ret, frame = self.cap_handler.read()
            
        #Grab time for timestamp
        #t = time.localtime() #if we want sub second time resolution need to use datetime instead of time 

        #Create a timestamp
        timestamp = datetime.utcnow().strftime('%b-%d-%Y_%H-%M-%S.%f')[:-3] #use date time to get timestamps down to a msec
        
        #if want month number (ie 10 instead of Oct use this)
        #timestamp = datetime.utcnow().strftime('%Y-%m-%d %H-%M-%S.%f')[:-3]
        
        #This is old, using time which only goes down to a second 
        #timestamp = time.strftime('%b-%d-%Y_%H%M_%S', t)
        
        #Create the filename to save the file with 
        filename = (fileSave + degrees + 'deg' + '_' + timestamp + '.jpeg')
            
        #Save image
        cv2.imwrite(filename,frame)

        print('Acquisition running at ' + timestamp + ', capturing image')
        
        # Wait a certain amount of time (defined in main)
        time.sleep(delay)
    
if __name__ == "__main__":
    
    
    # Dialog for degrees
    degrees = simpledialog.askstring("Input", "What is the angle in degrees?")  
    #degrees = '0'
    
    
    #number of seconds in between saved frames (set frame rate of camera)
    delay = 0.067  #0.067 = 15 fps 
    
    #Define Location to save images
    root = tk.Tk()
    root.withdraw()
    fileSave = filedialog.askdirectory(title = "Select Folder to Save Images")+'/'
    #fileSave = '/Users/callie_macbookair/Desktop/cameraTest/'
    
    #Define folder that are watching for updates
    root2 = tk.Tk()
    root2.withdraw()
    digitizerPath = filedialog.askdirectory(title = "Select Digitizer Folder")+'/'
    #digitizerPath = '/Users/callie_macbookair/Desktop/cameraTest/spoofDigitizerData/'
    
    
    #Initialize camera
    cap = cv2.VideoCapture(0) #number in () determines which camera you are using
       
    #Initialize observer 
    event_handler = Event(cap)
    observer = Observer()
    observer.schedule(event_handler, digitizerPath, recursive=True)
    observer.start()
    
    #Watch for interrupt and close everything nicely 
    try:
        while True:
            time.sleep(0.001)
    except KeyboardInterrupt:
        #close camera
        cap.release()
        
        #close observer
        observer.stop()
        
        print('Interrupt received...quitting')
    observer.join()
    sys.exit()