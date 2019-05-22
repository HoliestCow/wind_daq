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


#include <CAENDigitizer.h>
#include "digiTES.h"
#include "ParamParser.h"
#include "Console.h"


int load_default=1;
int ch=-1, brd=-1;
int jobrun = -1;
int ValidParameterName = 0;


// --------------------------------------------------------------------------------------------------------- 
// Description: compare two strings
// Inputs:		str1, str2: strings to compare
// Outputs:		-
// Return:		1=equal, 0=not equal
// --------------------------------------------------------------------------------------------------------- 
int streq(char *str1, char *str2)
{
	if (strcmp(str1, str2) == 0) {
		ValidParameterName = 1;
		return 1;
	} else {
		return 0;
	}
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read an integer (decimal) from the conig file
// Inputs:		f_ini: config file
// Outputs:		-
// Return:		integer value read from the file
// --------------------------------------------------------------------------------------------------------- 
int GetInt(FILE *f_ini)
{
	int ret;
	fscanf(f_ini, "%d", &ret);
	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read a value from the conig file followed by an optional time unit (ps, ns, us, ms, s)
//              and convert it in a time expressed in ns as a float 
// Inputs:		f_ini: config file
//				tu: time unit of the returned time value
// Outputs:		-
// Return:		time value (in ns) read from the file
// --------------------------------------------------------------------------------------------------------- 
float GetTime(FILE *f_ini, char *tu)
{
	double timev, ns;
	long fp;
	char str[100];

	fscanf(f_ini, "%lf", &timev);
	// try to read the unit from the config file (string)
	fp = ftell(f_ini);  // save current pointer before "str"
	fscanf(f_ini, "%s", str);  // read string "str"
	if (streq(str, "ps"))		ns = timev*1e-3;
	else if (streq(str, "ns"))	ns = timev;
	else if (streq(str, "us"))	ns = timev*1e3;
	else if (streq(str, "ms"))	ns = timev*1e6;
	else if (streq(str, "s") )	ns = timev*1e9;
	else if (streq(str, "m") )	ns = timev*60e9;
	else if (streq(str, "h") )	ns = timev*3600e9;
	else if (streq(str, "d") )	ns = timev*24*(3600e9);
	else {
		fseek(f_ini, fp, SEEK_SET); // move pointer back to beginning of "str" and use it again for next parsing
		return (float)timev;  // no time unit specified in the config file; assuming equal to the requested one
	}

	if (streq(tu, "ps"))		return (float)(ns*1e3);
	else if (streq(tu, "ns"))	return (float)(ns);
	else if (streq(tu, "us"))	return (float)(ns/1e3);
	else if (streq(tu, "ms"))	return (float)(ns/1e6);
	else if (streq(tu, "s") )	return (float)(ns/1e9);
	else if (streq(tu, "m") )	return (float)(ns/60e9);
	else if (streq(tu, "h") )	return (float)(ns/3600e9);
	else if (streq(tu, "d") )	return (float)(ns/(24*3600e9));
	else return (float)timev;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read an integer (hexadecimal) from the conig file
// Inputs:		f_ini: config file
// Outputs:		-
// Return:		integer value read from the file
// --------------------------------------------------------------------------------------------------------- 
int GetHex(FILE *f_ini)
{ 
	int ret;
	fscanf(f_ini, "%x", &ret);
	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read a float from the conig file
// Inputs:		f_ini: config file
// Outputs:		-
// Return:		float value read from the file
// --------------------------------------------------------------------------------------------------------- 
float GetFloat(FILE *f_ini)
{
	float ret;
	char str[1000];
	int i;
	fgets(str, 1000, f_ini);
	// replaces ',' with '.' (decimal separator)
	for(i=0; i<(int)strlen(str); i++)
		if (str[i] == ',') str[i] = '.';
	sscanf(str, "%f", &ret);
	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read the number of bins (channels) of an histogram as a number or text
// Inputs:		f_ini: config file
// Outputs:		-
// Return:		integer value read from the file
// --------------------------------------------------------------------------------------------------------- 
int GetHnbin(FILE *f_ini)
{
	char str[100];
	int i, ret;
	fscanf(f_ini, "%s", str);
	for (i=0; i<(int)strlen(str); i++)
		str[i] = toupper(str[i]);
	if      (streq(str, "1K"))  ret = 1024;
	else if (streq(str, "2K"))  ret = 2048;
	else if (streq(str, "4K"))  ret = 4096;
	else if (streq(str, "8K"))  ret = 8192;
	else if (streq(str, "16K")) ret = 16384;
	else if (streq(str, "32K")) ret = 32768;
	else {
		sscanf(str, "%d", &ret);
		// convert to a power of 2
		for(i=8; i<=16; i++) {
			if (ret <= (1<<i)) {
				ret = (1 << i);
				break;
			}
		}
	}
	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Set a parameter (individual channel or broadcast) to a given integer value 
// Inputs:		param: array of parameters 
//				val: value to set
// Outputs:		-
// Return:		-
// --------------------------------------------------------------------------------------------------------- 
void SetChannelParam(int param[MAX_NBRD][MAX_NCH], int val)
{
	int i, b;
    if (ch == -1)
        for(b=0; b<MAX_NBRD; b++)
			for(i=0; i<MAX_NCH; i++)
				param[b][i] = val;
    else
        param[brd][ch] = val;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Set a parameter (individual channel or broadcast) to a given float value 
// Inputs:		param: array of parameters 
//				val: value to set
// Outputs:		-
// Return:		-
// --------------------------------------------------------------------------------------------------------- 
void SetChannelfParam(float param[MAX_NBRD][MAX_NCH], float val)
{
	int i, b;
    if (ch == -1)
        for(b=0; b<MAX_NBRD; b++)
			for(i=0; i<MAX_NCH; i++)
				param[b][i] = val;
    else
        param[brd][ch] = val;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Set a parameter (individual channel or broadcast) to a given float value 
// Inputs:		param: array of parameters 
//				val: value to set
// Outputs:		-
// Return:		-
// --------------------------------------------------------------------------------------------------------- 
void SetChannelParamFloat(float param[MAX_NBRD][MAX_NCH], float val)
{
	int i, b;
    if (ch == -1)
        for(b=0; b<MAX_NBRD; b++)
			for(i=0; i<MAX_NCH; i++)
				param[b][i] = val;
    else if (brd == -1)
		param[0][ch] = val;
	else
        param[brd][ch] = val;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Read a config file, parse the parameters and set the relevant fields in the WDcfg structure
// Inputs:		f_ini: config file pinter
// Outputs:		WDcfg: struct with all parameters
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ParseConfigFile(FILE *f_ini, Config_t *WDcfg) 
{
	char str[1000], str1[1000];
	int i, b, val, Off=0, tr = -1;
	FILE *runnum, *fcfg, *fcal;

	if (load_default) {
		/* Default settings */
		strcpy(WDcfg->InputDataFileName, "");
		strcpy(WDcfg->DataFilePath, DATA_FILE_PATH);
		strcpy(WDcfg->InputDataFilePath, DATA_FILE_PATH);
		strcpy(WDcfg->RunDescription, "");
		strcpy(WDcfg->FwTypeString, "Unknown");
		WDcfg->NumBrd = 0;
		WDcfg->NumPhyCh = 8;
		WDcfg->NumAcqCh = 8;
		if (jobrun < 0)	WDcfg->RunNumber = 0;
		else jobrun = WDcfg->RunNumber;
		WDcfg->SaveHistograms = 0x3;  // Energy and Time (PSD disabled)
		WDcfg->OutFileFormat = OUTFILE_ASCII;
		WDcfg->OutFileTimeStampUnit = 0; 
		WDcfg->HeaderInListFiles = 0;
		WDcfg->NoBadEventsInListFiles = 0;
		WDcfg->InputFileType = 0;
		WDcfg->ConfirmFileOverwrite = 0;
		WDcfg->HistoOutputFormat = HISTO_FILE_FORMAT_1COL;
		WDcfg->AutoRunNumber = 0;
		WDcfg->JobStartRun = 0;
		WDcfg->JobStopRun = 0;
		WDcfg->StatUpdateTime = 1000;
		WDcfg->AcquisitionMode = ACQ_MODE_LIST;
		WDcfg->WaveformEnabled = 0;
		WDcfg->StartMode = START_MODE_INDEP_SW;
		WDcfg->TrginMode = TRGIN_MODE_GLOBAL_TRG;
		WDcfg->SyncinMode = SYNCIN_MODE_TSTAMP_RESET;
		WDcfg->TrgoutMode = TRGOUT_MODE_CH_TRG;
		WDcfg->TrgoutMask = 0xFF;
		WDcfg->StopOnTotalEvents = 0;
		WDcfg->StopOnEnergyEvents = 0;
		WDcfg->StopOnTimeEvents = 0;
		WDcfg->StopOnTime = 0;
		WDcfg->StopOnLiveTime = 0;
		WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_NIM;
		WDcfg->RecordLength = 1024;
		WDcfg->PreTrigger = 128;
		WDcfg->TrgHoldOff = 1;
		WDcfg->CoincMode=0;  
		WDcfg->AntiCoincidence=0;
		WDcfg->CoincWindow = 10;
		WDcfg->MajorityLevel = 2;
		WDcfg->FanSpeed = 0;
		WDcfg->BuildMode = EVBUILD_NONE;
		WDcfg->EnableEnergyFilter = 0;
		WDcfg->EnableEnergySkim = 0;
		WDcfg->EnablePSDFilter = 0;
		WDcfg->WaveformProcessor = 0;
		WDcfg->SaveWaveforms = 0;
		WDcfg->TimeCorrelWindow = 100;
		WDcfg->GWn = 0;
		WDcfg->THnbin = TMAXNBITS;
		WDcfg->EHnbin = EMAXNBITS;
		WDcfg->MCSHnbin = 1024;
		WDcfg->DwellTime = 1000;
		WDcfg->CalibrationRun = 0;
		WDcfg->EventBuffering = 0;
		WDcfg->TspectrumMode = TAC_SPECTRUM_COMMON_START;
		WDcfg->ScatterPlotMode = SCATTER_PSD_HORIZONTAL;
		WDcfg->CloverNch = 4;
		WDcfg->CloverMajority = 1;
		WDcfg->AddBackFullScale = 0; // KeV
        for(b=0; b<MAX_NBRD; b++) {
			for(i=0; i<MAX_NCH; i++) {
				WDcfg->DCoffset[b][i]=8000;  
				WDcfg->ACcoupling[MAX_NBRD][MAX_NCH]=0;			
				WDcfg->ACPZcomp[MAX_NBRD][MAX_NCH]=0x8000;			
				WDcfg->BaselineDCoffset[b][i]=-1;
				WDcfg->EnableInput[b][i]=1;  
				WDcfg->TrgThreshold[b][i]=10;  
				WDcfg->PulsePolarity[b][i]=1;  
				WDcfg->NsBaseline[b][i]=2;  
				WDcfg->TrapNSBaseline[b][i]=-1; 
				WDcfg->FixedBaseline[b][i]=0;  
				WDcfg->GateWidth[b][i]=20;  
				WDcfg->ShortGateWidth[b][i]=5;  
				WDcfg->PreGate[b][i]=2;  
				WDcfg->ChargeLLD[b][i]=0;
				WDcfg->ChargeSensitivity[b][i]=-1; // if not set by the user, this is calculated from the EnergyGain
				WDcfg->EnablePedestal[b][i]=0;
				WDcfg->InputDynamicRange[b][i]=0;  
				WDcfg->CFDfraction[b][i]=0;  
				WDcfg->CFDdelay[b][i]=0;  
				WDcfg->CFDinterp[b][i]=0;  
				WDcfg->DiscrMode[b][i]=0;  
				WDcfg->EnableIPE[b][i]=0;  
				WDcfg->IPEaddnoise[b][i]=1;
				WDcfg->IPErandom[b][i]=1;
				WDcfg->IPEamplitude[b][i]=3;
				WDcfg->IPEdecay[b][i]=2;
				WDcfg->IPEfrequency[b][i]=1;
				WDcfg->IPErisetime[b][i]=1;
				WDcfg->PSDcutThr[b][i]=0;
				WDcfg->PSDcutType[b][i]=0;
				WDcfg->PileUpMode[b][i]=0;
				WDcfg->PurGap[b][i]=10;
				WDcfg->TrapRiseTime[b][i]=5000;
				WDcfg->TrapFlatTop[b][i]=1000;
				WDcfg->TrapPoleZero[b][i]=50000;
				WDcfg->PeakingTime[b][i]=500;
				WDcfg->NSPeak[b][i]=0;
				WDcfg->PeakHoldOff[b][i]=1000;
				WDcfg->TTFsmoothing[b][i]=8;
				WDcfg->SmoothedForCharge[b][i]=0;
				WDcfg->TTFdelay[b][i]=100;
				WDcfg->Decimation[b][i]=0;
				WDcfg->TrapDigitalGain[b][i]=-1;  // if not set by the user, this is calculated from the EnergyGain
				WDcfg->VetoWindow[b][i]=0;
				WDcfg->HV_Vset[b][i]=0;  // 0 means "read form HW"
				WDcfg->HV_Iset[b][i]=0;  // 0 means "read form HW"
				WDcfg->HV_RampUp[b][i]=0;  // 0 means "read form HW"
				WDcfg->HV_RampDown[b][i]=0;  // 0 means "read form HW"

				WDcfg->ZeroVoltLevel[b][i]=0;  
				WDcfg->EnableZCcalibr[b][i] = 0;
				WDcfg->EnergyLCut[b][i]=0;  
				WDcfg->EnergyUCut[b][i]=0;  
				WDcfg->PsdLCut[b][i]=0;  
				WDcfg->PsdUCut[b][i]=0;  
				WDcfg->EnergyCoarseGain[b][i]=1.0;
				WDcfg->EnergyFineGain[b][i]=1.0;
				WDcfg->EnergyDiv[b][i]=1;
				WDcfg->ECalibration_c0[b][i]=0;
				WDcfg->ECalibration_c1[b][i]=1.0;
				WDcfg->ECalibration_c2[b][i]=0;
				WDcfg->ECalibration_c3[b][i]=0;
				WDcfg->TstampOffset[b][i]=0;  
				WDcfg->THmin[b][i]=0;  
				WDcfg->THmax[b][i]=TMAXNBITS;  
			}
		}
	}


	// search for baseline calibration file (used with standard FW to refer the trigger threshold to the baseline, that is the zero of the signal)
	fcal = fopen("bslcal.txt", "r");
	if (fcal != NULL) {
		int br, ch, bz, r, i;
		while(!feof(fcal)) {
			r  = fscanf(fcal, "%d", &br);
			r += fscanf(fcal, "%d", &ch);
			r += fscanf(fcal, "%d", &bz);
			if ((r == 3) && (br >= 0) && (br < MAX_NBRD)) {
				if (ch == -1) {
					for(i=0; i<MAX_NCH; i++)
						WDcfg->ZeroVoltLevel[br][i]=bz;
				} else if ((ch >= 0) && (ch < MAX_NCH))
					WDcfg->ZeroVoltLevel[br][ch]=bz;
			} else {
				break;
			}
		}
	}

	// append cfg file to _cfg.txt
	if (load_default)
		fcfg = fopen("_cfg.txt", "w");
	else
		fcfg = fopen("_cfg.txt", "a");
	while ((fcfg != NULL) && (!feof(f_ini))) {
		char line[1000], str[100];
		fgets(line, 1000, f_ini);
		sscanf(line, "%s", str);
        if ((str[0] != '#') && (!streq(str, "Load")))
			fputs(line, fcfg);
	}
	fclose(fcfg);
	rewind(f_ini);

	/* read config file and assign parameters */
	while(!feof(f_ini)) {
		int read;

		ValidParameterName = 0;
        // read a word from the file
        read = fscanf(f_ini, "%s", str);
        if( !read || (read == EOF) || !strlen(str))
			continue;

		// Parameter renaming for backward compatibility with old config files
		if (streq(str, "CFDatten"))					strcpy(str, "CFDfraction");
		if (streq(str, "EnergyLLD"))				strcpy(str, "EnergyLCut");
		if (streq(str, "EnergyULD"))				strcpy(str, "EnergyUCut");
		if (streq(str, "PsdLLD"))					strcpy(str, "PsdLCut");
		if (streq(str, "PsdULD"))					strcpy(str, "PsdUCut");
		if (streq(str, "EnergyGain"))				strcpy(str, "EnergyFineGain");
		if (streq(str, "DelayLine"))				strcpy(str, "TstampOffset");
		if (streq(str, "EnableTimeCorrelFilter"))	strcpy(str, "EventBuildMode");
		// Discontinued
		if (streq(str, "RunNumberInDataFiles") || streq(str, "AntiCoincidence") || streq(str, "EHmin") || streq(str, "EHmax")) {
			fgets(str, 1000, f_ini);  // read line
			msg_printf(MsgLog, "WARNING: %s: discontinued\n", str); 
		}

        // skip comments
        if (str[0] == '#') {
			fgets(str, 1000, f_ini);
			continue;
		}
		if (streq(str, "@ON")) {
			Off = 0;
			continue;
		} else if (streq(str, "@OFF")) {
			Off = 1;
			continue;
		}
        if (Off) continue;

        // Section (COMMON or individual channel)
		if (str[0] == '[')	{
			ValidParameterName = 1;
			if (strstr(str, "COMMON")!=NULL) {
				ch = -1;
				brd = -1;
			} else if (strstr(str, "BOARD")!=NULL) {
				ch = -1;
				fscanf(f_ini, "%s", str1);
				sscanf(str1, "%d", &val);
				if (val < 0 || val >= MAX_NBRD) msg_printf(MsgLog, "%s: Invalid board number\n", str);
				else brd = val;
			} else if (strstr(str, "CHANNEL")!=NULL) {
				fscanf(f_ini, "%s", str1);
				sscanf(str1, "%d", &val);
				if (val < 0 || val >= MAX_NCH) msg_printf(MsgLog, "%s: Invalid channel number\n", str);
				if ((brd == -1) && (WDcfg->NumBrd == 1)) brd = 0;
				else ch = val;
			} else if (strstr(str, "JOBEND")!=NULL) {
				continue;
			} else if (strstr(str, "JOB")!=NULL) {
				fscanf(f_ini, "%s", str1);
				sscanf(str1, "%d", &val);
				if (val != jobrun) {
					while(!streq(str, "[JOBEND]")) fscanf(f_ini, "%s", str);
				}
				continue;
			}
		}
        // LOAD: open another config file
		if (streq(str, "Load"))	{	
			FILE *cf;
			fscanf(f_ini, "%s", str1);
			cf = fopen(str1, "r");
			if (cf!=NULL) {
				load_default=0;
				ParseConfigFile(cf, WDcfg);
				fclose(cf);
				load_default=1;
			} else {
				msg_printf(MsgLog, "Can't open secondary config file %s\n", str1);
			}
		}
 		if (streq(str, "Open"))	{
			if (brd==-1) {
				msg_printf(MsgLog, "%s: cannot be a common setting (must be in a [BOARD] section)\n", str); 
				fgets(str1, 1000, f_ini);
			} else {
				fscanf(f_ini, "%s", str1);
				if	(streq(str1, "USB"))			WDcfg->LinkType[brd] = CAEN_DGTZ_USB;
				else if (streq(str1, "PCI"))		WDcfg->LinkType[brd] = CAEN_DGTZ_PCI_OpticalLink;
				else if (streq(str1, "VIRTUAL"))	WDcfg->LinkType[brd] = VIRTUAL_BOARD_TYPE;
				else 	msg_printf(MsgLog, "%s: invalid setting for %s\n", str1, str);

				WDcfg->LinkNum[brd] = GetInt(f_ini);
				if (WDcfg->LinkType[brd] == CAEN_DGTZ_USB) WDcfg->ConetNode[brd] = 0;
				else WDcfg->ConetNode[brd] = GetInt(f_ini);
				if (WDcfg->LinkType[brd] != VIRTUAL_BOARD_TYPE)
					WDcfg->BaseAddress[brd] = GetHex(f_ini);
				WDcfg->NumBrd++;
			}
		}
		if (streq(str, "WriteRegister")) {	
			if (WDcfg->GWn < MAX_GW) {
				WDcfg->GWbrd[WDcfg->GWn]=brd;
				fscanf(f_ini, "%x", (int *)&WDcfg->GWaddr[WDcfg->GWn]);
				fscanf(f_ini, "%x", (int *)&WDcfg->GWdata[WDcfg->GWn]);
				fscanf(f_ini, "%x", (int *)&WDcfg->GWmask[WDcfg->GWn]);
				WDcfg->GWn++;
			} else {
				msg_printf(MsgLog, "WARNING: MAX_GW Generic Write exceeded (%d). Change MAX_GW and recompile\n", MAX_GW);
			}
		}
		if (streq(str, "WriteRegisterBits")) {	
			if (WDcfg->GWn < MAX_GW) {
				int start, stop, data;
				WDcfg->GWbrd[WDcfg->GWn]=brd;
				fscanf(f_ini, "%x", (int *)&WDcfg->GWaddr[WDcfg->GWn]);
				fscanf(f_ini, "%d", &start);
				fscanf(f_ini, "%d", &stop);
				fscanf(f_ini, "%d", &data);
				WDcfg->GWmask[WDcfg->GWn] = ((1<<(stop-start+1))-1) << start;
				WDcfg->GWdata[WDcfg->GWn] = ((uint32_t)data << start) & WDcfg->GWmask[WDcfg->GWn];
				WDcfg->GWn++;
			} else {
				msg_printf(MsgLog, "WARNING: MAX_GW Generic Write exceeded (%d). Change MAX_GW and recompile\n", MAX_GW);
			}
		}
		if (streq(str, "AcquisitionMode"))	{	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "LIST"))					WDcfg->AcquisitionMode = ACQ_MODE_LIST;
			else if (streq(str1, "MIXED"))					WDcfg->AcquisitionMode = ACQ_MODE_MIXED;
			else if (streq(str1, "OFF_LINE"))				WDcfg->AcquisitionMode = ACQ_MODE_OFFLINE;
			else if (streq(str1, "EMULATOR"))				WDcfg->AcquisitionMode = ACQ_MODE_EMULATOR_MIXED;
			else if (streq(str1, "EMULATOR_MIXED"))			WDcfg->AcquisitionMode = ACQ_MODE_EMULATOR_MIXED;
			else if (streq(str1, "EMULATOR_LIST"))			WDcfg->AcquisitionMode = ACQ_MODE_EMULATOR_LIST;
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "TriggerMode"))	{	fscanf(f_ini, "%s", str1); msg_printf(MsgLog, "WARNING: %s: discontinued\n", str); } // discontinued
		if (streq(str, "StartMode"))	{	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "INDEP_SW"))				WDcfg->StartMode = START_MODE_INDEP_SW;		
			else if	(streq(str1, "SYNCIN_1ST_SW"))			WDcfg->StartMode = START_MODE_SYNCIN_1ST_SW;
			else if	(streq(str1, "SYNCIN_1ST_HW"))			WDcfg->StartMode = START_MODE_SYNCIN_1ST_HW;
			else if (streq(str1, "TRGIN_1ST_SW")) 			WDcfg->StartMode = START_MODE_TRGIN_1ST_SW;	
			else if (streq(str1, "TRGIN_1ST_HW"))			WDcfg->StartMode = START_MODE_TRGIN_1ST_HW;	
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "TrginMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "DISABLED"))				WDcfg->TrginMode = TRGIN_MODE_DISABLED;
			else if	((streq(str1,"COMMON_TRG")) ||	
					(streq(str1, "EXTTRG_ONLY")))			WDcfg->TrginMode = TRGIN_MODE_GLOBAL_TRG;
			else if	((streq(str1,"EXTTRG_START")) ||		// discontinued
					(streq(str1, "COMMON_TRG_START")))		WDcfg->TrginMode = TRGIN_MODE_GLOBAL_TRG;  // discontinued
			else if (streq(str1, "VETO")) 					WDcfg->TrginMode = TRGIN_MODE_VETO;
			else if (streq(str1, "GATE"))					WDcfg->TrginMode = TRGIN_MODE_GATE;
			else if (streq(str1, "COINC"))					WDcfg->TrginMode = TRGIN_MODE_COINC; 
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "SyncinMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "DISABLED"))				WDcfg->SyncinMode = SYNCIN_MODE_DISABLED;
			else if	(streq(str1, "TSTAMP_RESET"))			WDcfg->SyncinMode = SYNCIN_MODE_TSTAMP_RESET;
			else if (streq(str1, "RUN_CTRL")) 				WDcfg->SyncinMode = SYNCIN_MODE_RUN_CTRL;
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "TrgoutMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "DISABLED"))				WDcfg->TrgoutMode = TRGOUT_MODE_DISABLED;
			else if	(streq(str1, "CHANNEL_TRIGGERS"))		WDcfg->TrgoutMode = TRGOUT_MODE_CH_TRG;
			else if	(streq(str1, "PROP_TRGIN"))				WDcfg->TrgoutMode = TRGOUT_MODE_PROP_TRGIN;
			else if (streq(str1, "SYNC_OUT")) 				WDcfg->TrgoutMode = TRGOUT_MODE_SYNC_OUT;
			else if (streq(str1, "SQR_WAVE_1KHZ")) 			WDcfg->TrgoutMode = TRGOUT_MODE_SQR_1KHZ;
			else if (streq(str1, "PULSES_1KHZ")) 			WDcfg->TrgoutMode = TRGOUT_MODE_PLS_1KHZ;
			else if (streq(str1, "SQR_WAVE_10KHZ")) 		WDcfg->TrgoutMode = TRGOUT_MODE_SQR_10KHZ;
			else if (streq(str1, "PULSES_10KHZ")) 			WDcfg->TrgoutMode = TRGOUT_MODE_PLS_10KHZ;
			else if (streq(str1, "CLOCK")) 					WDcfg->TrgoutMode = TRGOUT_CLOCK;
			else if (streq(str1, "SIGSCOPE")) 				WDcfg->TrgoutMode = TRGOUT_SIGSCOPE;
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "CoincMode")) {	
			fscanf(f_ini, "%s", str1);
			if      (streq(str1, "DISABLED"))				WDcfg->CoincMode = COINC_DISABLED;
			else if (streq(str1, "0"))						WDcfg->CoincMode = COINC_DISABLED;  // for compatibility with old settings
			else if (streq(str1, "MAJORITY"))				WDcfg->CoincMode = COINC_MAJORITY;
			else if (streq(str1, "MINORITY"))				WDcfg->CoincMode = COINC_MINORITY;
			else if (streq(str1, "COUPLES"))				WDcfg->CoincMode = COINC_PAIRED_AND;  // back compatibility
			else if (streq(str1, "PAIRED_AND"))				WDcfg->CoincMode = COINC_PAIRED_AND;
			else if (streq(str1, "ANTI_PAIRED_AND"))		WDcfg->CoincMode = COINC_PAIRED_NAND;
			else if (streq(str1, "PAIRED_NAND"))			WDcfg->CoincMode = COINC_PAIRED_NAND;
			else if (streq(str1, "PAIRED_OR"))				WDcfg->CoincMode = COINC_PAIRED_OR;
			else if (streq(str1, "CH0_AND_ANY"))			WDcfg->CoincMode = COINC_CH0_AND_ANY;
			else if (streq(str1, "CH0_NAND_ANY"))			WDcfg->CoincMode = COINC_CH0_NAND_ANY;
			else if (streq(str1, "CH0_TO_ALL"))				WDcfg->CoincMode = COINC_CH0_TO_ALL;
			else if (streq(str1, "AND_ALL"))				WDcfg->CoincMode = COINC_AND_ALL;
			else if (streq(str1, "OR_ALL"))					WDcfg->CoincMode = COINC_OR_ALL;
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "DiscrMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "DISABLED"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_DISABLED); 
			else if	(streq(str1, "-1"))						SetChannelParam(WDcfg->DiscrMode, -1); 
			else if	(streq(str1, "LED_PSD"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_LED_PSD); 
			else if	(streq(str1, "RCCR2_PHA"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_RCCR2_PHA);
			else if	(streq(str1, "RCCR2"))					SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_RCCR2_PHA);
			else if	(streq(str1, "0"))						SetChannelParam(WDcfg->DiscrMode, 0);
			else if	(streq(str1, "CFD_PSD"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_CFD_PSD);
			else if	(streq(str1, "CFD"))					SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_CFD_PSD);
			else if	(streq(str1, "1"))						SetChannelParam(WDcfg->DiscrMode, 1);
			else if	(streq(str1, "CFD_PHA"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_CFD_PHA);
			else if	(streq(str1, "2"))						SetChannelParam(WDcfg->DiscrMode, 2);
			else if	(streq(str1, "LED_PHA"))				SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_LED_PHA);
			else if	(streq(str1, "LED"))					SetChannelParam(WDcfg->DiscrMode, DISCR_MODE_LED_PHA);
			else if	(streq(str1, "3"))						SetChannelParam(WDcfg->DiscrMode, 3);
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "OutFileFormat")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "BINARY"))					WDcfg->OutFileFormat = OUTFILE_BINARY;
			else if (streq(str1, "ASCII"))					WDcfg->OutFileFormat = OUTFILE_ASCII;
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "InputFileType")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "RAW"))					WDcfg->InputFileType = INPUTFILE_RAW;
			else if (streq(str1, "BINARY_LIST"))			WDcfg->InputFileType = INPUTFILE_BINARY_LIST;
			else if (streq(str1, "ASCII_LIST"))				WDcfg->InputFileType = INPUTFILE_ASCII_LIST;
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "HistoOutputFormat")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "ASCII_1COL"))				WDcfg->HistoOutputFormat = HISTO_FILE_FORMAT_1COL;
			else if	(streq(str1, "ASCII_2COL"))				WDcfg->HistoOutputFormat = HISTO_FILE_FORMAT_2COL;
			else if (streq(str1, "ANSI_42"))				WDcfg->HistoOutputFormat = HISTO_FILE_FORMAT_ANSI42;
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "TspectrumMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "START_STOP"))				WDcfg->TspectrumMode = TAC_SPECTRUM_COMMON_START;
			else if	(streq(str1, "INTERVALS"))				WDcfg->TspectrumMode = TAC_SPECTRUM_INTERVALS;
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "ScatterPlotMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "PSD_HORIZONTAL"))			WDcfg->ScatterPlotMode = SCATTER_PSD_HORIZONTAL;
			else if	(streq(str1, "PSD_DIAGONAL"))			WDcfg->ScatterPlotMode = SCATTER_PSD_DIAGONAL;
			else if	(streq(str1, "E_VS_DELTAE"))			WDcfg->ScatterPlotMode = SCATTER_E_VS_DELTAE;
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "EventBuildMode")) {	
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "0"))						WDcfg->BuildMode = EVBUILD_NONE;
			else if	(streq(str1, "NONE"))					WDcfg->BuildMode = EVBUILD_NONE;
			else if	(streq(str1, "1"))						WDcfg->BuildMode = EVBUILD_CHREF_AND_ANYOTHER;
			else if	(streq(str1, "CHREF_AND_ANYOTHER"))		WDcfg->BuildMode = EVBUILD_CHREF_AND_ANYOTHER;
			else if	(streq(str1, "PAIRED_AND"))				WDcfg->BuildMode = EVBUILD_PAIRED_AND;
			else if	(streq(str1, "CLOVER")) {
				int np, p1, p2;
				WDcfg->BuildMode = EVBUILD_CLOVER;
				fgets(str1, 1000, f_ini);
				np = sscanf(str1, "%d %d", &p1, &p2);
				if (np >= 1) WDcfg->CloverNch = p1;
				if (np >= 2) WDcfg->CloverMajority = p2;
				if ((WDcfg->CloverNch <= 0) || (WDcfg->CloverNch > 16)) {
					msg_printf(MsgLog, "WARNING: Invalid Num of Ch for the Clover. Forced to 4\n");
					WDcfg->CloverNch = 4;
				}
			}
			else	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "FPIOtype"))	{	
			fscanf(f_ini, "%s", str1);
			if      (streq(str1, "TTL"))					WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_TTL;
			else if (streq(str1, "NIM"))					WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_NIM;
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "EnergyCoarseGain"))	{	
			float cg;
			fscanf(f_ini, "%s", str1);
			if      (streq(str1, "1/16")) cg = 0.0625;
			else if (streq(str1, "1/8"))  cg = 0.125;
			else if (streq(str1, "1/4"))  cg = 0.25;
			else if (streq(str1, "1/2"))  cg = 0.5;
			else sscanf(str1, "%f", &cg);
			SetChannelParamFloat(WDcfg->EnergyCoarseGain, cg);
		}

		if (streq(str, "OutFileTimeStampUnit"))	{	
			fscanf(f_ini, "%s", str1);
			if (isdigit(str1[0])) sscanf(str1, "%d", &WDcfg->OutFileTimeStampUnit);
			else if (streq(str1, "ps")) WDcfg->OutFileTimeStampUnit = 0;
			else if (streq(str1, "ns")) WDcfg->OutFileTimeStampUnit = 1;
			else if (streq(str1, "us")) WDcfg->OutFileTimeStampUnit = 2;
			else if (streq(str1, "ms")) WDcfg->OutFileTimeStampUnit = 3;
			else if (streq(str1, "s"))  WDcfg->OutFileTimeStampUnit = 4;
			else msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}

		if (streq(str, "JobRuns")) {	
			WDcfg->JobStartRun = GetInt(f_ini); 
			WDcfg->JobStopRun = GetInt(f_ini); 
			if (jobrun < 0) {
				jobrun = WDcfg->JobStartRun;
				WDcfg->RunNumber = WDcfg->JobStartRun;
			}
		}
		if (streq(str, "RunDescription")) {
			int i;
			fgets(str1, 1000, f_ini);
			for(i=0; i<(int)strlen(str1); i++)
				if (!isspace(str1[i])) break;
			if (i<(int)strlen(str1))
				sprintf(WDcfg->RunDescription, "%s", &str1[i]);
		}
		if (streq(str, "ECalibration")) {
			float c0, c1, c2, c3;
			int np;
			fgets(str1, 1000, f_ini);
			np = sscanf(str1, "%f %f %f %f", &c0, &c1, &c2, &c3);
			if (np == 1) {
				c1 = c0;
				c0 = 0;
			} 
			if (np < 3) c2 = 0;
			if (np < 4) c3 = 0;
			SetChannelParamFloat(WDcfg->ECalibration_c0, c0);
			SetChannelParamFloat(WDcfg->ECalibration_c1, c1);
			SetChannelParamFloat(WDcfg->ECalibration_c2, c2);
			SetChannelParamFloat(WDcfg->ECalibration_c3, c3);
		}
		if (streq(str, "PulsePolarity")) {
			fscanf(f_ini, "%s", str1);
			if		(streq(str1, "NEGATIVE"))		SetChannelParam(WDcfg->PulsePolarity, CAEN_DGTZ_PulsePolarityNegative);
			else if (streq(str1, "POSITIVE"))		SetChannelParam(WDcfg->PulsePolarity, CAEN_DGTZ_PulsePolarityPositive);
			else 	msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
		}
		if (streq(str, "RunNumber")) {
			fscanf(f_ini, "%s", str1);
			if (streq(str1, "AUTO")) {
				WDcfg->AutoRunNumber = 1;
			} else {
				int rn;
				if (sscanf(str1, "%d", &rn) == 1)
					WDcfg->RunNumber = rn;
				else
					msg_printf(MsgLog, "WARNING: %s: invalid setting for %s\n", str1, str);
			}
		}
		if (streq(str, "InputDataFileName"))		fscanf(f_ini, "%s", WDcfg->InputDataFileName); 
		if (streq(str, "DataFilePath"))				fscanf(f_ini, "%s", WDcfg->DataFilePath);
		if (streq(str, "InputDataFilePath"))		fscanf(f_ini, "%s", WDcfg->InputDataFilePath);
		if (streq(str, "ZCcalibFileName"))			fscanf(f_ini, "%s", SysVars.ZCcalibrFileName);
		if (streq(str, "ZCcalibRun"))				WDcfg->CalibrationRun			= GetInt(f_ini); 
		if (streq(str, "JobSleep"))					WDcfg->JobSleep					= (int)GetTime(f_ini, "ms"); 
		if (streq(str, "LoopInputFile"))			WDcfg->LoopInputFile			= GetInt(f_ini); 
		if (streq(str, "HeaderInListFiles"))		WDcfg->HeaderInListFiles		= GetInt(f_ini);
		if (streq(str, "NoBadEventsInListFiles"))	WDcfg->NoBadEventsInListFiles	= GetInt(f_ini);
		if (streq(str, "ConfirmFileOverwrite"))		WDcfg->ConfirmFileOverwrite		= GetInt(f_ini);  
		if (streq(str, "FlagsInListFiles"))			WDcfg->FlagsInListFiles			= GetHex(f_ini);  
		if (streq(str, "AutoRunNumber"))			WDcfg->AutoRunNumber			= GetInt(f_ini);  // deprecated (use "RunNumber AUTO")
		if (streq(str, "StatUpdateTime"))			WDcfg->StatUpdateTime			= (int)GetTime(f_ini, "ms");  
		if (streq(str, "EventBuffering"))			WDcfg->EventBuffering			= GetInt(f_ini);
		if (streq(str, "TrgoutMask"))				WDcfg->TrgoutMask				= GetHex(f_ini);
		if (streq(str, "RecordLength"))				WDcfg->RecordLength				= GetInt(f_ini);
		if (streq(str, "PreTrigger")) 				WDcfg->PreTrigger				= GetInt(f_ini); 
		if (streq(str, "TrgHoldOff")) 				WDcfg->TrgHoldOff				= (int)GetTime(f_ini, "ns");
		if (streq(str, "StopOnTime")) 				WDcfg->StopOnTime				= (int)GetTime(f_ini, "ms");
		if (streq(str, "StopOnLiveTime")) 			WDcfg->StopOnLiveTime			= (int)GetTime(f_ini, "ms");
		if (streq(str, "StopOnTotalEvents"))		WDcfg->StopOnTotalEvents		= GetInt(f_ini);
		if (streq(str, "StopOnTimeEvents"))			WDcfg->StopOnTimeEvents			= GetInt(f_ini);
		if (streq(str, "StopOnEnergyEvents"))		WDcfg->StopOnEnergyEvents		= GetInt(f_ini);
		if (streq(str, "THnbin"))					WDcfg->THnbin					= GetHnbin(f_ini);
		if (streq(str, "EHnbin"))					WDcfg->EHnbin					= GetHnbin(f_ini);
		if (streq(str, "MCSHnbin"))					WDcfg->MCSHnbin					= GetHnbin(f_ini);
		if (streq(str, "DwellTime"))				WDcfg->DwellTime				= (int)(GetTime(f_ini, "us"));
		//if (streq(str, "EnableTimeCorrelFilter"))	WDcfg->EnableTimeCorrelFilter	= GetInt(f_ini);
		if (streq(str, "EnableEnergyFilter"))		WDcfg->EnableEnergyFilter		= GetInt(f_ini);
		if (streq(str, "EnableEnergySkim"))			WDcfg->EnableEnergySkim			= GetInt(f_ini);
		if (streq(str, "EnablePSDFilter"))			WDcfg->EnablePSDFilter			= GetInt(f_ini);
		if (streq(str, "AddBackFullScale"))			WDcfg->AddBackFullScale			= GetFloat(f_ini);
		if (streq(str, "TimeCorrelWindow")) 		WDcfg->TimeCorrelWindow			= GetTime(f_ini, "ns");
		if (streq(str, "WaveformProcessor")) 		WDcfg->WaveformProcessor		= GetInt(f_ini);
		if (streq(str, "FanSpeed"))					WDcfg->FanSpeed					= GetInt(f_ini);
		if (streq(str, "TOFstartChannel"))			WDcfg->TOFstartChannel			= GetInt(f_ini);
		if (streq(str, "TOFstartBoard"))			WDcfg->TOFstartBoard			= GetInt(f_ini);
		if (streq(str, "SaveLists"))				WDcfg->SaveLists				= GetInt(f_ini);
		if (streq(str, "SaveRawData"))				WDcfg->SaveRawData				= GetInt(f_ini);
		if (streq(str, "SaveHistograms"))			WDcfg->SaveHistograms			= GetInt(f_ini);
		if (streq(str, "SaveWaveforms"))			WDcfg->SaveWaveforms			= GetInt(f_ini);
		if (streq(str, "SaveRunInfo"))				WDcfg->SaveRunInfo				= GetInt(f_ini);
		if (streq(str, "CoincWindow"))				WDcfg->CoincWindow				= (int)GetTime(f_ini, "ns");
		if (streq(str, "MajorityLevel"))			WDcfg->MajorityLevel			= GetInt(f_ini);
		if (streq(str, "EnableInput"))				SetChannelParam(WDcfg->EnableInput,				GetInt(f_ini));
		if (streq(str, "DCoffset"))					SetChannelParam(WDcfg->DCoffset,				(int)((GetFloat(f_ini)+50) * 65535 / 100));
		if (streq(str, "ACcoupling"))				SetChannelParam(WDcfg->ACcoupling,				GetInt(f_ini));
		if (streq(str, "ACPZcomp"))					SetChannelParam(WDcfg->ACPZcomp,				GetInt(f_ini));
		if (streq(str, "HV_Vset"))					SetChannelParamFloat(WDcfg->HV_Vset,			GetFloat(f_ini));
		if (streq(str, "HV_Iset"))					SetChannelParamFloat(WDcfg->HV_Iset,			GetFloat(f_ini));
		if (streq(str, "HV_RampUp"))				SetChannelParamFloat(WDcfg->HV_RampUp,			GetFloat(f_ini));
		if (streq(str, "HV_RampDown"))				SetChannelParamFloat(WDcfg->HV_RampDown,		GetFloat(f_ini));
		if (streq(str, "BaselineDCoffset"))			SetChannelParamFloat(WDcfg->BaselineDCoffset,	GetFloat(f_ini));
		if (streq(str, "TriggerThreshold"))			SetChannelParam(WDcfg->TrgThreshold,			GetInt(f_ini));
		if (streq(str, "NSBaseline")) 				SetChannelParam(WDcfg->NsBaseline,				GetInt(f_ini));
		if (streq(str, "TrapNSBaseline")) 			SetChannelParam(WDcfg->TrapNSBaseline,			GetInt(f_ini));
		if (streq(str, "FixedBaseline")) 			SetChannelParam(WDcfg->FixedBaseline,			GetInt(f_ini));
		if (streq(str, "PreGate")) 					SetChannelParam(WDcfg->PreGate,					(int)GetTime(f_ini, "ns"));
		if (streq(str, "ChargeSensitivity"))		SetChannelParam(WDcfg->ChargeSensitivity,		GetInt(f_ini));
		if (streq(str, "ChargeLLD"))				SetChannelParam(WDcfg->ChargeLLD,				GetInt(f_ini));
		if (streq(str, "InputDynamicRange"))		SetChannelParam(WDcfg->InputDynamicRange,		GetInt(f_ini));
		if (streq(str, "ZeroVoltLevel"))			SetChannelParam(WDcfg->ZeroVoltLevel,			GetInt(f_ini));
		if (streq(str, "EnableZCcalibr"))			SetChannelParam(WDcfg->EnableZCcalibr,			GetInt(f_ini));  
		if (streq(str, "GateWidth"))				SetChannelParam(WDcfg->GateWidth,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "ShortGateWidth"))			SetChannelParam(WDcfg->ShortGateWidth,			(int)GetTime(f_ini, "ns"));
		if (streq(str, "EnablePedestal"))			SetChannelParam(WDcfg->EnablePedestal,			GetInt(f_ini));
		if (streq(str, "TrapRiseTime"))				SetChannelParam(WDcfg->TrapRiseTime,			(int)GetTime(f_ini, "ns"));
		if (streq(str, "TrapFlatTop"))				SetChannelParam(WDcfg->TrapFlatTop,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "TrapPoleZero"))				SetChannelParam(WDcfg->TrapPoleZero,			(int)GetTime(f_ini, "ns"));
		if (streq(str, "PeakingTime"))				SetChannelParam(WDcfg->PeakingTime,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "NSPeak")) 					SetChannelParam(WDcfg->NSPeak,					GetInt(f_ini));
		if (streq(str, "PeakHoldOff"))				SetChannelParam(WDcfg->PeakHoldOff,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "Decimation"))				SetChannelParam(WDcfg->Decimation,				GetInt(f_ini));
		if (streq(str, "SmoothedForCharge"))		SetChannelParam(WDcfg->SmoothedForCharge,		GetInt(f_ini));
		if (streq(str, "TTFsmoothing"))				SetChannelParam(WDcfg->TTFsmoothing,			GetInt(f_ini));
		if (streq(str, "TTFdelay"))					SetChannelParam(WDcfg->TTFdelay,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "VetoWindow"))				SetChannelfParam(WDcfg->VetoWindow,				GetTime(f_ini, "ns"));
		if (streq(str, "TrapDigitalGain"))			SetChannelParamFloat(WDcfg->TrapDigitalGain,	GetFloat(f_ini));
		if (streq(str, "PileUpMode"))				SetChannelParam(WDcfg->PileUpMode,				GetInt(f_ini));
		if (streq(str, "PurGap"))					SetChannelParam(WDcfg->PurGap,					GetInt(f_ini));
		if (streq(str, "EnergyLCut"))				SetChannelParamFloat(WDcfg->EnergyLCut,			GetFloat(f_ini));
		if (streq(str, "EnergyUCut"))				SetChannelParamFloat(WDcfg->EnergyUCut,			GetFloat(f_ini));
		if (streq(str, "PsdLCut"))					SetChannelParamFloat(WDcfg->PsdLCut,			GetFloat(f_ini));
		if (streq(str, "PsdUCut"))					SetChannelParamFloat(WDcfg->PsdUCut,			GetFloat(f_ini));
		if (streq(str, "TstampOffset"))				SetChannelParamFloat(WDcfg->TstampOffset,		GetFloat(f_ini));
		if (streq(str, "THmin"))					SetChannelParamFloat(WDcfg->THmin,				GetTime(f_ini, "ns"));
		if (streq(str, "THmax"))					SetChannelParamFloat(WDcfg->THmax,				GetTime(f_ini, "ns"));
		if (streq(str, "EnergyFineGain"))			SetChannelParamFloat(WDcfg->EnergyFineGain,		GetFloat(f_ini));
		//if (streq(str, "EnergyOffset"))				SetChannelParamFloat(WDcfg->EnergyOffset,		GetFloat(f_ini));
		if (streq(str, "ECalibration_m"))			SetChannelParamFloat(WDcfg->ECalibration_c1,	GetFloat(f_ini));  // deprecated (use ECalibration c0 c1 c2 c3)
		if (streq(str, "ECalibration_q"))			SetChannelParamFloat(WDcfg->ECalibration_c0,	GetFloat(f_ini));  // deprecated (use ECalibration c0 c1 c2 c3)
		if (streq(str, "ECalibration_c0"))			SetChannelParamFloat(WDcfg->ECalibration_c0,	GetFloat(f_ini)); 
		if (streq(str, "ECalibration_c1"))			SetChannelParamFloat(WDcfg->ECalibration_c1,	GetFloat(f_ini)); 
		if (streq(str, "ECalibration_c2"))			SetChannelParamFloat(WDcfg->ECalibration_c2,	GetFloat(f_ini)); 
		if (streq(str, "ECalibration_c3"))			SetChannelParamFloat(WDcfg->ECalibration_c3,	GetFloat(f_ini)); 
		if (streq(str, "CFDdelay"))					SetChannelParam(WDcfg->CFDdelay,				(int)GetTime(f_ini, "ns"));
		if (streq(str, "CFDfraction"))				SetChannelParam(WDcfg->CFDfraction,				GetInt(f_ini));
		if (streq(str, "CFDinterp"))				SetChannelParam(WDcfg->CFDinterp,				GetInt(f_ini));
		if (streq(str, "PSDcutThr"))				SetChannelParamFloat(WDcfg->PSDcutThr,			GetFloat(f_ini));
		if (streq(str, "PSDcut"))					SetChannelParamFloat(WDcfg->PSDcutThr,			(float)GetInt(f_ini)/1024);  // deprecated (keep for backward compatibility)
		if (streq(str, "PSDcutType"))				SetChannelParam(WDcfg->PSDcutType,				GetInt(f_ini));
		if (streq(str, "EnableIPE"))				SetChannelParam(WDcfg->EnableIPE,				GetInt(f_ini));
		if (streq(str, "IPEamplitude"))				SetChannelParam(WDcfg->IPEamplitude,			GetInt(f_ini));
		if (streq(str, "IPErandom"))				SetChannelParam(WDcfg->IPErandom,				GetInt(f_ini));
		if (streq(str, "IPEaddnoise"))				SetChannelParam(WDcfg->IPEaddnoise,				GetInt(f_ini));
		if (streq(str, "IPErisetime"))				SetChannelParam(WDcfg->IPErisetime,				GetInt(f_ini));
		if (streq(str, "IPEfrequency"))				SetChannelParam(WDcfg->IPEfrequency,			GetInt(f_ini));
		if (streq(str, "IPEdecay"))					SetChannelParam(WDcfg->IPEdecay,				GetInt(f_ini));

		if (!ValidParameterName) {
			msg_printf(MsgLog, "WARNING: %s: unkwown parameter\n", str);
			fgets(str, (int)strlen(str)-1, f_ini);
		}
	}

#ifdef LINUX
	if (WDcfg->DataFilePath[strlen(WDcfg->DataFilePath)-1] != '/')	sprintf(WDcfg->DataFilePath, "%s/", WDcfg->DataFilePath);
	if (WDcfg->DataFilePath[strlen(WDcfg->InputDataFilePath)-1] != '/')	sprintf(WDcfg->InputDataFilePath, "%s/", WDcfg->InputDataFilePath);
#else
	if (WDcfg->DataFilePath[strlen(WDcfg->DataFilePath)-1] != '\\')	sprintf(WDcfg->DataFilePath, "%s\\", WDcfg->DataFilePath);
	if (WDcfg->InputDataFilePath[strlen(WDcfg->DataFilePath)-1] != '\\') sprintf(WDcfg->InputDataFilePath, "%s\\", WDcfg->InputDataFilePath);
#endif

	// Set Waveform mode
	if ((WDcfg->AcquisitionMode == ACQ_MODE_MIXED) || (WDcfg->AcquisitionMode == ACQ_MODE_EMULATOR_MIXED))
		WDcfg->WaveformEnabled = 1;

	// Recalculate max value of the histograms in order to have an integer bin
	for(b=0; b<MAX_NBRD; b++) {
		for(ch=0; ch<MAX_NCH; ch++) {
			double binsize;
			binsize = ceil(1000.0 * (WDcfg->THmax[b][ch] - WDcfg->THmin[b][ch]) / WDcfg->THnbin);  // in ps
			WDcfg->THmax[b][ch] = (float)(WDcfg->THmin[b][ch] + (binsize * WDcfg->THnbin) / 1000.0);
		}
	}

	// Disable not existing channels 
	for(b=WDcfg->NumBrd; b<MAX_NBRD; b++) {
		for(ch=0; ch<MAX_NCH; ch++) {
			WDcfg->EnableInput[b][ch] = 0;
		}
	}

	// Load Run Number from file 
	if ((jobrun < 0) && (load_default && WDcfg->AutoRunNumber)) {
		runnum = fopen("RunNumber.txt", "r");
		if (runnum != NULL)   {
			fscanf(runnum, "%d", &WDcfg->RunNumber);
			fclose(runnum);
		}
	}
	runnum = fopen("RunNumber.txt", "w");
	fprintf(runnum, "%d", WDcfg->RunNumber+1);
	fclose(runnum);

	return 0;
}


