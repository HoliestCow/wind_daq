#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 10 10:51:39 2018

@author: callie_macbookair
"""

#This code reads in xml file with settings that compass saves, parses it and dumps it into a dictionary and maps the settings into appropriately named structures 


import xmltodict

#Make class defining a struture for things that are set digitizer-wide for sure 
class parameters:
    def __init__(self, RecordLength, PulsePolarity, ChannelMask, EventAggr, IOlev, AcqMode, LinkType, VMEBaseAddress, DCOffset, ConetNode, LinkNum,Trace):
        self.RecordLength = RecordLength 
        self.PulsePolarity = PulsePolarity 
        self.ChannelMask = ChannelMask 
        self.EventAggr = EventAggr  
        self.IOlev = IOlev  
        self.AcqMode = AcqMode 
        self.LinkType = LinkType
        self.VMEBaseAddress = VMEBaseAddress 
        self.DCOffset = DCOffset 
        self.ConetNode = ConetNode 
        self.LinkNum = LinkNum
        self.Trace = Trace

#Make class defining a struture for things that need to be set individually for each channel 
#Other info in settings by channel we might need: 
# calibration, energy low and high cuts,     
class dppParameters:
    def __init__(self, trgho, thr, selft, csens, sgate, lgate, pgate, tvaw, nsbl, trgc, purh, purgap):
        self.trgho = trgho #trgo - integer, trigger hold off in sample #
        self.thr = thr #thr - integer, threshold (LSB)
        self.selft = selft #selft - integer, 1 or 0, self trigger enable/disable
        self.csens = csens  #csens - integer, Charge Sensitivity (options for x720: 0= 40, 1 = 160, 2= 640, 3 = 2560 fC/LSB, options for x751: 0 = 20, 1 = 40, 2 = 80, 3 = 160, 4= 320, 5 = 640 fC/LSB)
        self.sgate = sgate  #sgate - integer, short gate width (samples)
        self.lgate = lgate #lgate - integer, long gate width (samples)
        self.pgate = pgate #pgate - integer, gate offset (samples)
        self.tvaw = tvaw #tvaw - integer, trigger validation acceptance window when in coincidence mode (samples)
        self.nsbl = nsbl #nsbl - number of samples for baseline mean options for x720: 0 = FIXED, 1 = 8, 2 = 32, 3 = 128; options for x751: 0 = FIXED, 1 = 8, 2 = 16, 3 = 32, 4 = 64, 5 = 128, 6 = 256, 7 = 512; options for x730/x725: 0 = FIXED, 1 = 16, 2= 64, 3 = 256, 4 = 1024)
        self.trgc = trgc  #trgc - who the hell knows what this is, CAEN says is deprecated and must be set to 1
        self.purh = purh #purh - *Pile-Up option selection (0 = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly, 1 =CAEN_DGTZ_DPP_PSD_PUR_Enabled). Ignored forx751
        self.purgap = purgap #purgap - Pile-Up Rejection GAP value (LSB). Ignored for x751
        
# Function that takes dictionary name and 'variable' want to look up and searches dictionary for settings on each channel, set string to true if want output to be a string 
def search(dictionary, lookup, string='False'):    
    # Figure out the number of channels (should always be 4 but why not make it easier if we go with another CAEN digitizer sometime)
    numChannels = int(float(dictionary['configuration']['board']['channelCount']))
    
    #Initialize empty array
    settingValue = []
    
    #Make array with appropriate number of channels, fill with something that caen wouldnt write in setting file 
    for c in range(0,numChannels):
        settingValue.append('empty')
    
    # Look for desired setting in the channel settings
    for c in range(0,numChannels):
        entryLength = len(dictionary['configuration']['board']['channel'][c]['values']['entry'])
        for e in range(0,entryLength):
            for key, value in dictionary['configuration']['board']['channel'][c]['values']['entry'][e].items():
                if key == 'key':
                    if value == lookup:
                        if string == 'True':  
                            #Leave as string
                            settingValue[c] = dictionary['configuration']['board']['channel'][c]['values']['entry'][e]['value']['#text']
                        else:
                            #round and make integer 
                            settingValue[c] = int(round(float(dictionary['configuration']['board']['channel'][c]['values']['entry'][e]['value']['#text'])))
                        # uncomment next line for debugging
                        #print(value,dictionary['configuration']['board']['channel'][c]['values']['entry'][e]['value']['#text'])
    
    # If setting is not in the channel settings, assume need to use board-wide setting 
    for c in range(0,numChannels):
        if settingValue[c] == 'empty':
            entryLength = len(dictionary['configuration']['board']['parameters']['entry'])
            for e in range(0,entryLength):
                for key, value in dictionary['configuration']['board']['parameters']['entry'][e].items():
                    if key == 'key':
                        if value == lookup:
                            if string == 'True':  
                                #Leave as string
                                settingValue[c] = dictionary['configuration']['board']['parameters']['entry'][e]['value']['value']['#text']
                            else:
                                #round and make integer 
                                settingValue[c] = int(round(float(dictionary['configuration']['board']['parameters']['entry'][e]['value']['value']['#text'])))
                            # uncomment next line for debugging
                            #print(value,dictionary['configuration']['board']['channel'][c]['values']['entry'][e]['value']['#text'])
    return(settingValue)


# Function that actually builds the structures 
#   Uses search function to search XML setting file for settings and build relevant structures 
#   
def readSettings(filename):
    
    #Initialize arrays for objects that have a setting for each channel of the digitizer
    
    with open(filename) as getSettings:
        settings = xmltodict.parse(getSettings.read())
        
        #Mark, need to compare with whats in CoMPASS because I'm not sure if these values are in ns or clock ticks (samples). Caen documentation says they want clock ticks but unclear if that or real time is whats saved in setting file 
        
        #DPP parameters 
        trgho =  search(settings, 'SRV_PARAM_CH_TRG_HOLDOFF','False')
        thr = search(settings, 'SRV_PARAM_CH_THRESHOLD','False')
        selft = [0,0,0,0] #can't find in settings file, assume don't ever want to to self trigger 
        csens = search(settings, 'SRV_PARAM_CH_ENERGY_COARSE_GAIN','True') #mark, not sure if string or int 
        sgate = search(settings, 'SRV_PARAM_CH_GATESHORT','False')
        lgate = search(settings, 'SRV_PARAM_CH_GATE','False')
        pgate = search(settings, 'SRV_PARAM_CH_GATEPRE','False')
        tvaw = search(settings, 'SRV_PARAM_COINC_TRGOUT','False') #mark, not sure if this is correct, just guessing 
        nsbl = search(settings, 'SRV_PARAM_CH_BLINE_NSMEAN','True') #mark, not sure if string or int
        trgc = ['CAEN_DGTZ_DPP_TriggerConfig_Threshold','CAEN_DGTZ_DPP_TriggerConfig_Threshold','CAEN_DGTZ_DPP_TriggerConfig_Threshold','CAEN_DGTZ_DPP_TriggerConfig_Threshold'] #mark, caen says this needs to be set to 1 in the documentation but then code has is set to text value so going with value for now 
        purh = search(settings, 'SW_PARAM_CH_PUR_ENABLE','True') #mark, another discrepancy 
        purgap = search(settings, 'SRV_PARAM_CH_PURGAP','False')
        
        #Parameters 
        VMEBaseAddress = int(float(settings['configuration']['board']['address']))
        LinkNum = int(float(settings['configuration']['board']['linkNum']))
        LinkType = settings['configuration']['board']['connectionType']
        ConetNode = int(float(settings['configuration']['board']['conetNode']))
        EventAggr = search(settings, 'SRV_PARAM_EVENTAGGR','False')[0] #just grabbing the first one of these and others in this series because know that its just in the digitizer-wide settings but dont feel like making a new function 
        DCOffset = search(settings, 'SRV_PARAM_CH_BLINE_DCOFFSET','False')[0]
        Trace = search(settings, 'SRV_PARAM_WAVEFORMS','True')[0] #i THINK this is whether to record traces 
        IOlev = search(settings, 'SRV_PARAM_IOLEVEL','True')[0]
        PulsePolarity = search(settings, 'SRV_PARAM_CH_POLARITY','True')[0]
        ChannelMask  = search(settings, 'SRV_PARAM_CH_ENABLED','True') #mark, enable boolean but example code has 0xF hex value, unsure of whats what 
        AcqMode = settings['configuration']['acquisitionMemento']['acquisitionMode'] # mark, settings file has this as 'list', but example code has longer string 'CAEN_DGTZ_DPP_ACQ_MODE_Mixed', WTF Caen 
        RecordLength = search(settings, 'SRV_PARAM_RECLEN','False')[0]
        
    DigitizerParameters = parameters(RecordLength,PulsePolarity,ChannelMask,EventAggr,IOlev,AcqMode,LinkType,VMEBaseAddress,DCOffset,ConetNode,LinkNum,Trace)
    DPPParameters = dppParameters(trgho,thr,selft,csens,sgate,lgate,pgate,tvaw,nsbl,trgc,purh,purgap)
    return(DigitizerParameters,DPPParameters)
    
# Search key names:
#for key, value in settings['configuration']['board'].items():
#    print(key)
#    
    
    # Another way to print key names:
#    for e in range(0,len(settings['configuration']['board']['parameters']['entry'])):
#    for key,value in settings['configuration']['board']['parameters']['entry'][e].items():
#        if key == 'key':
#            print(value,settings['configuration']['board']['parameters']['entry'][e]['value']['value']['#text'])
    
#For channel by channel names
#ch=0
#for e in range(0,len(settings['configuration']['board']['channel'][ch]['values']['entry'])):
#    for key,value in settings['configuration']['board']['channel'][ch]['values']['entry'][e].items():
#        if key == 'key':
#            print(value,settings['configuration']['board']['channel'][ch]['values']['entry'][e]['value']['#text'] )
#                       

    
    
    
    
    
    
    
    
    
    