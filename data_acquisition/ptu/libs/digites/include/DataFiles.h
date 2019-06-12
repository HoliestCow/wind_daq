/******************************************************************************
* 
* CAEN SpA - Front End Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
******************************************************************************/

#ifndef _DATAFILES_H
#define _DATAFILES_H                    // Protect against multiple inclusion

#include "digiTES.h"

// Global Variables
extern FILE *InputDataFile;				// event data input file (for the data replay mode)

//****************************************************************************
// Function prototypes
//****************************************************************************
int OpenInputDataFile();
int OpenOutputDataFiles();
int CheckOutputDataFilePresence();
int CloseOutputDataFiles();
int SaveAllHistograms();
int SaveRawData(int b, int ch, GenericDPPEvent_t evnt);
int SaveWaveform(int b, int ch, GenericDPPEvent_t evnt);
int SaveList(int b, int ch, GenericDPPEvent_t evnt);
int SaveMergedList(int b, int ch, GenericDPPEvent_t evnt);
int SaveBuiltEventsList(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int NumEv[MAX_NBRD][MAX_NCH]);
int SaveRunInfo();

#endif