#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Aug 28 13:53:20 2018

@author: callie_macbookair
"""

import numpy as np
from astropy.table import Table

def makeHistogram(table,string):
    hist,bins=np.histogram(table[string],bins=int(max(table[string])-min(table[string])),range=(min(table[string]),max(table[string])))
    bins=np.linspace(start=min(table[string]), stop=max(table[string]),num=max(table[string])-min(table[string]), endpoint=True)
    return(hist,bins)
    
    
def makeHistogram_noTable(data):
    hist,bins=np.histogram(data,bins=int(max(data)-min(data)),range=(min(data),max(data)))
    bins=np.linspace(start=min(data), stop=max(data),num=max(data)-min(data), endpoint=True)
    return(hist,bins)
    
    
def makeHistogram_defBins(data,numBins):
    hist,bins=np.histogram(data,bins=numBins,range=(min(data),max(data)))
    bins=np.linspace(start=min(data), stop=max(data),num=numBins, endpoint=True)
    return(hist,bins)


def makeHistogram_time(table):
    timeS = []
    hist = []
    cps = []
    b = []
    iteration = int(np.ceil(table['time'][len(table['time'])-1]))
    print(iteration)
    #print(str(iteration))
    for n in range(1,iteration+1):
        timeS.append(n)
        data = table['long'][((n-1) <= table['time']) &  (table['time'] < n)]
        temp,bins = np.histogram(data,bins = 2**14, range=(0,2**14))
        cps.append(np.sum(temp)) #count rate 
        #bins=np.linspace(start=0, stop=2**14,num=2**14, endpoint=True)
        hist.append(temp)
        b.append(bins)
    tableOut = Table()
    tableOut['time'] = timeS #time in seconds 
    tableOut['hist'] = hist #histogram collected for 1 second 
    tableOut['cps'] = cps #count rate 
    tableOut['bins'] = b
    return(tableOut)