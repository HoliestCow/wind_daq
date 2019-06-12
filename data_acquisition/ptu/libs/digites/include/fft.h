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

#ifndef __FFT_H
#define __FFT_H

// Types of windowing
#define HANNING_FFT_WINDOW    0
#define HAMMING_FFT_WINDOW    1
#define BLACKMAN_FFT_WINDOW   2
#define RECT_FFT_WINDOW       3

int InitFFT(uint32_t *AllocatedSize);
int CloseFFT();
int SetFFTaverage(int nfft);
int AccumulateFFT(Waveform_t Wave);
int GetFFT(int *nfft, int *ns, double **fft);

#endif
