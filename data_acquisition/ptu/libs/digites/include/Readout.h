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

#ifndef _READOUT_H
#define _READOUT_H                    // Protect against multiple inclusion

#include "CAENDigitizer.h"

#define MAX_NUM_WAVEFORMS 10000
#define WFM_BUFFER_SIZE  (1024*1024*128)

//****************************************************************************
// Function prototypes
//****************************************************************************
int InitReadout(uint32_t *AllocatedMemSize);
int CloseReadout();
int ReadData();
int FreeWaveform(Waveform_t *Waveform);

#endif