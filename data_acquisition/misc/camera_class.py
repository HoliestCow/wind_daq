#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Apr 16 10:54:20 2019

@author: root

Class to run camera. 
Acquire requires the name of the run, the path to save the images and run time in seconds. 
Saves time stamped images at a given frame rate 
"""
import cv2
import time
from datetime import  datetime 
import numpy as np

class Camera():
     def __init__(self):
         #Open camera 
         self.camera_handle = cv2.VideoCapture(1) #1 is the camera, 0 is the camera on the laptop
         self.fps  = 15 # frames per second
         print("Opening connection to camera")
     def acquire(self,run_name,file_location,measurement_time):
         
         print("Collecting camera images at " + str(self.fps) + " frames per second for " + str(measurement_time) + " seconds.")
         
         counter = 0 #initialize a counter 
         
         #Calculate how many frames are in requested measurement period
         counter_maximum = np.ceil(measurement_time*self.fps)
         
         #Acquire frames
         while(True):
             #Grab a frame
                 ret, self.frame = self.camera_handle.read()
                 
             #check all is ok with camera
                 if ret:
                     pass
                 else:
                     print("Camera not responding, check connection.")
                     self.close()
                     break
                 
                #Make a timestamp
                 self.timestamp = datetime.utcnow().strftime('%b-%d-%Y_%H-%M-%S.%f')[:-3]
                 
                 #Build the file name 
                 filename = (file_location +'/' + run_name +'_' + self.timestamp + '.jpeg')
                 
                 #Save the image 
                 cv2.imwrite(filename,self.frame)
                 
                 #Wait for a set amount of time so recording at desired frame rate 
                 time.sleep(1/self.fps)
                 
                 #Increment frame counter
                 counter = counter + 1 
                 
                 #Quit when have reached end of measurement period 
                 if cv2.waitKey(1) & counter == counter_maximum:
                     break
     def close(self):
         print("Closing connection to camera")
         self.camera_handle.release()