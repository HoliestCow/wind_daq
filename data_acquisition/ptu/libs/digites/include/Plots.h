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

#ifndef _PLOTS_H
#define _PLOTS_H                    // Protect against multiple inclusion

#include "digiTES.h"

//****************************************************************************
// Function prototypes
//****************************************************************************
int OpenPlotter();
int ClosePlotter();
int PlotWaveforms(Waveform_t Wfm, char *title);
int PlotFFT(double *fft, int Ns, char *title);
int PlotHisto2D(uint32_t *Histo2D, int nx, int ny, char *title);
int PlotSelectedHisto(int HistoPlotType, int Xunits);

#endif