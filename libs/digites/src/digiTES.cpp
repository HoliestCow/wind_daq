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

// This progam can be dowloaded from https://goo.gl/exoNsa

#define digiTES_Revision    "4.5.13"  // 19_September_2018

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <CAENDigitizer.h>

#include "digiTES.h"
#include "Console.h"
#include "Histograms.h"
#include "Plots.h"
#include "Queues.h"
#include "Readout.h"
#include "PreProcess.h"
#include "DataFiles.h"
#include "ParamParser.h"
#include "Configure.h"
#include "BoardUtils.h"
#include "ZCcal.h"
#include "Statistics.h"
#include "fft.h"


/* ###########################################################################
*  Global Variables
*  ########################################################################### */
Config_t		WDcfg;		// acquisition parameters and user settings
Stats_t			Stats;		// variables for the statistics
Histos_t		Histos;		// histograms (spectra)
SysVars_t       SysVars;	// system variables

int Quit=0;
int AcqRun=0;
int Stopping=0;
int Failure=0;
int RestartAll=0;
int OvwrRunNumber=-1;			// Overwrite Run number 
int CalibrRun = 0;				// ZC calibration Run
int ChToPlot=0, BrdToPlot=0;
int ContinuousTrigger=0;
int SingleTrigger=0;
int DoRefresh=1;
int DoRefreshSingle=0;
int StatsMode=0;
int IntegratedRates=0;
int SkipCalibration=0;
int HistoPlotType=HPLOT_ENERGY;
int WavePlotType=WPLOT_DISABLED;
int ForceWaveMode=0;
int Xunits=0;
int handle[MAX_NBRD];           // board handles 
int StopCh[MAX_NBRD][MAX_NCH];	// Individual Stop Acquisition (based on time or counts)
char ConfigFileName[500];		// Config file name

int TraceSet[MAX_NTRACES];
char TraceNames[MAX_NTRACES][MAX_NTRSETS][20];

FILE *cntlog=NULL;
FILE *MsgLog=NULL;


/* ###########################################################################
*  Functions
*  ########################################################################### */
// --------------------------------------------------------------------------------------------------------- 
// Load/Add config file from/to a list 
// --------------------------------------------------------------------------------------------------------- 
#define MAX_NUM_CFG_FILES 50
int ImpExpCfgFile()
{
	char cflname[500] = "cfg_files.txt";
	char fname[MAX_NUM_CFG_FILES][500];
	char fdescr[MAX_NUM_CFG_FILES][500];
	char *fp, buff[500];
	int nf=0, i;
	FILE *cfglist, *ncf, *cf;

	cfglist = fopen(cflname, "r");
	if (cfglist == NULL) {
		printf("Can't find file '%s' with the config file list\n", cflname);
		Sleep(1000);
		return -1;
	}
	printf("\n\nAvailable configuration files: \n");
	for(i=0; i<MAX_NUM_CFG_FILES; i++) {
		if (feof(cfglist)) break;
		if (fscanf(cfglist, "%s", fname[i]) < 1) break;
		if (strlen(fname[i]) == 0) break;
		fgets(buff, 500, cfglist);
		if (strlen(buff) == 0) break;
		if (buff[strlen(buff)-1] == '\n') buff[strlen(buff)-1] = '\0';
		fp = buff;
		while (*fp != '"') fp++;
		strcpy(fdescr[i], fp);
		ncf = fopen(fname[i], "r");  // try to read the file to check that it exists
		if (ncf == NULL) continue;
		fclose(ncf);
		printf("%2d - %s\n", i, fdescr[i]);
		nf++;
	}
	fclose(cfglist);

	printf("\nEnter the file number to load a Configuration file from the list\n");
	printf("or 'a' to add the current Configuration file to the list\n");
	printf("Option: ");
	scanf("%s", buff);
	if (tolower(buff[0]) == 'a') {
		if (nf >= MAX_NUM_CFG_FILES) {
			printf("Max number of configuration files reached. Can't add more\n");
			Sleep(1000);
			return -1;
		}
		printf("Enter filename of the new Configuration (no spaces): ");
		scanf("%s", fname[nf]);
		printf("Enter description of the new Configuration: ");
		GetString(fdescr[nf], 500);
		if ((strlen(fdescr[nf]) > 0) && (fdescr[nf][strlen(fdescr[nf])-1] == '\n')) fdescr[nf][strlen(fdescr[nf])-1] = '\0';
		// make a copy of the config file
		ncf = fopen(fname[nf], "w");
		cf = fopen(ConfigFileName, "r");
		if ((cf == NULL) || (ncf == NULL)) {
			printf("Can't save file '%s'\n", fname[nf]);
			Sleep(1000);
			return -1;
		}
		while(!feof(cf)) {
			if (fgets(buff, 1000, cf) != NULL)
				fputs(buff, ncf);
		}
		fclose(cf);
		fclose(ncf);
		nf++;
		// add cfg file to the list
		cfglist = fopen(cflname, "w");
		if (cfglist == NULL) {
			printf("Can't write file '%s' with the config file list\n", cflname);
			Sleep(1000);
			return -1;
		}
		for(i=0; i<nf; i++) {
			if (fdescr[i][0] == '"') fprintf(cfglist, "%s %s\n", fname[i], fdescr[i]);
			else fprintf(cfglist, "%s \"%s\"\n", fname[i], fdescr[i]);
		}
		fclose(cfglist);
		return 0;
	} else {
		sscanf(buff, "%d", &i);
		if ((i<0) || (i>=nf)) {
			printf("Option %d is not available\n", i);
			Sleep(1000);
			return -1;
		} else {
			printf("\nWarning: overwriting '%s' with %s. \nPress 'y' to continue\n", ConfigFileName, fname[i]);
			if (tolower(getch()) != 'y') return -1;
			ncf = fopen(fname[i], "r");
			cf = fopen(ConfigFileName, "w");
			if (cf == NULL) {
				printf("The file '%s' is used by another program; can't update it\n", ConfigFileName);
				Sleep(1000);
				return -1;
			}
			while(!feof(ncf)) {
				if (fgets(buff, 1000, ncf) != NULL)
					fputs(buff, cf);
			}
			fclose(cf);
			fclose(ncf);
			return 1;
		}
	}
}

// --------------------------------------------------------------------------------------------------------- 
// save plot settings
// --------------------------------------------------------------------------------------------------------- 
int SavePlotSettings()
{
	FILE *rt;
	int i;
	char fn[500];
	sprintf(fn, "%srtset1.txt", CONFIG_FILE_PATH);
	rt = fopen(fn, "w");
	if (rt==NULL)
		return -1;
	fprintf(rt, "%d %d %d %d %d %d %d\n", RTSET_VERSION, WDcfg.DppType, ChToPlot, BrdToPlot, 1, HistoPlotType, Xunits);
	for (i=0; i<MAX_NTRACES; i++)
		fprintf(rt, "%d ", TraceSet[i]);
	fclose(rt);
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// load plot settings
// --------------------------------------------------------------------------------------------------------- 
int LoadPlotSettings()
{
	FILE *rt;
	int i, ver, dpptype;
	char fn[500];
	sprintf(fn, "%srtset1.txt", CONFIG_FILE_PATH);
	rt = fopen(fn, "r");
	if (rt==NULL)
		return -1;
	msg_printf(MsgLog, "INFO: Reading run time settings from '%s'\n", fn);
	fscanf(rt, "%d\n", &ver);
	if (ver > 1) {
		int unused;
		if (ver == 1)
			fscanf(rt, "%d %d %d %d %d\n", &dpptype, &ChToPlot, &BrdToPlot, &unused, &HistoPlotType);
		else
			fscanf(rt, "%d %d %d %d %d %d\n", &dpptype, &ChToPlot, &BrdToPlot, &unused, &HistoPlotType, &Xunits);
		if (dpptype != WDcfg.DppType)
			return -1;
		for (i=0; i<MAX_NTRACES; i++)
			fscanf(rt, "%d ", &TraceSet[i]);
		SetVirtualProbes(BrdToPlot);
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// load system variables
// --------------------------------------------------------------------------------------------------------- 
int LoadSysVars()
{
	FILE *sv;
	char varname[100];

	// set defaults
	SysVars.CheckHeaderErrorBit = 0;
	SysVars.MaxOutputFileDataSize = 1024; // 1 GB
	SysVars.FineTstampMode = 1;
	SysVars.UseRollOverFakeEvents = 1;
	SysVars.InactiveTimeout = 1000; // ms
	SysVars.ZCcorr_Ncnt = 1024*256;
	SysVars.AutoReloadPlotsettings = 0;
	SysVars.ImmediateStart = 1;
	SysVars.AutoRestartOnCfgChange = 1;
	SysVars.HistoAutoSave = 60;  // save every minute
	SysVars.HVmax = 5100;
	sprintf(SysVars.ZCcalibrFileName, "%szcc.dat", WORKING_DIR);
	sprintf(SysVars.GnuplotCmd, "%s", DEFAULT_GNUPLOT_PATH);
	sv = fopen("sysvars.txt", "r");
	if (sv==NULL)
		return 0;

	//~ msg_printf(MsgLog, "INFO: Reading system variables from 'sysvars.txt' \n");
	while(!feof(sv)) {
		fscanf(sv, "%s", varname);
		if (strcmp(varname, "UseRollOverFakeEvents")==0)	fscanf(sv, "%d", &SysVars.UseRollOverFakeEvents); 
		if (strcmp(varname, "FineTstampMode")==0)			fscanf(sv, "%d", &SysVars.FineTstampMode); 
		if (strcmp(varname, "CheckHeaderErrorBit")==0)		fscanf(sv, "%d", &SysVars.CheckHeaderErrorBit); 
		if (strcmp(varname, "MaxOutputFileDataSize")==0)	fscanf(sv, "%d", &SysVars.MaxOutputFileDataSize); 
		if (strcmp(varname, "ImmediateStart")==0)			fscanf(sv, "%d", &SysVars.ImmediateStart); 
		if (strcmp(varname, "AutoRestartOnCfgChange")==0)	fscanf(sv, "%d", &SysVars.AutoRestartOnCfgChange); 
		if (strcmp(varname, "AutoReloadPlotsettings")==0)	fscanf(sv, "%d", &SysVars.AutoReloadPlotsettings); 
		if (strcmp(varname, "InactiveTimeout")==0)			fscanf(sv, "%d", &SysVars.InactiveTimeout); 
		if (strcmp(varname, "HistoAutoSave")==0)			fscanf(sv, "%d", &SysVars.HistoAutoSave); 
		if (strcmp(varname, "HVmax")==0)					fscanf(sv, "%d", &SysVars.HVmax); 
		if (strcmp(varname, "ZCcorr_Ncnt")==0)				fscanf(sv, "%d", &SysVars.ZCcorr_Ncnt); 
		if (strcmp(varname, "ZCcalibrFileName")==0) {
			char fname[500];
			fscanf(sv, "%s", fname); 
			sprintf(SysVars.ZCcalibrFileName, "%s%s", WORKING_DIR, fname);
		}
	}
	fprintf(MsgLog, "INFO: System Variables (some of them):\n");
	fprintf(MsgLog, "INFO:   HistoAutoSave = %d\n", SysVars.HistoAutoSave);
	fprintf(MsgLog, "INFO:   MaxOutputFileDataSize = %d\n", SysVars.MaxOutputFileDataSize);
	fprintf(MsgLog, "INFO:   FineTstampMode = %d\n", SysVars.FineTstampMode);
	fprintf(MsgLog, "INFO:   UseRollOverFakeEvents = %d\n", SysVars.UseRollOverFakeEvents);
	fprintf(MsgLog, "INFO:   ZCcorr_Ncnt = %d\n", SysVars.ZCcorr_Ncnt);
	fprintf(MsgLog, "INFO:   ZCcalibrFileName = %s\n", SysVars.ZCcalibrFileName);
	fprintf(MsgLog, "INFO:   HVmax = %d\n", SysVars.HVmax);
	fprintf(MsgLog, "INFO:   GnuplotCmd = %s\n\n", SysVars.GnuplotCmd);
	return 0;
}


// make a string with frequency value and units
void FreqUnits(float freq, char str[1000])
{
	if (freq >= 1000000) 
		sprintf(str, "%6.2f MHz", freq/1000000);
	else if (freq >= 1000) 
		sprintf(str, "%6.2f KHz", freq/1000);
	else
		sprintf(str, "%6.2f Hz ", freq);
}

// Header string (static text, one per board)
void BoardLogString(int b, int StatsMode, char *str)
{
	//                                                |   |             |          |        |        |          |
	if (WDcfg.CalibrationRun)	sprintf(str, "Brd  Ch |   Throughput    OCR        Match%%  ZCcal-Progress%%     TotCnt");
	else if (StatsMode==0)		sprintf(str, "Brd  Ch |   Throughput    ICR        OCR      Match%%  DeadT%%     TotCnt");
	else if (StatsMode==1)		sprintf(str, "Brd  Ch |    Satur%%     Ovf%%  UnCorr%%    Busy%%   Queue%%        DeltaCnt");
}

// channel string reporting the statistics
void ChannelLogString(int b, int ch, int StatsMode, char *str)
{
	char satur;
	char ecrs[100], ocrs[100], icrs[100];
	float satperc=0, ovfperc=0, unmperc=0;
	uint64_t nev, totnev;

	totnev = Stats.EvRead_cnt[b][ch];
	nev = Stats.EvRead_dcnt[b][ch];

	FreqUnits(Stats.EvRead_rate[b][ch],  ecrs);
	if (Stats.EvInput_rate[b][ch] < 0)	
		sprintf(icrs, "   N.A.   ");
	else 
		FreqUnits(Stats.EvInput_rate[b][ch], icrs);
	FreqUnits(Stats.EvOutput_rate[b][ch],  ocrs);
	if (Stats.EvRead_rate[b][ch]>0)
		satperc = 100 * (float)Stats.Satur_rate[b][ch]/Stats.EvRead_rate[b][ch];
	if (satperc > 100) satperc = 100;
	satur = satperc > 1 ? 'S' : ' ';

	if (Stats.EvRead_rate[b][ch]>0)
		ovfperc = 100 * (float)Stats.EvOvf_rate[b][ch]/Stats.EvRead_rate[b][ch];
	if (ovfperc > 100) ovfperc = 100;

	if (Stats.EvRead_rate[b][ch]>0)
		unmperc = 100 * (float)Stats.EvUncorrel_rate[b][ch]/Stats.EvRead_rate[b][ch];
	if (unmperc > 100) unmperc = 100;

	if ((ch < WDcfg.NumPhyCh) || (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE))
		sprintf(str, "%3d %2d  | %c", b, ch, satur);
	else
		sprintf(str, "%3d %2d  $ %c", b, ch, satur);
	if (!WDcfg.EnableInput[b][ch]) {			
		sprintf(str, "%s   Disabled", str);
	} else if (StopCh[b][ch] || (!AcqRun)) {
		sprintf(str, "%s   Stopped: Last time(s) = %-10.2f             %10lu", str, (float)Stats.LatestProcTstamp[b][ch]/1e9, Stats.EvRead_cnt[b][ch]);
	} else if (WDcfg.CalibrationRun) {
		//                                              ECR   OCR   Match%%                        ZCcal-Progress%%       TotCnt");
		sprintf(str, "%s %s %s    %6.2f%%        %d%%    %10lu", str, ecrs, ocrs, 100*Stats.MatchingRatio[b][ch], ZCcal_GetFillingProgress(b, ch), totnev);
	} else if (StatsMode==0) {
		//                                                    ECR   ICR   OCR   Match%%                         DeadT%%                  DeltaCnt");
		sprintf(str, "%s %s %s %s %6.2f%% %6.2f%% %10lu", str, ecrs, icrs, ocrs, 100*Stats.MatchingRatio[b][ch], 100*Stats.DeadTime[b][ch], totnev);
	} else if (StatsMode==1) {  
		//                                                                    Satur%%  Ovf%%  Uncorreled%%  Busy%%                     QueueOccup%%              EcCnt");
		sprintf(str, "%s %6.2f%%  %6.2f%%  %6.2f%%  %6.2f%%  %6.2f%%      %10lu", str, satperc, ovfperc, unmperc, 100*Stats.BusyTime[b][ch], GetQueueOccupancy(b, ch), nev);
	}

}


// --------------------------------------------------------------------------------------------------------- 
// keyboard menu
// ---------------------------------------------------------------------------------------------------------
// Consider making this CheckState().
void CheckState(int *state, FILE * logging)
{
	char c=0, c1;
	int ch, b, i, t;
	CAEN_DGTZ_BoardInfo_t BoardInfo;		
	char enastring[2][10] = {"DISABLED", "ENABLED"};
	char cmd[200], ext[10];
	msg_printf(logging, "before ifs, value is %d\n", *state);
	if (*state == 1) {
		msg_printf(logging, "in 1\n");
		ResetStatistics();
		StartAcquisition();
		ClearQueues();
		AcqRun = 1;
		*state = 0;
	} else if (*state == 2) {
		msg_printf(logging, "in 2\n");
		StopAcquisition();
		Stopping = 1;
		*state = 0;
	} else if (*state == 3) {
		msg_printf(logging, "in 3\n");
		StopAcquisition();
		Stopping = 1;
		*state = 0;
		Quit = 1;
	}
	msg_printf(logging, "after ifs\n");

	//~ // Check keyboard
	//~ if (kbhit()) {
		//~ c = getch();
		//~ menu:
		//~ switch(c) {

		//~ case '1':
		//~ case '2':
		//~ case '3':
		//~ case '4':
		//~ case '5':
		//~ case '6':
			//~ t = c - '1';
			//~ if (c != '1')
				//~ printf("x - OFF\n");
			//~ for(i=0; i<MAX_NTRSETS; i++) {
				//~ if (TraceNames[t][i][0] == '-')
					//~ continue;
				//~ printf("%x - %s\n", i, TraceNames[t][i]);
			//~ }
			//~ c1 = getch();
			//~ if ((c1 == 'x') && (c != '1'))
				//~ TraceSet[t] = -1;
			//~ else if ((c1 >= '0') && (c1 <= '9'))
				//~ TraceSet[t] = (int)(c1 - '0');
			//~ else
				//~ TraceSet[t] = (int)(c1 - 'a' + 10);
			//~ SetVirtualProbes(BrdToPlot);
			//~ Sleep(100);
			//~ break;

		//~ case 'b':  
			//~ printf("Enter board number: ");
			//~ scanf("%d", &b);
			//~ if ((b >= 0) && (b<WDcfg.NumBrd)) {
				//~ BrdToPlot = b;
				//~ printf("Active Board for plotting is now %d\n", BrdToPlot);
				//~ SetVirtualProbes(BrdToPlot);
			//~ } else {
				//~ printf("%d is an invalid board number\n", b);
			//~ }
			//~ break;

		//~ case 'B':
			//~ if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
				//~ for(b=0; b<WDcfg.NumBrd; b++) {
					//~ if (WDcfg.LinkType[b] != VIRTUAL_BOARD_TYPE) {
						//~ CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
						//~ printf("Digitizer n.%d: Model %s, SerNum %d\n", b, BoardInfo.ModelNameclasses and, BoardInfo.SerialNumber);
						//~ printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
						//~ printf("AMC FPGA Release is %s (%s)\n", BoardInfo.AMC_FirmwareRel, WDcfg.FwTypeString);
						//~ if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) {
							//~ uint32_t t1, t2;
							//~ CAEN_DGTZ_ReadRegister(handle[b], 0x10A8, &t1);  // read temp from ch0 (highest)
							//~ CAEN_DGTZ_ReadRegister(handle[b], 0x10A8 + ((WDcfg.NumPhyCh-1) << 8), &t2);  // read temp from last ch
							//~ printf("Temperature: ch0 = %d ch%d = %d\n\n", t1, WDcfg.NumPhyCh-1, t2);
						//~ }
					//~ } else {
						//~ printf("Virtual Board n.%d connected to Digitizer n. %d\n", b, WDcfg.LinkNum[b]);
					//~ }
				//~ }
				//~ getch();
			//~ }
			//~ break;

		//~ case 'c':  
			//~ printf("Enter Channel number: ");
			//~ scanf("%d", &ch);
			//~ if ((ch >= 0) && (ch < WDcfg.NumAcqCh)) {
				//~ ChToPlot = ch;
				//~ printf("Active Channel for plotting is now %d of Board %d\n", ChToPlot, BrdToPlot);
			//~ } else {
				//~ printf("%d is an invalid channel number\n", ch);
			//~ }
			//~ break;

		//~ case 'C':
			//~ if (ImpExpCfgFile() == 1) {
				//~ Quit = 1; 
				//~ RestartAll = 1;
			//~ }
			//~ break;

		//~ case 'd':
			//~ HistoPlotType = HPLOT_TIME;
			//~ break;

		//~ case 'D':
			//~ WDcfg.SaveRawData = WDcfg.SaveRawData ? 0 : 1;
			//~ printf("Raw Data Saving = %d\n", WDcfg.SaveRawData);
			//~ break;

		//~ case 'E':
			//~ system(EDIT_CFG_FILE);
			//~ break;

		//~ case 'e':
			//~ HistoPlotType = HPLOT_ENERGY;
			//~ break;

		//~ case 'f':
			//~ DoRefresh ^= 1;
			//~ if (!DoRefresh) printf("Plots and Logs refresh is now disabled; press 'o' for single shots!\n");
			//~ break;

		//~ case 'F':
			//~ if ((WavePlotType == WPLOT_WAVEFORMS) && (WDcfg.AcquisitionMode != ACQ_MODE_LIST)) {
				//~ WavePlotType = WPLOT_FFT;
			//~ } else if (WavePlotType == WPLOT_FFT) {
				//~ int nfft;
				//~ printf("Enter Number of cycles for the mean: ");
				//~ scanf("%d", &nfft);
				//~ SetFFTaverage(nfft);
			//~ } else {
				//~ printf("Waveform readout not enabled; press 'w' to enable it");
				//~ Sleep(500);
			//~ }
			//~ break;

		//~ case 'g':
//~ #ifdef WIN32
			//~ system("start pythonw TriDAQ.py");
//~ #else
			//~ system("python3 TriDAQ.py &");
//~ #endif
			//~ break;

		//~ case 'h':
			//~ SaveAllHistograms();
			//~ break;

		//~ case 'H':
			//~ HVsettings();
			//~ break;

		//~ case 'i':
			//~ IntegratedRates ^= 1;
			//~ if (IntegratedRates) printf("Statistics mode: integral\n");
			//~ else printf("Statistics mode: instantaneous\n");
			//~ break;

		//~ case 'I':
			//~ printf("Enter Run Description: ");
			//~ GetString(WDcfg.RunDescription, MAX_RUN_DESCR_LENGTH);
			//~ if ((strlen(WDcfg.RunDescription) > 0) && (WDcfg.RunDescription[strlen(WDcfg.RunDescription)-1] == '\n')) WDcfg.RunDescription[strlen(WDcfg.RunDescription)-1] = '\0';
			//~ break;

		//~ case 'j':
			//~ HistoPlotType = HPLOT_SAMPLES;
			//~ break;

		//~ case 'k':
			//~ SaveAllHistograms();
			//~ if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_ANSI42) sprintf(ext, "n42");
			//~ else sprintf(ext, "txt");
			//~ sprintf(cmd, "start keview DataFiles\\Run%d_Ehisto_%d_%d.%s", WDcfg.RunNumber, BrdToPlot, ChToPlot, ext);
			//~ system(cmd);
			//~ break;

		//~ case 'K':
			//~ SaveAllHistograms();
			//~ if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_ANSI42) sprintf(ext, "n42");
			//~ else sprintf(ext, "txt");
			//~ sprintf(cmd, "start keview");
			//~ for(ch=0; ch<WDcfg.NumAcqCh; ch++)
				//~ if (WDcfg.EnableInput[BrdToPlot][ch])
					//~ sprintf(cmd, "%s DataFiles\\Run%d_Ehisto_%d_%d.%s", cmd, WDcfg.RunNumber, BrdToPlot, ch, ext);
			//~ system(cmd);
			//~ break;

		//~ case 'l':
			//~ if (!WDcfg.SaveLists) {
				//~ WDcfg.SaveLists = 0xFF;
				//~ printf("List Saving Enabled\n");
			//~ } else {
				//~ WDcfg.SaveLists = 0;
				//~ printf("List Saving Disabled\n");
			//~ }
			//~ break;

		//~ case 'm':
			//~ HistoPlotType = HPLOT_MCS;
			//~ break;

		//~ case 'M':
			//~ ManualController();
			//~ break;

		//~ case 'o':
			//~ DoRefresh = 0;
			//~ DoRefreshSingle = 1;
			//~ break;

		//~ case 'p':
			//~ HistoPlotType = HPLOT_PSD;
			//~ break;

		//~ case 'P':
			//~ HistoPlotType = HPLOT_ENERGYvsPSD;
			//~ break;

		//~ case 'q':  
			//~ Quit = 1; 
			//~ break;

		//~ case 'r':
			//~ printf("Use capital 'R' instead of 'r'\n");
			//~ Sleep(200);
			//~ break;

		//~ case 'R':
			//~ printf("Are you really sure (press 'y' for yes)?\n");
			//~ c = getch();
			//~ if (c == 'y') 
				//~ ResetStatistics();
				//~ ClearQueues();
			//~ break;

		//~ case 's':
			//~ // Start/Stop Acquisition
			//~ if (AcqRun) {
				//~ StopAcquisition();
				//~ Stopping = 1;
			//~ } else {
				//~ ResetStatistics();
				//~ StartAcquisition();
				//~ ClearQueues();
				//~ AcqRun = 1;
				//~ printf("Acquisition Restarted\n");
			//~ }
			//~ break;

		//~ case 'S':
			//~ FILE *runnum;
			//~ for(;;) {
				//~ printf("Current run = %d. Enter new Run Number: ", WDcfg.RunNumber);
				//~ scanf("%d", &WDcfg.RunNumber);
				//~ OvwrRunNumber = WDcfg.RunNumber;
				//~ if (CheckOutputDataFilePresence() < 0) {
					//~ printf("Run %d already present. Overwrite [y/n]?\n", WDcfg.RunNumber);
					//~ if (getch() == 'y') break;
				//~ } else {
					//~ break;
				//~ }
			//~ }
			//~ printf("Enter Run Description: ");
			//~ GetString(WDcfg.RunDescription, MAX_RUN_DESCR_LENGTH);
			//~ if ((strlen(WDcfg.RunDescription) > 0) && (WDcfg.RunDescription[strlen(WDcfg.RunDescription)-1] == '\n')) WDcfg.RunDescription[strlen(WDcfg.RunDescription)-1] = '\0';
			//~ runnum = fopen("RunNumber.txt", "w");
			//~ fprintf(runnum, "%d", WDcfg.RunNumber+1);
			//~ fclose(runnum);
			//~ Quit = 1; 
			//~ RestartAll = 1;
			//~ printf("Acquisition Restarted\n");
			//~ break;

		//~ case 'T':
			//~ ContinuousTrigger = ContinuousTrigger ? 0 : 1;
			//~ printf("Continuous Trigger = %d\n", ContinuousTrigger);
			//~ break;

		//~ case 't':
			//~ SingleTrigger = 1;
			//~ break;

		//~ case 'x':
			//~ Xunits ^= 1;
			//~ break;

		//~ case 'X':
			//~ HistoPlotType = HPLOT_DISABLED;
			//~ break;

		//~ case 'u':
			//~ printf("Enter Update Time (in ms): ");
			//~ scanf("%d", &WDcfg.StatUpdateTime);
			//~ break;

		//~ case 'w':
			//~ if (WavePlotType == WPLOT_FFT) {
				//~ WavePlotType = WPLOT_WAVEFORMS;
			//~ } else if (WDcfg.AcquisitionMode == ACQ_MODE_LIST) {
				//~ printf("The waveform readout is not enabled; restart acquisition to enable [y/n]?\n");
				//~ if (getch() != 'y') break;
				//~ WavePlotType = WPLOT_WAVEFORMS;
				//~ ForceWaveMode = 1;
				//~ Quit = 1; 
				//~ RestartAll = 1;
				//~ AcqRun = 0;
			//~ } else if ((WavePlotType != WPLOT_DISABLED) && (WDcfg.AcquisitionMode == ACQ_MODE_MIXED)) {
				//~ printf("The waveform readout is enabled; restart acquisition to disable [y/n]?\n");
				//~ if (getch() != 'y') break;
				//~ WavePlotType = WPLOT_DISABLED;
				//~ ForceWaveMode = 1;
				//~ Quit = 1; 
				//~ RestartAll = 1;
				//~ AcqRun = 0;
			//~ } else {
				//~ WavePlotType = WPLOT_WAVEFORMS;
			//~ }
			//~ break;

		//~ /*
		//~ case 'z':  // ZC calibration run
			//~ WDcfg.CalibrationRun ^= 1;
			//~ if (WDcfg.CalibrationRun) {
				//~ CalibrRun = 1;
				//~ Quit = 1; 
				//~ RestartAll = 1;
				//~ AcqRun = 0;
				//~ WDcfg.StopOnTime = 0;
				//~ for(b=0; b<WDcfg.NumBrd; b++) 
					//~ for(ch=0; ch<WDcfg.NumPhyCh; ch++)
						//~ WDcfg.EnableZCcalibr[b][ch] = WDcfg.EnableInput[b][ch];
				//~ ZCcal_ResetZCTables(SysVars.ZCcorr_Ncnt);
			//~ } else {
				//~ ZCcal_SaveCorrectionTables(SysVars.ZCcalibrFileName);
				//~ RestartAll = 1;
			//~ }
			//~ break;
			//~ */

		//~ case '\t':
			//~ StatsMode ^= 1;
			//~ break;

		//~ case ' ':
			//~ printf("\n");
			//~ printf("s )  Start/Stop acquisition\n");
			//~ printf("S )  Restart acquisition (change run number)\n");
			//~ printf("C )  Open/Save config files\n");
			//~ printf("E )  Edit config file\n");
			//~ printf("I )  Enter run description\n");
			//~ printf("M )  Manual Controller\n");
			//~ printf("H )  HV settings\n");
			//~ printf("1-6) Change Probe settings for oscilloscope traces\n");
			//~ printf("t )  Force trigger (one shot)\n");
			//~ printf("T )  Force trigger (enable/disable continuous) \n");
			//~ printf("g )  Start TriDAQ GUI\n");
			//~ printf("R )  Reset Histograms and Statistics\n");
			//~ printf("h )  Force Histograms saving to files\n");
			//~ printf("l )  Enable/Disable List files saving\n");
			//~ printf("D )  Enable/Disable Raw Data file saving\n");
			//~ printf("b-c) Choose board/channel to plot\n");
			//~ printf("f )  Enable/Disable automatic refersh (plots and logs)\n");
			//~ printf("i )  Toggle statistics mode (integral/istantaneous)\n");
			//~ printf("o )  One shot refresh (plots and logs)\n");
			//~ printf("w )  Enable/Disable Waveform plot (restart acq.)\n");
			//~ printf("F )  Plot FFT. Press 'F' again to change FFT mean\n");
			//~ printf("e )  Energy Histogram plot\n");
			//~ printf("j )  Sample Histogram plot\n");
			//~ printf("m )  MCS Histogram plot\n");
			//~ printf("d )  DeltaT Histogram plot\n");
			//~ printf("p )  PSD Histogram plot\n");
			//~ printf("P )  Energy-PSD scatter 2D-plot\n");
			//~ printf("x )  Toggle between Channels and Units in the spectra\n");
			//~ printf("B )  Print Board Info\n");
			//~ printf("u )  Change Update Time for the statistics\n");
			//~ //printf("z )  Start a timing calibration run\n");
			//~ printf("q )  Quit\n");
			//~ printf("Tab )  Change Statistics\n\n");
			//~ printf("Space )  Print Menu\n\n");
			//~ printf("To select an option press the relevant key or Enter to continue\n\n");
			//~ c = getch();
			//~ goto menu;
			//~ break;

		//~ default: break;
		//~ }
	//~ }
	//~ SavePlotSettings();
}

/* ########################################################################### */
/* MAIN                                                                        */
/* ########################################################################### */
//~ int main(int argc, char *argv[])

extern "C" {
void measurement_spool(int *state) //, uint32_t **EHistoShort, uint32_t **EHistoLong)
{
	char InputDataFileName[500] = "";			// Input data file (off-line run) 
	char filename[500];							// String used to compose file names
	CAEN_DGTZ_ErrorCode ret= CAEN_DGTZ_Success;	// return code of the CAEN_DGTZ library functions
	GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH];  // Events extracted from the queue
	int WPch = 0, WPref = 0, WavePlotRdy = 0;   // Waveforms ready for plotting
	int NumEv[MAX_NBRD][MAX_NCH];               // Num of built events for each channel
	int TestConnection = 0;						// Test connection with the board, read info then quit
	// Other variables 
	int i, b, ch, ForceStatUpdate = 1, NoData=1;
	uint32_t AllocSize, TotAllocSize=0;
	int ExitSleep = WAIT_ON_EXIT; // waiting time before closing the console (only in case of error)
	int Wave_RefreshRate = 200;
	uint64_t CurrentTime, ElapsedTime, PrevStatTime, PrevLogTime, PrevPlotTime, PrevKeybTime, PrevAustosave;
	uint64_t CfgUpdateTime=0;
	uint64_t PrevChTimeStamp[MAX_NBRD][MAX_NCH];
	FILE *f_ini;  // config file

	/* *************************************************************************************** */
	/* Initialize system                                                                       */
	/* *************************************************************************************** */
	//~ InitConsole();  // init the console window (I/O terminal)

	// open message log file
	sprintf(filename, "%sMsgLog.txt", WORKING_DIR);
	MsgLog = fopen(filename, "w");
	if (MsgLog == NULL) {
		printf("ERROR: Can't open message log file %s. Quitting...\n", filename);
		goto QuitProgram;
	}
    msg_printf(MsgLog, "*************************************************\n");
    msg_printf(MsgLog, "CAEN SpA - digiTES Rev %s\n", digiTES_Revision);
    msg_printf(MsgLog, "*************************************************\n");


	// Load system variables
	LoadSysVars();

	/* *************************************************************************************** */
	/* Get command line options                                                                */
	/* *************************************************************************************** */
	sprintf(ConfigFileName, "%sdigiTES_Config.txt", CONFIG_FILE_PATH);
	//~ for (i=1; i<argc; i++) {
		//~ if (argv[i][0] == '-') {
			//~ if (strcmp(argv[i]+1, "t")==0) TestConnection = 1;
			//~ if (strcmp(argv[i]+1, "i")==0) SysVars.ImmediateStart = 1;
			//~ if (strcmp(argv[i]+1, "l")==0) SysVars.AutoReloadPlotsettings = 1;
			//~ if (strcmp(argv[i]+1, "zcc")==0) CalibrRun = 1;
			//~ if (argv[i][1] == 'r') sscanf(&argv[i][2], "%d", &OvwrRunNumber);
			//~ if (argv[i][1] == 'f') strcpy(InputDataFileName, &argv[i][2]);
			//~ if (argv[i][1] == 'h') {
				//~ printf("Syntax: digiTES [options] [ConfigFileName]\n");
				//~ printf(" ConfigFileName = configuration file (default = digiTES_Config.txt)\n");
				//~ printf(" -i : Immediate Start Acquisition (don't ask to press 's')\n");
				//~ printf(" -l : reload previous settings for plots (channel, traces, etc...)\n");
				//~ printf(" -rN : set run number = N\n");
				//~ printf(" -fInputDataFile : Off-line run taking data from InputDataFile\n");
				//~ printf(" -zcc : Zero crossing calibration run\n");
				//~ printf(" -t : Test Board Connection, then quit\n");
				//~ goto QuitProgram;
			//~ }
		//~ } else {
			//~ strcpy(ConfigFileName, argv[i]);
		//~ }
	//~ }

	Restart:
	/* *************************************************************************************** */
	/* Open and parse configuration file                                                       */
	/* *************************************************************************************** */
	msg_printf(MsgLog, "INFO: Opening configuration file %s\n", ConfigFileName);
	f_ini = fopen(ConfigFileName, "r");
	if (f_ini == NULL ) {
		msg_printf(MsgLog, "ERROR: Can't open configuration file %s\n", ConfigFileName);
		Failure = 1;
		goto QuitProgram;
	}
	ParseConfigFile(f_ini, &WDcfg);
	fclose(f_ini);
	msg_printf(MsgLog, "INFO: Configuration file parsed\n", ConfigFileName);
	if (!RestartAll && WDcfg.WaveformEnabled) {
		WavePlotType = WPLOT_WAVEFORMS;
	} else if (RestartAll && ForceWaveMode && (WavePlotType != WPLOT_DISABLED)) {
		WDcfg.AcquisitionMode = ACQ_MODE_MIXED;  // force waveform mode
		WDcfg.WaveformEnabled = 1; 
		ForceWaveMode = 0;
	}

	// get last update time of the config file
	GetFileUpdateTime(ConfigFileName, &CfgUpdateTime);

	// overwrite some WDcfg settings with command line options
	if (OvwrRunNumber >= 0) 
		WDcfg.RunNumber = OvwrRunNumber;
	if (strlen(InputDataFileName) > 0) {
		strcpy(WDcfg.InputDataFileName, InputDataFileName);
		WDcfg.AcquisitionMode = ACQ_MODE_OFFLINE;
	}
	if (CalibrRun) WDcfg.CalibrationRun = 1;

	msg_printf(MsgLog, "INFO: Run Number = %d\n", WDcfg.RunNumber);

	if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
		char cstr[500];
		/* *************************************************************************************** */
		/* Open the digitizers and read board information                                          */
		/* *************************************************************************************** */
		sprintf(filename, "%sConnectionLog.txt", WORKING_DIR);
		cntlog = fopen(filename, "w");
		if (cntlog == NULL) {
			msg_printf(MsgLog, "ERROR: Can't open connection log file %s. Quitting...\n", filename);
			Failure = 1;
			goto QuitProgram;
		}
		msg_printf(MsgLog, "INFO: On-line run with %d board(s)\n", WDcfg.NumBrd);
		for(b=0; b<WDcfg.NumBrd; b++) {
			char ct[2][10] = {"USB", "OPT_LINK"};

			if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE)
				msg_printf(MsgLog, "\nINFO: Opening Virtual Board n. %d connected to board %d\n", b, WDcfg.LinkNum[b]);
 			else if (!RestartAll) {
				fprintf(cntlog, "BOARD%d, ", b);
				msg_printf(MsgLog, "\nINFO: Opening digitizer n. %d through %s %d ", b, ct[WDcfg.LinkType[b]], WDcfg.LinkNum[b]);
				if (WDcfg.LinkType[b] != 0) msg_printf(MsgLog, "%d ",  WDcfg.ConetNode[b]);
				if (WDcfg.BaseAddress[b] != 0) msg_printf(MsgLog, "BA=%08X ", WDcfg.BaseAddress[b]);
				ret = CAEN_DGTZ_OpenDigitizer((CAEN_DGTZ_ConnectionType)WDcfg.LinkType[b], WDcfg.LinkNum[b], WDcfg.ConetNode[b], WDcfg.BaseAddress[b], &handle[b]);
				if (ret) {
					msg_printf(MsgLog, ": Failed!!! Error Code = %d\n", ret);
					fprintf(cntlog, "Failed, -, -, -, -, -, -, -\n");
					if (TestConnection) continue;
					msg_printf(MsgLog, "ERROR: Can't open digitizer n. %d\n", b);
					Failure = 1;
					goto QuitProgram;
				} else {
					msg_printf(MsgLog, ": Success\n");
				}
				//RestartAll = 0;
				fprintf(cntlog, "Success, %s\n", cstr);
			}
			// Read Board Info
			if (ReadBoardInfo(b, cstr) < 0) {
				Failure = 1;
				goto QuitProgram;
			}
		}
	} else if (WDcfg.AcquisitionMode == ACQ_MODE_OFFLINE) {  // Off-line run (data coming from file)
		if (OpenInputDataFile() < 0) {
			Failure = 1;
			goto QuitProgram;
		} 
		msg_printf(MsgLog, "INFO: Off-line run with input data file %s\n", WDcfg.InputDataFileName);
	} else {
		msg_printf(MsgLog, "INFO: Running in emulation mode\n");
	}

	if (WDcfg.BuildMode == EVBUILD_CLOVER)
		WDcfg.NumAcqCh = WDcfg.NumPhyCh + WDcfg.NumPhyCh/WDcfg.CloverNch;
	else
		WDcfg.NumAcqCh = WDcfg.NumPhyCh;

	// Disable channels that don't exist
	for(b=0; b<WDcfg.NumBrd; b++)
		for(i=WDcfg.NumAcqCh; i<MAX_NCH; i++)
			WDcfg.EnableInput[b][i] = 0;

	if (cntlog != NULL)
		fclose(cntlog);
	if (TestConnection) {  // Just tested the connection to the boards; the program quits here
		ExitSleep = 0;
		goto QuitProgram;
	}

	// Set channel to plot as the first enabled channel in board 0
	BrdToPlot = 0;
	for(i=0; i<MAX_NCH; i++) {
		if (WDcfg.EnableInput[0][i]) {
			ChToPlot = i;
			break;
		}
	}

	SetTraceNames();

	/* *************************************************************************************** */
	/* Program the digitizer (see function ProgramDigitizer)                                   */
	/* *************************************************************************************** */
	if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
		for(b=0; b<WDcfg.NumBrd; b++) {
			if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE) continue;
			ret = (CAEN_DGTZ_ErrorCode) ProgramDigitizer(b, SkipCalibration);
			if (ret) {
				msg_printf(MsgLog, "ERROR: Failed to program the digitizer\n");
				Failure = 1;
				goto QuitProgram;
			}
			SetVirtualProbes(b);
		}
	}
	SkipCalibration = 1;

	/* *************************************************************************************** */
	/* Allocate memory buffers and init readout                                                */
	/* *************************************************************************************** */
	// Allocate and initialize Histograms
	TotAllocSize = 0;
	if (CreateHistograms(&AllocSize) < 0) goto QuitProgram;
	TotAllocSize += AllocSize;
	if (InitReadout(&AllocSize) < 0) goto QuitProgram;
	TotAllocSize += AllocSize;
	if (WDcfg.AcquisitionMode != ACQ_MODE_LIST) {
		if (InitFFT(&AllocSize) < 0) goto QuitProgram;
		TotAllocSize += AllocSize;
	}
	msg_printf(MsgLog, "INFO: Allocated %.2f MB for readout data, histograms and counters\n", (float)TotAllocSize/(1024*1024));

	/* *************************************************************************************** */
	/* Open Output data files                                                                  */
	/* *************************************************************************************** */
	if (OpenOutputDataFiles() < 0) {
		ExitSleep = 0;
		goto QuitProgram;
	}
	msg_printf(MsgLog, "INFO: Output data files open\n");

	/* *************************************************************************************** */
	/* Open the plotters                                                                       */
	/* *************************************************************************************** */
	//~ if (OpenPlotter() < 0) {
		//~ Failure = 1;
		//~ goto QuitProgram;
	//~ }

	/* *************************************************************************************** */
	/* Readout Loop                                                                            */
	/* *************************************************************************************** */
	//~ if (SysVars.AutoReloadPlotsettings)	LoadPlotSettings();
	if ((WDcfg.RecordLength * WDcfg.Tsampl) > 10000) Wave_RefreshRate = 500;

	memset(NumEv, 0, MAX_NBRD * MAX_NCH * sizeof(**NumEv));
	memset(StopCh, 0, MAX_NBRD * MAX_NCH * sizeof(**StopCh));
	memset(PrevChTimeStamp, 0, sizeof(uint64_t) * MAX_NCH * MAX_NBRD);
	memset(&Stats, 0, sizeof(Stats));
	ResetHistograms();
	///////////////////////////////////////////////////
	// NOTE: This is an interesting implementation here.
	// I'm not sure if this actually matters.
	///////////////////////////////////////////////////
	//~ if (kbhit()) SysVars.ImmediateStart = 0;
	//~ while (!AcqRun) {
		//~ int c;
		//~ static int ms=1;
		//~ Sleep(10);
		//~ if (SysVars.ImmediateStart || RestartAll) {
			//~ c = 's';
		//~ } else {
			//~ if (ms) printf("press 's' to start the acquisition, 'q' to quit\n");
			//~ c = getch();
			//~ ms=0;
		//~ }
		//~ if (c == 'M') ManualController();
		//~ if (c == 'H') HVsettings();
		//~ if (c == 'q') {
			//~ ExitSleep = 0;
			//~ goto QuitProgram;
		//~ }
		//~ if (c == 's') {
			//~ StartAcquisition();
			//~ AcqRun = 1;
		//~ }
	//~ }
	RestartAll = 0;
	Stats.StartTime = get_time();
	PrevLogTime  = Stats.StartTime;
	PrevStatTime = Stats.StartTime;
	PrevPlotTime = Stats.StartTime;
	PrevKeybTime = Stats.StartTime;
	PrevAustosave = Stats.StartTime;
	//~ printf("Press [Space] for help\n");
	//~ if (!DoRefresh) printf("Plots and Logs are OFF; press 'f' to enable or 'o' for a singles\n");

	msg_printf(MsgLog, "INFO: Starting Acquisition (Run %d)\n", WDcfg.RunNumber);
	while(!Quit && !RestartAll) {
		CurrentTime = get_time();
		if (Failure) goto QuitProgram;

		// ----------------------------------------------------------------------------------- 
		// Check from commands from the keyboard 
		// ----------------------------------------------------------------------------------- 
		///////////////////////////////////////////////////
		// NOTE: This is where the money is. 
		// This is where state changes occur.
		///////////////////////////////////////////////////
		msg_printf(MsgLog, "Before checkstate\n");
		if ((CurrentTime - PrevKeybTime) > 200) {
			CheckState(state, MsgLog);
			PrevKeybTime = CurrentTime;
		}
		msg_printf(MsgLog, "After checkstate\n");

		// ----------------------------------------------------------------------------------- 
		// Send a software trigger to each board 
		// ----------------------------------------------------------------------------------- 
		if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode) && (ContinuousTrigger || SingleTrigger)) {
			if ((WDcfg.TrginMode == TRGIN_MODE_VETO) || (WDcfg.TrginMode == TRGIN_MODE_GATE)) {
				//~ printf("Can't send SW triggers when TrginMode = VETO or GATE\n");
				ContinuousTrigger = 0;
			} else {
				for(b=0; b<WDcfg.NumBrd; b++)
					if (WDcfg.LinkType[b] != VIRTUAL_BOARD_TYPE)
						CAEN_DGTZ_SendSWtrigger(handle[b]);
				//~ if (SingleTrigger)				
					//~ printf("Single Software Trigger issued\n");
			}
			SingleTrigger = 0;
		}


		// NOTE: Remove the  print out screen stuff every second.
		// ----------------------------------------------------------------------------------- 
		// Update statistics and print them onto the screen (once every second)
		// ----------------------------------------------------------------------------------- 
		ElapsedTime = CurrentTime - PrevLogTime; // in ms
		//~ if ((ElapsedTime > 1000) && (DoRefresh || DoRefreshSingle)) {
			//~ int stoprun = 1;
			//~ uint64_t cfgtime;
			//~ if (ForceStatUpdate || ((CurrentTime - PrevStatTime) > WDcfg.StatUpdateTime)) {
				//~ UpdateStatistics(CurrentTime);
				//~ PrevStatTime = CurrentTime;
				//~ ForceStatUpdate = 0;
			//~ }
			//~ ClearScreen();
			//~ printf("Press [Space] for help\n");
			//~ if (WDcfg.JobStopRun > 0) printf("RUN n. %d - Job %d of %d\n", WDcfg.RunNumber, WDcfg.RunNumber - WDcfg.JobStartRun + 1, WDcfg.JobStopRun - WDcfg.JobStartRun + 1);
			//~ else printf("RUN n. %d : %s\n", WDcfg.RunNumber, WDcfg.RunDescription);
			//~ if (IntegratedRates) printf("Statistics Mode: Integral\n");
			//~ else printf("Statistics Mode: Istantaneous\n");
			//~ if (AcqRun && ((WDcfg.StopOnTime == 0) || (Stats.AcqRealTime < WDcfg.StopOnTime))) {
				//~ if (Stats.RealTimeSource == REALTIME_FROM_BOARDS)	printf("RealTime (from boards) = %.2f s", Stats.AcqRealTime/1000);
				//~ else												printf("RealTime (from computer) = %.2f s", Stats.AcqRealTime/1000);
				//~ if (WDcfg.StopOnTime > 0) printf(" - Stop Time = %.2f s (%d%%)\n", (float)WDcfg.StopOnTime/1000, (int)(100*Stats.AcqRealTime/WDcfg.StopOnTime));
				//~ else printf("\n");
			//~ } else {
				//~ printf("Stopped at RealTime = %.2f s\n", Stats.AcqStopTime/1000);
			//~ }
			//~ printf("Readout Rate = %.2f MB/s\n", Stats.RxByte_rate);
			//~ printf("Enabled Output Files: ");
			//~ if (WDcfg.SaveHistograms) {
				//~ printf("Histograms (");
				//~ if (WDcfg.SaveHistograms & 1) printf("E");
				//~ if (WDcfg.SaveHistograms & 2) printf("T");
				//~ if (WDcfg.SaveHistograms & 4) printf("P");
				//~ printf(") ");
			//~ }
			//~ if (WDcfg.SaveRawData)    printf("Raw ");
			//~ if (WDcfg.SaveLists)      printf("Lists ");
			//~ if (WDcfg.SaveWaveforms)  printf("Waveforms ");
			//~ if (WDcfg.SaveRunInfo)    printf("Info ");
			//~ printf("\n");

			//~ for(b=0; b<WDcfg.NumBrd; b++) {
				//~ char str[1000];
				//~ BoardLogString(b, StatsMode, str);
				//~ printf("\n%s\n", str);
				//~ printf("-----------------------------------------------------------------------\n");
				//~ for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
					//~ ChannelLogString(b, ch, StatsMode, str);
					//~ printf("%s\n", str);
				//~ }
			//~ }
			//~ printf("\n\n");
			//~ PrevLogTime = CurrentTime;

			//~ if (HistoPlotType != HPLOT_DISABLED) PlotSelectedHisto(HistoPlotType, Xunits);

			//~ // check for config file modification
			//~ if (SysVars.AutoRestartOnCfgChange) {
				//~ GetFileUpdateTime(ConfigFileName, &cfgtime);
				//~ if (cfgtime > CfgUpdateTime) {
					//~ CfgUpdateTime = cfgtime;
					//~ printf("Config file Changed; restarting...\n\n");
					//~ RestartAll = 1;
					//~ goto QuitProgram;
				//~ }
			//~ }

		//~ }

		// if not running, close the while loop here (the Sleep prevents heavy CPU usage)
		if (!AcqRun) {
			Sleep(10);
			continue;
		}
		
		////////////////////////////////////////////////////////
		// NOTE: This is unnecessary. 
		// This seems like for file output stuff.
		////////////////////////////////////////////////////////
		// AutoSave histograms
		//~ if ((SysVars.HistoAutoSave > 0) && ((CurrentTime - PrevAustosave) > (SysVars.HistoAutoSave * 1000))) {
			//~ SaveAllHistograms();
			//~ PrevAustosave = CurrentTime;
		//~ }
		
		////////////////////////////////////////////////////////
		// NOTE: 
		// I will have to implement a similar functionality.
		// Probably reset the histogram array every second, while 
		// relying on Python to readout the pointer arrays every second.
		////////////////////////////////////////////////////////
		if (ElapsedTime > 1000) 
		{
			ResetHistograms();
			PrevLogTime = CurrentTime;
		}

		// ----------------------------------------------------------------------------------- 
		// Check stopping criteria 
		// ----------------------------------------------------------------------------------- 
		if (Stopping && (NoData || ACQ_MODE_UNPLUGGED(WDcfg.AcquisitionMode))) { // Stop command from the keyboard
			SaveAllHistograms();
			Stopping = 0;
			AcqRun = 0;
			//~ printf("Acquisition Stopped\n");
		}
		for(b=0; b<WDcfg.NumBrd; b++) {
			for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
				if (((WDcfg.StopOnTotalEvents > 0)  && (Stats.EvRead_cnt[b][ch] >= (uint32_t)WDcfg.StopOnTotalEvents))  ||
					((WDcfg.StopOnTimeEvents > 0)   && (Histos.TH[b][ch].H_cnt  >= (uint32_t)WDcfg.StopOnTimeEvents))   ||
					((WDcfg.StopOnEnergyEvents > 0) && (Histos.EH[b][ch].H_cnt  >= (uint32_t)WDcfg.StopOnEnergyEvents)) ||
					((WDcfg.StopOnTime > 0)         && (Stats.LatestProcTstamp[b][ch] >= ((uint64_t)WDcfg.StopOnTime * 1e6))) || 
					((WDcfg.StopOnTime > 0)         && ((Stats.AcqRealTime > (float)WDcfg.StopOnTime) && (Stats.EvRead_rate[b][ch]==0))) ||
					((WDcfg.StopOnLiveTime > 0)     && (Stats.AcqRealTime * (1 - Stats.DeadTime[b][ch]) >= (WDcfg.StopOnLiveTime/1000)))) {
					int bb, cc, AllStopped = 1;
					StopCh[b][ch] = 1;
					// check if all channels are stopped; if so, stop the acquisition
					for(bb=0; bb<WDcfg.NumBrd; bb++)
						for(cc=0; cc<WDcfg.NumAcqCh; cc++) 
							if (WDcfg.EnableInput[bb][cc] && !StopCh[bb][cc])
								AllStopped = 0;
					if (AllStopped) {
						StopAcquisition();
						SaveAllHistograms();
						AcqRun = 0;
						if (Stats.AcqStopTime < (float)WDcfg.StopOnTime) Stats.AcqStopTime = (float)WDcfg.StopOnTime;
						if (WDcfg.JobStopRun > 0) Quit = 2;
					}
				}
			}
		}
		
		///////////////////////////////////////////////////
		// NOTE: Cut out all the analyzing of events.
		///////////////////////////////////////////////////

		//~ // ----------------------------------------------------------------------------------- 
		//~ // Analyze Events data 
		//~ // ----------------------------------------------------------------------------------- 
		//~ if (GetBuiltEvents(evnt, NumEv) > 0) {  // Get built events
			//~ NoData = 0;

			//~ if ((WDcfg.SaveLists & SAVELIST_BUILTEVENTS) && (WDcfg.BuildMode != EVBUILD_NONE)) 
				//~ SaveBuiltEventsList(evnt, NumEv);

			//~ for(b=0; b<WDcfg.NumBrd; b++) {
				//~ for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
					//~ int BrdRef = WDcfg.TOFstartBoard;
					//~ int ChRef = WDcfg.TOFstartChannel;
					//~ int BadEvent = 0;
					//~ int Ebin;
					//~ uint16_t psdbin, bx, by;

					//~ if (NumEv[b][ch] == 0)  // no data from this channel
						//~ continue;
					//~ if (StopCh[b][ch]) { // single channel stopped: data are still coming, but they are discarded
						//~ Stats.EvRead_cnt[b][ch]--;  
						//~ FreeWaveform(evnt[b][ch].Waveforms);
						//~ continue;
					//~ }

					//~ // Get the newest time stamp (used to calculate the real acquisition time)
					//~ if (evnt[b][ch].TimeStamp > Stats.LatestProcTstampAll)
						//~ Stats.LatestProcTstampAll = evnt[b][ch].TimeStamp;
					//~ Stats.AcqStopTime = (float)(Stats.LatestProcTstampAll/1e6);
					//~ Stats.LatestProcTstamp[b][ch] = evnt[b][ch].TimeStamp;

					//~ // ------------------------------------------------------------------------------
					//~ // Energy & PSD Spectra 
					//~ // ------------------------------------------------------------------------------
					//~ if ((evnt[b][ch].Flags & EVFLAGS_PILEUP) && !((WDcfg.DppType == DPP_PSD_730) && (WDcfg.PileUpMode[b][ch] == 0))) { // Pile-up
						//~ BadEvent = 1;
					//~ } else if (evnt[b][ch].Flags & EVFLAGS_SATUR) { // input saturation (Vin > dynamic range)
						//~ BadEvent = 1;
					//~ } else if (evnt[b][ch].Flags & EVFLAGS_E_OVR) {  // charge saturation
						//~ BadEvent = 1;
						//~ Ebin = 0xFFFF;
						//~ Histo1D_AddCount(&Histos.EH[b][ch], Ebin);  // increment over_range counter
					//~ } else if (evnt[b][ch].Flags & EVFLAGS_E_UNR) {  // negative charge 
						//~ BadEvent = 1;
						//~ Ebin = -1;
						//~ Histo1D_AddCount(&Histos.EH[b][ch], Ebin); // increment under_range counter
					//~ } else {
						//~ Ebin = (int)evnt[b][ch].Energy;
						//~ psdbin = (int)(evnt[b][ch].psd*1024);
						//~ by, bx = Ebin * HISTO2D_NBINX / WDcfg.EHnbin;
						//~ if (WDcfg.ScatterPlotMode == SCATTER_PSD_DIAGONAL)
							//~ by = (int)((evnt[b][ch].Energy - evnt[b][ch].psd*evnt[b][ch].Energy) * HISTO2D_NBINY / WDcfg.EHnbin);
						//~ else 
							//~ by = (int)(evnt[b][ch].psd * HISTO2D_NBINY);
						//~ Histo1D_AddCount(&Histos.PSDH[b][ch], psdbin);
						//~ if ((WDcfg.ScatterPlotMode == SCATTER_PSD_DIAGONAL) || (WDcfg.ScatterPlotMode == SCATTER_PSD_HORIZONTAL))
							//~ Histo2D_AddCount(&Histos.PSDvsE[b][ch], bx, by);
						//~ Histo1D_AddCount(&Histos.EH[b][ch], Ebin);
					//~ }

					//~ // ------------------------------------------------------------------------------
					//~ // Energy vs deltaE (alternative to PSD)
					//~ // ------------------------------------------------------------------------------
					//~ if (!BadEvent && (WDcfg.ScatterPlotMode == SCATTER_E_VS_DELTAE)) {
						//~ if (((ch & 1) == 0) && (NumEv[b][ch+1])) { // assuming deltaE is in the same channel pair of E (ch0=E, ch1=dE, etc...)
							//~ by = (int)evnt[b][ch+1].Energy * HISTO2D_NBINX / WDcfg.EHnbin;
							//~ Histo2D_AddCount(&Histos.PSDvsE[b][ch], bx, by);
							//~ Histo2D_AddCount(&Histos.PSDvsE[b][ch+1], bx, by);
						//~ }
					//~ }

					//~ // ------------------------------------------------------------------------------
					//~ // Sample Histogram (NOTE: the sample histogram grows only when active on the plot)
					//~ // ------------------------------------------------------------------------------
					//~ if ((evnt[b][ch].Waveforms != NULL) && (HistoPlotType == HPLOT_SAMPLES)) {
						//~ for (i=0; i<evnt[b][ch].Waveforms->Ns; i++)
							//~ Histo1D_AddCount(&Histos.SH[b][ch], evnt[b][ch].Waveforms->AnalogTrace[0][i]);
					//~ }


					//~ // ------------------------------------------------------------------------------
					//~ // Time Spectrum 
					//~ // ------------------------------------------------------------------------------
					//~ double time;
					//~ uint32_t Tbin;
					//~ if (WDcfg.TspectrumMode == TAC_SPECTRUM_INTERVALS)  
						//~ time = (double)(evnt[b][ch].TimeStamp - PrevChTimeStamp[b][ch]);  // delta T between pulses on the same channel (in ns)
					//~ else
						//~ time = (double)evnt[b][ch].TimeStamp - (double)evnt[BrdRef][ChRef].TimeStamp + ((double)evnt[b][ch].FineTimeStamp - (double)evnt[BrdRef][ChRef].FineTimeStamp)/1000;  // delta T from Ref Channel (in ns)
						
					//~ PrevChTimeStamp[b][ch] = evnt[b][ch].TimeStamp;
					//~ Tbin = (uint32_t)((time - WDcfg.THmin[b][ch]) * WDcfg.THnbin / (WDcfg.THmax[b][ch] - WDcfg.THmin[b][ch]));
					//~ Histo1D_AddCount(&Histos.TH[b][ch], Tbin);

					//~ // ------------------------------------------------------------------------------
					//~ // Multi Channel Scaler 
					//~ // ------------------------------------------------------------------------------
					//~ Tbin = (uint32_t)(evnt[b][ch].TimeStamp / ((uint64_t)WDcfg.DwellTime*1000));
					//~ Histo1D_AddCount(&Histos.MCSH[b][ch], Tbin);

					//~ // ------------------------------------------------------------------------------
					//~ // Save and Plot
					//~ // ------------------------------------------------------------------------------
					//~ // save event into the enabled output files
					//~ if ((WDcfg.SaveLists & SAVELIST_INDIVIDUAL) && !(BadEvent && WDcfg.NoBadEventsInListFiles))
						//~ SaveList(b, ch, evnt[b][ch]);
					//~ if (WDcfg.SaveLists & SAVELIST_MERGED)
						//~ SaveMergedList(b, ch, evnt[b][ch]);
					//~ if (WDcfg.SaveWaveforms && (evnt[b][ch].Waveforms != NULL)) 
						//~ SaveWaveform(b, ch, evnt[b][ch]);

					//~ // Accumulate FFT (average)
					//~ if (WavePlotType == WPLOT_FFT) AccumulateFFT(*evnt[b][ch].Waveforms);

					//~ // Waveform Plotting 
					//~ if ((b==BrdToPlot) && (ch==ChToPlot) && (WavePlotType != WPLOT_DISABLED) && ((CurrentTime-PrevPlotTime) > Wave_RefreshRate) && (DoRefresh || DoRefreshSingle) && (evnt[b][ch].Waveforms != NULL)) {
						//~ char title[500];
						//~ if (WavePlotType == WPLOT_WAVEFORMS) { // Waveform
							//~ sprintf(title, "WAVEFORMS : Board %d - Channel %d (Energy = %7d)", b, ch, evnt[b][ch].Energy);
							//~ PlotWaveforms(*evnt[b][ch].Waveforms, title);
							//~ PrevPlotTime = CurrentTime;
						//~ } else if (WavePlotType == WPLOT_FFT) { // fft
							//~ int NsFFT, NmeanFFT;
							//~ double *fft;
							//~ if (GetFFT(&NmeanFFT, &NsFFT, &fft)) {
								//~ sprintf(title, "FFT : Board %d - Channel %d (Mean = %d)", b, ch, NmeanFFT);
								//~ PlotFFT(fft, NsFFT, title);
								//~ PrevPlotTime = CurrentTime;
							//~ }
						//~ }
					//~ }

					//~ // free waveform memory
					//~ FreeWaveform(evnt[b][ch].Waveforms);
				//~ }
			//~ }
			//~ DoRefreshSingle = 0;
		//~ } else if (Stopping) {
			//~ NoData = 1;
		//~ }
	} // End of readout loop

	UpdateStatistics(CurrentTime);
	if (WDcfg.SaveRunInfo) SaveRunInfo();

	SaveAllHistograms();
	if (WDcfg.CalibrationRun) {
		ZCcal_SaveCorrectionTables(SysVars.ZCcalibrFileName);
		msg_printf(MsgLog, "INFO: Saving ZCcalibration file %s\n", SysVars.ZCcalibrFileName);
	}
	ExitSleep = 0;

QuitProgram:
	/* stop the acquisition, close the device and free the buffers */
	if (AcqRun) {
		StopAcquisition();
		AcqRun = 0;
	}
	if (!Failure) msg_printf(MsgLog, "INFO: Closing Run %d.\n\n", WDcfg.RunNumber);
	DestroyHistograms();
	CloseReadout();
	CloseOutputDataFiles();
	CloseFFT();
	if ((WDcfg.JobStopRun > WDcfg.JobStartRun) && (WDcfg.RunNumber < WDcfg.JobStopRun)) {
		char c = 'n';
		if (Quit == 1) {  // received quit command
			printf("Do you want to quit the jobs [y/n]?\n");
			fflush(stdin);
			c = getch();
		}
		if (tolower(c) != 'y') {
			WDcfg.RunNumber++;
			RestartAll = 1;
			Sleep(WDcfg.JobSleep);
		}
	}
	Quit = 0;
	if (Failure) {
		int c;
		printf("\nPress 'e' to edit Config File, any other key to quit\n");
		c = getch();
		if (tolower(c) == 'e') {
			// system(EDIT_CFG_FILE);
			printf("Press 'q' to quit, any other key to restart\n");
			c = getch();
			if (tolower(c) != 'q') {
				Failure = 0;
				RestartAll = 1;
			} else {
				ExitSleep = 0;
			}
		} else {
			ExitSleep = 0;
		}
	}
	if (RestartAll && !Failure) {
		//~ ClearScreen();
		goto Restart;
	}
	//~ ClosePlotter();
	for(b=0; b<WDcfg.NumBrd; b++)
		if (WDcfg.LinkType[b] != VIRTUAL_BOARD_TYPE)
			CAEN_DGTZ_CloseDigitizer(handle[b]);
	if (MsgLog != NULL)
		fclose(MsgLog);
	if (ExitSleep > 0) {
		printf("Press a key to quit\n");
		for (i=0; i<(ExitSleep/10); i++) {
			Sleep(10);
			//~ if (kbhit())
				//~ break;
		}
	}
	return;
}
}


