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
#include "Console.h"
#include "Plots.h"


// Global Variables
FILE *hplot=NULL, *wplot=NULL;   // gnuplot pipes
uint64_t PlotTime=0;
int LastHPlotType = -1;
int LastWPlotType = -1;
float GnuplotVersion = 0;


static int OpenGnuplot(FILE **gplot)
{
	FILE *vars = NULL;
	char str[1000];
	int i;

	if (*gplot != NULL) return 0;
	*gplot = popen(SysVars.GnuplotCmd, "w");
	if (*gplot == NULL) return -1;

	if (GnuplotVersion == 0) {
		vars = fopen("gpvars.txt", "r");
		if (vars != NULL) {
			fclose(vars);
			system("del gpvars.txt");
		}
		fprintf(*gplot, "save var 'gpvars.txt'\n");
		fflush(*gplot);
		for(i=0; i<200; i++) {
			vars = fopen("gpvars.txt", "r");
			if (vars != NULL) break;
			Sleep(10);
		}
		if (vars == NULL) return -1;
		Sleep(50);
		while (!feof(vars)) {
			fscanf(vars, "%s", str);
			if (strcmp(str, "Version") == 0) {
				fscanf(vars, "%f", &GnuplotVersion);
				break;
			}
		}
		fclose(vars);
		msg_printf(MsgLog, "INFO: using gunplot Ver. %.1f\n", GnuplotVersion);
		//if (GnuplotVersion == 0) return -1;
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Open gnuplot for waveforms and histograms
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int OpenPlotter()
{
	/* open gnuplot in a pipe and the data file */
	if (OpenGnuplot(&hplot) < 0) {
		msg_printf(MsgLog, "WARNING: Can't open gnuplot for histograms\n");
	}
	if (hplot == NULL) return -1;
#ifdef LINUX
	fprintf(hplot, "set terminal x11 noraise title 'digiTES - Spectra' size 1200,800 position 680,30\n");
#else
	if (GnuplotVersion >= 5.0)
		fprintf(hplot, "set terminal wxt noraise title 'digiTES - Spectra' size 1200,800 position 680,30\n");
#endif
	fprintf(hplot, "set grid\n");
	fprintf(hplot, "set title 'Board %d - Channel %d'\n", BrdToPlot, ChToPlot);
	fprintf(hplot, "set mouse\n");
	fprintf(hplot, "bind y 'set autoscale y'\n");
	fprintf(hplot, "bind x 'set autoscale x'\n");
	fprintf(hplot, "xc = 0\n");
	fprintf(hplot, "yc = 0\n");
	fprintf(hplot, "bind \"Button1\" 'unset arrow; xc = MOUSE_X; yc = MOUSE_Y; set arrow from xc, graph 0 to xc, graph 1 nohead; replot'\n");
	fprintf(hplot, "bind + 'set xrange [xc - (GPVAL_X_MAX-GPVAL_X_MIN)/4: xc + (GPVAL_X_MAX-GPVAL_X_MIN)/4]; replot'\n");
	fprintf(hplot, "bind - 'set xrange [xc - (GPVAL_X_MAX-GPVAL_X_MIN): xc + (GPVAL_X_MAX-GPVAL_X_MIN)]; replot'\n");
	fprintf(hplot, "bind \"Up\" 'set yrange [GPVAL_Y_MIN: GPVAL_Y_MAX/2]; replot'\n");
	fprintf(hplot, "bind \"Down\" 'set yrange [GPVAL_Y_MIN: GPVAL_Y_MAX*2]; replot'\n");
	fflush(hplot);

	if ((wplot != NULL) && !WDcfg.WaveformEnabled) {
		fprintf(wplot, "quit\n");
		fflush(wplot);
		Sleep(100);
		fclose(wplot);
		wplot = NULL;
	} else if ((wplot == NULL) && WDcfg.WaveformEnabled) {
		if (OpenGnuplot(&wplot) < 0) {
			msg_printf(MsgLog, "WARNING: Can't open gnuplot for waveforms\n");
		}
#ifdef LINUX
		fprintf(wplot, "set terminal x11 noraise title 'digiTES - Waveforms' size 1200,800 position 680,50\n");
#else
		if (GnuplotVersion >= 5.0)
			fprintf(wplot, "set terminal wxt noraise title 'digiTES - Waveforms' size 1200,800 position 680,50\n");
#endif
		fprintf(wplot, "set grid\n");
		fprintf(wplot, "set yrange [0:%d]\n", 1 << WDcfg.Nbit);
		fprintf(wplot, "set xlabel 'us'\n");
		fprintf(wplot, "set ylabel 'LSB'\n");
		fprintf(wplot, "set title 'Board %d - Channel %d'\n", BrdToPlot, ChToPlot);
		fprintf(wplot, "set mouse\n");
		fprintf(wplot, "bind y 'set yrange [0:%d]'\n", 1 << WDcfg.Nbit);
		fprintf(wplot, "bind x 'set autoscale x'\n");
		fprintf(wplot, "xc = 0\n");
		fprintf(wplot, "yc = 0\n");
		fprintf(wplot, "bind \"Button1\" 'unset arrow; xc = MOUSE_X; yc = MOUSE_Y; set arrow from xc, graph 0 to xc, graph 1 nohead; replot'\n");
		fprintf(wplot, "bind + 'set xrange [xc - (GPVAL_X_MAX-GPVAL_X_MIN)/4: xc + (GPVAL_X_MAX-GPVAL_X_MIN)/4]; replot'\n");
		fprintf(wplot, "bind - 'set xrange [xc - (GPVAL_X_MAX-GPVAL_X_MIN): xc + (GPVAL_X_MAX-GPVAL_X_MIN)]; replot'\n");
		fflush(wplot);
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Close gnuplot for waveforms and histograms
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ClosePlotter()
{
	if (wplot != NULL) {
		fprintf(wplot, "quit\n");
		fflush(wplot);
		Sleep(100);
		fclose(wplot);
	}
	if (hplot != NULL) {
		fprintf(hplot, "quit\n"); 
		fflush(hplot);
		Sleep(100);
		fclose(hplot);
	}
	hplot = NULL;
	wplot = NULL;
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Plot the waveforms of one event
// Inputs:		wplot = gnuplot pipe
//				Wfm = Event to plot
//				title = title of the plot
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PlotWaveforms(Waveform_t Wfm, char *title)
{
	int i, cl=1;
	char comma;
	FILE *wpdata;
	int dtg = (1<<WDcfg.Nbit) / 20;
	int dto = 50;
	double mean=0, rms=0;

	if (LastWPlotType != 0) {
		fprintf(wplot, "set yrange [0:%d]\n", 1 << WDcfg.Nbit);
		fprintf(wplot, "set autoscale x\n");
		fprintf(wplot, "set xlabel 'us'\n");
		fprintf(wplot, "set ylabel 'LSB'\n");
		fprintf(wplot, "bind y 'set yrange [0:%d]'\n", 1 << WDcfg.Nbit);
		LastWPlotType = 0;
	}

	wpdata = fopen("PlotData.txt", "w");
	if (wpdata == NULL) {
		printf("Can't open plot data\n");
		return -1;
	}
								
	/* Save Waveform and Plot it using gnuplot */
	for(i=0; i<(int)Wfm.Ns; i++) {
		if (Wfm.TraceSet[0] != -1) fprintf(wpdata, "%d ", (short)Wfm.AnalogTrace[0][i]);  // analog trace 1
		if (Wfm.TraceSet[1] != -1) fprintf(wpdata, "%d ", (short)Wfm.AnalogTrace[1][i]);  // analog trace 2
		if (Wfm.TraceSet[2] != -1) fprintf(wpdata, "%d ", (Wfm.DigitalTraces[i]        & 1) * dtg + dto);          // digital trace 0
		if (Wfm.TraceSet[3] != -1) fprintf(wpdata, "%d ", ((Wfm.DigitalTraces[i] >> 1) & 1) * dtg + dto + 2*dtg);  // digital trace 1
		if (Wfm.TraceSet[4] != -1) fprintf(wpdata, "%d ", ((Wfm.DigitalTraces[i] >> 2) & 1) * dtg + dto + 4*dtg);  // digital trace 2
		if (Wfm.TraceSet[5] != -1) fprintf(wpdata, "%d ", ((Wfm.DigitalTraces[i] >> 3) & 1) * dtg + dto + 6*dtg);  // digital trace 3
		fprintf(wpdata, "\n");
		mean += (double)Wfm.AnalogTrace[0][i];
		rms += (double)Wfm.AnalogTrace[0][i]*Wfm.AnalogTrace[0][i];
	}
	mean = mean / Wfm.Ns;
	rms = sqrt(rms/Wfm.Ns - mean*mean);
	fclose(wpdata);
	fprintf(wplot, "set title '1: %s - mean = %.2f, rms = %.2f'\n", title, mean, rms);
	fprintf(wplot, "plot");
	comma = ' '; // first command after "plot" doesn't have comma
	for(i=0; i<MAX_NTRACES; i++) {
		if (Wfm.TraceSet[i] != -1) {
			fprintf(wplot, "%c 'PlotData.txt' u ($0*%f):%d t 'T%d: %s' w step ls %d", comma, (float)WDcfg.Tsampl/1000.0, cl++, i+1, TraceNames[i][Wfm.TraceSet[i]], i+1);
			comma = ',';
		}
	}
	fprintf(wplot, "\n");
	fflush(wplot);
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Plot the waveforms of one event
// Inputs:		wplot = gnuplot pipe
//				Wfm = Event to plot
//				title = title of the plot
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PlotFFT(double *fft, int Ns, char *title)
{
	int i;
	FILE *wpdata;

	if (LastWPlotType != 1) {
		fprintf(wplot, "set yrange [-140:0]\n");
		fprintf(wplot, "set autoscale x\n");
		fprintf(wplot, "set xlabel 'MHz'\n");
		fprintf(wplot, "set ylabel 'Amplitude'\n");
		fprintf(wplot, "bind y 'set yrange [-140:0]'\n");
		LastWPlotType = 1;
	}

	wpdata = fopen("PlotData.txt", "w");
	if (wpdata == NULL) {
		printf("Can't open plot data\n");
		return -1;
	}
	/* Save fft and Plot it using gnuplot */
	for(i=0; i<Ns; i++) {
		fprintf(wpdata, "%f\n", fft[i]);  
	}
	fclose(wpdata);
	fprintf(wplot, "set title '1: %s'\n", title);
	fprintf(wplot, "plot 'PlotData.txt'  u ($0*%f):1 t 'fft' w step\n", (1000.0/WDcfg.Tsampl)/(2*Ns));
	fflush(wplot);
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Plot one histogram
// Inputs:		hplot = gnuplot pipe
//				Histo = histogram to plot
//				Nbin = number of bins of the histogram
//				title = title of the plot
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PlotHisto(uint32_t *Histo , int Nbin, float Xmin, float Xmax, char *title, char *xlabel)
{
	int i;
	static int WasBelow10 = 0;
	uint32_t hmax=0;
	FILE *phdata;
	float xa, xb;

	xb = Xmin;
	xa = (Xmax-Xmin)/Nbin;
	phdata = fopen("PlotHistoData.txt", "w");
	if (phdata == NULL) {
		msg_printf(MsgLog, "Can't open plot data file\n");
		return -1;
	}
	for(i=0; i<Nbin; i++) {
		fprintf(phdata, "%d  \n", Histo[i]);
		hmax = max(Histo[i], hmax);
	}
	fclose(phdata);
	fprintf(hplot, "set title '%s'\n", title);
    fprintf(hplot, "set xlabel '%s'\n", xlabel);
    fprintf(hplot, "set ylabel 'Counts'\n");
	if (hmax < 10) {
		fprintf(hplot, "set yrange [0:10]\n");
		WasBelow10 = 1;
	} else if (WasBelow10) {
		fprintf(hplot, "set autoscale y\n");
		WasBelow10 = 0;
	}
	fprintf(hplot, "plot 'PlotHistoData.txt' using ($0*%f+%f):($1) title 'BinSize = %f' with step\n", xa, xb, xa);
	fflush(hplot);
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Plot 2D histogram
// Inputs:		hplot = gnuplot pipe
//				Histo2D = Histogram to plot
//				nx, ny = number of bins of the histogram (x and y axes)
//				title = title of the plot
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PlotHisto2D(uint32_t *Histo2D, int nx, int ny, float Xmin, float Xmax, char *title, char *xlabel)
{
    int i,j;
	FILE *w1;
	uint64_t CurrentTime = get_time();


	if ((CurrentTime - PlotTime) < 5000)
		return 0;

	fprintf(hplot, "set xlabel %s\n", xlabel);
	if (WDcfg.ScatterPlotMode == SCATTER_E_VS_DELTAE)
		fprintf(hplot, "set ylabel 'deltaE'\n");
	else
		fprintf(hplot, "set ylabel 'PSD'\n");
	fprintf(hplot, "set title '%s'\n", title);
	w1 = fopen("histogram2d.txt", "w");			
	for(i=0; i<nx; i++) {
		for(j=0; j<ny; j++) {
			if (WDcfg.ScatterPlotMode == SCATTER_E_VS_DELTAE) {
				fprintf(w1, "%f %f %d\n", Xmin + i*(Xmax-Xmin)/HISTO2D_NBINX, Xmin + j*(Xmax-Xmin)/HISTO2D_NBINY, Histo2D[i+j*ny]);
			} else {
				fprintf(w1, "%f %f %d\n", Xmin + i*(Xmax-Xmin)/HISTO2D_NBINX, (float)j/HISTO2D_NBINY, Histo2D[i+j*ny]);
			}
		}
		fprintf(w1, "\n");
	}
	fclose(w1);
	Sleep(10);
	//fprintf(hplot, "unset grid; set palette model CMY rgbformulae 7,5,15\n");
	fprintf(hplot, "unset grid; set palette model CMY rgbformulae 15,7,3\n");
	fprintf(hplot, "plot 'histogram2d.txt' with image\n");
	fflush(hplot);	
	PlotTime = CurrentTime;
	
    return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Plot the selcted histogram (energy, time, MCS, psd...)
// Inputs:		HistoPlotType: gnuplot pipe
//				Xunits: 0=channels; 1=physic units (KeV for energy, ns for time, etc...)
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PlotSelectedHisto(int HistoPlotType, int Xunits)
{
	char title[500], xlabel[500];
	double mean, rms, oup=0;
	uint32_t ovf, cnt;

	if (!WDcfg.EnableInput[BrdToPlot][ChToPlot] || (BrdToPlot >= WDcfg.NumBrd)) {
		int ch, b;
		for(b=0; b<WDcfg.NumBrd; b++) {
			for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
				if (WDcfg.EnableInput[b][ch]) {
					BrdToPlot = b;
					ChToPlot = ch;
					printf("WARNING: the selected channel for plot is disabled; now plotting Brd%d/Ch%d\n", BrdToPlot, ChToPlot);
				}
			}
		}
	}

	if (HistoPlotType == HPLOT_ENERGY) {
		float c3 = WDcfg.ECalibration_c3[BrdToPlot][ChToPlot];
		float c2 = WDcfg.ECalibration_c2[BrdToPlot][ChToPlot];
		float c1 = WDcfg.ECalibration_c1[BrdToPlot][ChToPlot];
		float c0 = WDcfg.ECalibration_c0[BrdToPlot][ChToPlot];
		if (LastHPlotType != HPLOT_ENERGY) {
			fprintf(hplot, "set xrange [0:%d]\n", WDcfg.EHnbin);
			fprintf(hplot, "set autoscale y\n");
			LastHPlotType = HPLOT_ENERGY;
		}
		mean = Histos.EH[BrdToPlot][ChToPlot].mean / Histos.EH[BrdToPlot][ChToPlot].H_cnt;
		rms = sqrt(Histos.EH[BrdToPlot][ChToPlot].rms / Histos.EH[BrdToPlot][ChToPlot].H_cnt - mean*mean);
		ovf = Histos.EH[BrdToPlot][ChToPlot].Ovf_cnt + Histos.EH[BrdToPlot][ChToPlot].Unf_cnt;
		cnt = Histos.EH[BrdToPlot][ChToPlot].H_cnt;
		if ((ovf+cnt) > 0)	oup = ovf * 100.0 / (ovf + cnt);
		sprintf(title, "ENERGY Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%% - M=%.3f S=%.2f", BrdToPlot, ChToPlot, Histos.EH[BrdToPlot][ChToPlot].H_cnt, oup, mean, rms);
		if ((Xunits) && !((c1 == 1) && (c0 == 0))) {
			sprintf(xlabel, "keV");
			PlotHisto(Histos.EH[BrdToPlot][ChToPlot].H_data, WDcfg.EHnbin, c0, WDcfg.EHnbin * c1 + c0, title, xlabel);
		} else {
			sprintf(xlabel, "Channels");
			//PlotHisto(Stats.EHisto[BrdToPlot][ChToPlot], WDcfg.EHnbin, WDcfg.EHmin[BrdToPlot][ChToPlot], WDcfg.EHmax[BrdToPlot][ChToPlot], title, xlabel);
			PlotHisto(Histos.EH[BrdToPlot][ChToPlot].H_data, WDcfg.EHnbin, 0, (float)WDcfg.EHnbin, title, xlabel);
		}

	} else if (HistoPlotType == HPLOT_SAMPLES) {
		if (LastHPlotType != HPLOT_SAMPLES) {
			fprintf(hplot, "set xrange [0:%d]\n", 1 << WDcfg.Nbit);
			fprintf(hplot, "set autoscale y\n");
			LastHPlotType = HPLOT_SAMPLES;
		}
		mean = Histos.SH[BrdToPlot][ChToPlot].mean / Histos.SH[BrdToPlot][ChToPlot].H_cnt;
		rms = sqrt(Histos.SH[BrdToPlot][ChToPlot].rms / Histos.SH[BrdToPlot][ChToPlot].H_cnt - mean*mean);
		ovf = Histos.SH[BrdToPlot][ChToPlot].Ovf_cnt + Histos.SH[BrdToPlot][ChToPlot].Unf_cnt;
		cnt = Histos.SH[BrdToPlot][ChToPlot].H_cnt;
		if ((ovf+cnt) > 0)	oup = ovf * 100.0 / (ovf + cnt);
		sprintf(title, "SAMPLES Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%% - M=%.3f S=%.2f", BrdToPlot, ChToPlot, Histos.EH[BrdToPlot][ChToPlot].H_cnt, oup, mean, rms);
		sprintf(xlabel, "Channels");
		PlotHisto(Histos.SH[BrdToPlot][ChToPlot].H_data, 1 << WDcfg.Nbit, 0, (float)(1 << WDcfg.Nbit), title, xlabel);

	} else if (HistoPlotType == HPLOT_MCS) {
		if (LastHPlotType != HPLOT_MCS) {
			fprintf(hplot, "set xrange [0:%d]\n", WDcfg.MCSHnbin);
			fprintf(hplot, "set autoscale y\n");
			LastHPlotType = HPLOT_MCS;
		}
		sprintf(title, "MCS Brd-%d Ch-%d: Cnt=%d", BrdToPlot, ChToPlot, Histos.MCSH[BrdToPlot][ChToPlot].H_cnt);
		if (Xunits) {
			sprintf(xlabel, "Time (ms)");
			PlotHisto(Histos.MCSH[BrdToPlot][ChToPlot].H_data, WDcfg.MCSHnbin, 0, (float)WDcfg.MCSHnbin*WDcfg.DwellTime/1000, title, xlabel);
		} else {
			sprintf(xlabel, "Channels");
			PlotHisto(Histos.MCSH[BrdToPlot][ChToPlot].H_data, WDcfg.MCSHnbin, 0, (float)WDcfg.MCSHnbin, title, xlabel);
		}

	} else if (HistoPlotType == HPLOT_PSD) {
		if (LastHPlotType != HPLOT_PSD) {
			if (Xunits) fprintf(hplot, "set xrange [0:1]\n");
			else fprintf(hplot, "set xrange [0:1024]\n");
			fprintf(hplot, "set autoscale y\n");
			LastHPlotType = HPLOT_PSD;
		}
		sprintf(title, "PSD Brd-%d Ch-%d: Cnt=%d", BrdToPlot, ChToPlot, Histos.PSDH[BrdToPlot][ChToPlot].H_cnt);
		sprintf(xlabel, "PSD");
		if (Xunits) PlotHisto(Histos.PSDH[BrdToPlot][ChToPlot].H_data, 1024, 0, 1, title, xlabel);
		else		PlotHisto(Histos.PSDH[BrdToPlot][ChToPlot].H_data, 1024, 0, 1024, title, xlabel);

	} else if (HistoPlotType == HPLOT_TIME) {
		mean = Histos.TH[BrdToPlot][ChToPlot].mean / Histos.TH[BrdToPlot][ChToPlot].H_cnt;
		rms = sqrt(Histos.TH[BrdToPlot][ChToPlot].rms / Histos.TH[BrdToPlot][ChToPlot].H_cnt - mean*mean);
		ovf = Histos.TH[BrdToPlot][ChToPlot].Ovf_cnt + Histos.TH[BrdToPlot][ChToPlot].Unf_cnt;
		cnt = Histos.TH[BrdToPlot][ChToPlot].H_cnt;
		if (LastHPlotType != HPLOT_TIME) {
			fprintf(hplot, "set autoscale x\n");
			fprintf(hplot, "set autoscale y\n");
			LastHPlotType = HPLOT_TIME;
		}
		if ((ovf+cnt) > 0)	oup = ovf * 100.0 / (ovf + cnt);
		if (Xunits) {
			float tbin = (WDcfg.THmax[BrdToPlot][ChToPlot] - WDcfg.THmin[BrdToPlot][ChToPlot]) / WDcfg.THnbin;
			sprintf(title, "TAC Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%% - M=%.3f ns, S=%.2f ps", BrdToPlot, ChToPlot, Histos.TH[BrdToPlot][ChToPlot].H_cnt, oup, WDcfg.THmin[BrdToPlot][ChToPlot]+tbin*mean, tbin*rms*1000);
			sprintf(xlabel, "ns");
			PlotHisto(Histos.TH[BrdToPlot][ChToPlot].H_data, WDcfg.THnbin, WDcfg.THmin[BrdToPlot][ChToPlot], WDcfg.THmax[BrdToPlot][ChToPlot], title, xlabel);
		} else {
			sprintf(title, "TAC Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%% - M=%.3f, S=%.2f", BrdToPlot, ChToPlot, Histos.TH[BrdToPlot][ChToPlot].H_cnt, oup, mean, rms);
			sprintf(xlabel, "Channels");
			PlotHisto(Histos.TH[BrdToPlot][ChToPlot].H_data, WDcfg.THnbin, 0, (float)WDcfg.THnbin, title, xlabel);
		}

	} else if (HistoPlotType == HPLOT_ENERGYvsPSD) {
		float c3 = WDcfg.ECalibration_c3[BrdToPlot][ChToPlot];
		float c2 = WDcfg.ECalibration_c2[BrdToPlot][ChToPlot];
		float c1 = WDcfg.ECalibration_c1[BrdToPlot][ChToPlot];
		float c0 = WDcfg.ECalibration_c0[BrdToPlot][ChToPlot];
		if (LastHPlotType != HPLOT_ENERGYvsPSD) {
			fprintf(hplot, "set xrange [0:%d]\n", WDcfg.EHnbin);
			if (WDcfg.ScatterPlotMode == SCATTER_E_VS_DELTAE) {
				fprintf(hplot, "set yrange [0:%d]\n", WDcfg.EHnbin);
			} else {
				fprintf(hplot, "set yrange [0:1]\n");
			}
			LastHPlotType = HPLOT_ENERGYvsPSD;
		}
		ovf = Histos.PSDvsE[BrdToPlot][ChToPlot].Ovf_cnt + Histos.PSDvsE[BrdToPlot][ChToPlot].Unf_cnt;
		cnt = Histos.PSDvsE[BrdToPlot][ChToPlot].H_cnt;
		if ((ovf+cnt) > 0)	oup = ovf * 100.0 / (ovf + cnt);
		if (WDcfg.ScatterPlotMode == SCATTER_E_VS_DELTAE) {
			sprintf(title, "E vs deltaE Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%%)", BrdToPlot, ChToPlot, Histos.PSDH[BrdToPlot][ChToPlot].H_cnt, oup);
		} else {
			sprintf(title, "Energy vs PSD Brd-%d Ch-%d: Cnt=%d Ovf=%.1f%%)", BrdToPlot, ChToPlot, Histos.PSDH[BrdToPlot][ChToPlot].H_cnt, oup);
		}
		if ((Xunits) && !((c1 == 1) && (c0 == 0))) {
			sprintf(xlabel, "keV");
			PlotHisto2D(Histos.PSDvsE[BrdToPlot][ChToPlot].H_data, HISTO2D_NBINX, HISTO2D_NBINY, c0, WDcfg.EHnbin * c1 + c0, title, xlabel);
		} else {
			sprintf(xlabel, "Channels");
			PlotHisto2D(Histos.PSDvsE[BrdToPlot][ChToPlot].H_data, HISTO2D_NBINX, HISTO2D_NBINY, 0, (float)WDcfg.EHnbin, title, xlabel);
		}		
	}
	return 0;
}
