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

#ifndef _HISTOGRAMS_H
#define _HISTOGRAMS_H                    // Protect against multiple inclusion

#include "digiTES.h"


int CreateHistograms(uint32_t *AllocatedSize);
int DestroyHistograms();
int ResetHistograms();
int Histo1D_AddCount(Histogram1D_t *Histo, int Bin);
int Histo2D_AddCount(Histogram2D_t *Histo, int BinX, int BinY);

extern "C" void readout_histograms(uint32_t ** EHistogramOut);

#endif
