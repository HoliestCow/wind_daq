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

#include "digiTES.h"
#include "Histograms.h"

int HistoCreated = 0;

int CreateHistogram1D(int Nbin, char *Title, char *Xlabel, char *Ylabel, Histogram1D_t *Histo) {
	Histo->H_data = (uint32_t *)malloc(Nbin*sizeof(uint32_t));
	Histo->Nbin = Nbin;
	Histo->H_cnt = 0;
	Histo->Ovf_cnt = 0;
	Histo->Unf_cnt = 0;
	Histo->mean = 0;
	Histo->rms = 0;
	Histo->real_time = 0;
	Histo->live_time = 0;
	Histo->n_ROI = 0;
	Histo->A[0] = 0;
	Histo->A[1] = 1;
	Histo->A[2] = 0;
	Histo->A[3] = 0;
	return 0;
}

int CreateHistogram2D(int NbinX, int NbinY, char *Title, char *Xlabel, char *Ylabel, Histogram2D_t *Histo) {
	Histo->H_data = (uint32_t *)malloc(NbinX * NbinY * sizeof(uint32_t));
	Histo->NbinX = NbinX;
	Histo->NbinY = NbinY;
	Histo->H_cnt = 0;
	Histo->Ovf_cnt = 0;
	Histo->Unf_cnt = 0;
	return 0;
}

int DestroyHistogram2D(Histogram2D_t Histo) {
	if (Histo.H_data != NULL)
		free(Histo.H_data);
	return 0;
}

int DestroyHistogram1D(Histogram1D_t Histo) {
	if (Histo.H_data != NULL)
		free(Histo.H_data);
	return 0;
}

int ResetHistogram1D(Histogram1D_t *Histo) {
	memset(Histo->H_data, 0, Histo->Nbin * sizeof(uint32_t));
	Histo->H_cnt = 0;
	Histo->Ovf_cnt = 0;
	Histo->Unf_cnt = 0;
	Histo->rms = 0;
	Histo->mean = 0;
	Histo->real_time = 0;
	Histo->live_time = 0;
	return 0;
}

int ResetHistogram2D(Histogram2D_t *Histo) {
	memset(Histo->H_data, 0, Histo->NbinX * Histo->NbinY * sizeof(uint32_t));
	Histo->H_cnt = 0;
	Histo->Ovf_cnt = 0;
	Histo->Unf_cnt = 0;
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Create histograms and allocate the memory
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CreateHistograms(uint32_t *AllocatedSize)
{
	int b, ch;

	*AllocatedSize = 0;
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch]) { 
				CreateHistogram1D(WDcfg.EHnbin, "Energy", "Channels", "Cnt", &Histos.EH[b][ch]);
				Histos.EH[b][ch].A[0] = WDcfg.ECalibration_c0[b][ch];
				Histos.EH[b][ch].A[1] = WDcfg.ECalibration_c1[b][ch];
				Histos.EH[b][ch].A[2] = WDcfg.ECalibration_c2[b][ch];
				Histos.EH[b][ch].A[3] = WDcfg.ECalibration_c3[b][ch];
				*AllocatedSize += WDcfg.EHnbin*sizeof(uint32_t);
				CreateHistogram1D(1 << WDcfg.Nbit, "Samples", "Channels", "Cnt", &Histos.SH[b][ch]);
				*AllocatedSize += (uint32_t)(1 << WDcfg.Nbit)*sizeof(uint32_t);
				CreateHistogram1D(WDcfg.THnbin, "TAC", "ns", "Cnt", &Histos.TH[b][ch]);
				*AllocatedSize += WDcfg.EHnbin*sizeof(uint32_t);
				CreateHistogram1D(1024, "PSD", "PSD", "Cnt", &Histos.PSDH[b][ch]);
				*AllocatedSize += 1024*sizeof(uint32_t);
				CreateHistogram1D(WDcfg.MCSHnbin, "MCS", "Time", "Cnt", &Histos.MCSH[b][ch]);
				*AllocatedSize += WDcfg.MCSHnbin*sizeof(uint32_t);
				CreateHistogram2D(HISTO2D_NBINX, HISTO2D_NBINY, "PSD vs E", "Energy", "PSD", &Histos.PSDvsE[b][ch]);
				*AllocatedSize +=  HISTO2D_NBINX * HISTO2D_NBINY * sizeof(uint32_t);
			}
		}
	}
	HistoCreated = 1;
	ResetHistograms();
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Destroy (freee memory) the histograms
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int DestroyHistograms()
{
	int b, ch;
	if (!HistoCreated) return 0;
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch]) { 
				DestroyHistogram1D(Histos.EH[b][ch]);
				DestroyHistogram1D(Histos.SH[b][ch]);
				DestroyHistogram1D(Histos.TH[b][ch]);
				DestroyHistogram1D(Histos.PSDH[b][ch]);
				DestroyHistogram1D(Histos.MCSH[b][ch]);
				DestroyHistogram2D(Histos.PSDvsE[b][ch]);
			}
		}
	}
	HistoCreated = 0;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Reset the histograms
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ResetHistograms()
{
	int b, ch;
	for(b=0; b<(WDcfg.NumBrd); b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if ((WDcfg.EnableInput[b][ch]) || (b >= WDcfg.NumBrd)) { 
				ResetHistogram1D(&Histos.EH[b][ch]);
				ResetHistogram1D(&Histos.SH[b][ch]);
				ResetHistogram1D(&Histos.TH[b][ch]);
				ResetHistogram1D(&Histos.PSDH[b][ch]);
				ResetHistogram1D(&Histos.MCSH[b][ch]);
				ResetHistogram2D(&Histos.PSDvsE[b][ch]);
			}
		}
	}
	return 0;
}

void readout_histograms(uint32_t ** EHistogramOut)
{
	int b, ch, bin;
	for (b = 0; b < WDcfg.NumBrd; b++) {
		for (ch = 0; ch < WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch]) {
				for (bin = 0; i < Histos.Nbin[b][ch]; bin++) {
					EHistogramOut[ch][bin] = Histos.EH[b][ch].H_data[bin];
				}
			}
		}
	}
	return;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Add one count to the histogram 1D
// Return:		0=OK, -1=under/over flow
// --------------------------------------------------------------------------------------------------------- 
int Histo1D_AddCount(Histogram1D_t *Histo, int Bin)
{
	if (Bin < 0) {
		Histo->Unf_cnt++;
		return -1;
	} else if (Bin >= (int)(Histo->Nbin-1)) {
		Histo->Ovf_cnt++;
		return -1;
	}
	Histo->H_data[Bin]++;
	Histo->H_cnt++;
	Histo->mean += (double)Bin;
	Histo->rms += (double)(Bin*Bin);
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Add one count to the histogram 1D
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int Histo2D_AddCount(Histogram2D_t *Histo, int BinX, int BinY)
{
	if ((BinX >= (int)(Histo->NbinX-1)) || (BinY >= (int)(Histo->NbinY-1))) {
		Histo->Ovf_cnt++;
		return -1;
	} else if ((BinX < 0) || (BinY < 0)) {
		Histo->Unf_cnt++;
		return -1;
	}
	Histo->H_data[BinX + HISTO2D_NBINY * BinY]++;
	Histo->H_cnt++;
	return 0;
}


