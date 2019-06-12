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
#include "DataFiles.h"
#include "Console.h"

#include <stdio.h>

// Input Data File (off-line run)
FILE *InputDataFile = NULL;				// event data input file (for the data replay mode)

// Output files  HACK: who closes the output files?
FILE *list[MAX_NBRD][MAX_NCH];			// individual list files
FILE *blist[MAX_NBRD][MAX_NCH];			// built events list files
FILE *list_merged = NULL;				// list output file merging all channels
FILE *wavefile[MAX_NBRD][MAX_NCH];		// waveform output files
FILE *OutputDataFile = NULL;			// event data output file (raw)
FILE *rinf = NULL;						// run info
uint32_t OutFileSize = 0;				// Size of the output data file (in bytes)

const char tu[5][3] = {"ps", "ns", "us", "ms", "s "}; // Time units


#define OUTPUTFILE_TYPE_RAW				0
#define OUTPUTFILE_TYPE_LIST			1
#define OUTPUTFILE_TYPE_LIST_MERGED		2
#define OUTPUTFILE_TYPE_LIST_BUILT_EV	3
#define OUTPUTFILE_TYPE_WAVE			4
#define OUTPUTFILE_TYPE_EHISTO			5
#define OUTPUTFILE_TYPE_THISTO			6
#define OUTPUTFILE_TYPE_PSDHISTO		7
#define OUTPUTFILE_TYPE_RUN_INFO		8


// List Format:
// Bit[0]     = Time Stamp is present
// Bit[1]     = Energy is present
// Bit[2]     = PSD is present
// Bit[3]     = Flags are present
// Bit[15:12] = Time Units; 0=ps, 1=ns, 2=us, 3=ms; 4=s
// Bit[23:16] = Header Size (number of lines for ASCII, number of bytes for binary)
// Bit[27:24] = List Sub Type or Format version
// Bit[31:28] = List Type; 0=individual, 1=merged, 2=built


// --------------------------------------------------------------------------------------------------------- 
// Description: create string with time value expressed in given units
// --------------------------------------------------------------------------------------------------------- 
void TimeString(uint64_t tps, char *str) {
	if      (WDcfg.OutFileTimeStampUnit == 0) sprintf(str, "%16llu",  (unsigned long long)tps);							// ps
	else if (WDcfg.OutFileTimeStampUnit == 1) sprintf(str, "%16.3lf", (double)tps/1000);			// ns
	else if (WDcfg.OutFileTimeStampUnit == 2) sprintf(str, "%16.6lf", (double)tps/1000000);			// us
	else if (WDcfg.OutFileTimeStampUnit == 3) sprintf(str, "%16.9lf", (double)tps/1000000000);		// ms
	else if (WDcfg.OutFileTimeStampUnit == 4) sprintf(str, "%16.12lf",(double)tps/1000000000000);	// s
}



// --------------------------------------------------------------------------------------------------------- 
// Description: open input data file and read the header
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int OpenInputDataFile() 
{
	char fname[200];
	char DataFormat;
	char *cfgimg;
	uint32_t header[128];
	uint32_t nbcfg;
	FILE *cfg;

	if (strlen(WDcfg.InputDataFilePath) > 0)
		sprintf(fname, "%s%s", WDcfg.InputDataFilePath, WDcfg.InputDataFileName);
	else
		sprintf(fname, "%s%s", WDcfg.DataFilePath, WDcfg.InputDataFileName);
	if (InputDataFile == NULL) {
		if ((WDcfg.InputFileType == INPUTFILE_RAW) || (WDcfg.InputFileType == INPUTFILE_BINARY_LIST))
			InputDataFile = fopen(fname, "rb");
		else
			InputDataFile = fopen(fname, "r");
	} else {
		rewind(InputDataFile);  // file already open; restart from top
	}

	if (InputDataFile == NULL) {
		msg_printf(MsgLog, "ERROR: Can't open Input Data File %s\n", fname);
		return -1;
	}

	if (WDcfg.InputFileType == INPUTFILE_RAW) {
		// read the header of the data file
		if (fread(&DataFormat, sizeof(char), 1, InputDataFile) < 1) { // Data format (version)
			msg_printf(MsgLog, "ERROR: Invalid format for a raw data file\n");
			return -1;
		}
		if (DataFormat == 0) {
			fread(header, sizeof(uint32_t), 1, InputDataFile);
			fread(header+1, sizeof(uint32_t), header[0]-1, InputDataFile);
			WDcfg.RecordLength   = header[1]; 
			WDcfg.DigitizerModel = header[2];
			WDcfg.DppType        = header[3];
			WDcfg.NumBrd         = header[4];
			WDcfg.NumPhyCh       = header[5];
			WDcfg.Tsampl         = header[6];
			WDcfg.Nbit           = header[7];
		}
		if (WDcfg.RecordLength == 0)
			WDcfg.WaveformEnabled = 0;
		else
			WDcfg.WaveformEnabled = 1;
		cfgimg = (char *)malloc(1024*1024);
		cfg = fopen("_cfg_datafile.txt", "wb");
		if (cfg != NULL) {
			fread(&nbcfg, sizeof(uint32_t), 1, InputDataFile);
			if (fread(cfgimg, sizeof(char), nbcfg, InputDataFile) < nbcfg) {
				free(cfgimg);
				msg_printf(MsgLog, "ERROR: Invalid data in input file\n");
				return -1;
			}
			fwrite(cfgimg, sizeof(char), nbcfg, cfg);
		}
		fclose(cfg);
		free(cfgimg);
	} else {
		WDcfg.Tsampl = 2;
		WDcfg.Nbit = 14;
		WDcfg.WaveformEnabled = 0;
	}
	return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: create the file name for an output file
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int CreateOutputFileName(int FileType, int b, int ch, char *fname) 
{
	char prefix[100], hext[10], wlext[10];
	sprintf(prefix, "%sRun%d_", WDcfg.DataFilePath, WDcfg.RunNumber);

	if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_ANSI42) sprintf(hext, "n42");
	else sprintf(hext, "txt");
	if (WDcfg.OutFileFormat == OUTFILE_ASCII) sprintf(wlext, "txt");
	else sprintf(wlext, "dat");

	if (FileType == OUTPUTFILE_TYPE_RAW) {
		sprintf(fname, "%sraw.dat", prefix);
	} else if (FileType == OUTPUTFILE_TYPE_LIST) {
		sprintf(fname, "%sList_%d_%d.%s", prefix, b, ch, wlext);
	} else if (FileType == OUTPUTFILE_TYPE_LIST_BUILT_EV) {
		sprintf(fname, "%sList_Built_%d_%d.%s", prefix, b, ch, wlext);
	} else if (FileType == OUTPUTFILE_TYPE_LIST_MERGED) {
		sprintf(fname, "%sList_Merged.%s", prefix, wlext);
	} else if (FileType == OUTPUTFILE_TYPE_WAVE) {
		sprintf(fname, "%sWave_%d_%d.%s", prefix, b, ch, wlext);
	} else if (FileType == OUTPUTFILE_TYPE_EHISTO) {
		sprintf(fname, "%sEhisto_%d_%d.%s", prefix, b, ch, hext);
	} else if (FileType == OUTPUTFILE_TYPE_THISTO) {
		sprintf(fname, "%sThisto_%d_%d.%s", prefix, b, ch, hext);
	} else if (FileType == OUTPUTFILE_TYPE_EHISTO) {
		sprintf(fname, "%sPSDhisto_%d_%d.%s", prefix, b, ch, hext);
	} else if (FileType == OUTPUTFILE_TYPE_RUN_INFO) {
		sprintf(fname, "%sinfo.txt", prefix);
	} else {
		fname[0] = '\0';
		return -1;
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: check if the output data files are already present
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CheckOutputDataFilePresence() 
{
	int b, ch;
	char fname[300];
	FILE *of;

	// Raw data
	if (WDcfg.SaveRunInfo) {
		CreateOutputFileName(OUTPUTFILE_TYPE_RUN_INFO, 0, 0, fname);
		if ((of = fopen(fname, "r")) != NULL) return -1;
	}

	// Raw data
	if (WDcfg.SaveRawData) {
		CreateOutputFileName(OUTPUTFILE_TYPE_RAW, 0, 0, fname);
		OutputDataFile = fopen(fname, "rb");
		if ((of = fopen(fname, "r")) != NULL) return -1;
	}

	// Merged list
	if (WDcfg.SaveLists & 0x2) {
		CreateOutputFileName(OUTPUTFILE_TYPE_LIST_MERGED, 0, 0, fname);
		list_merged = fopen(fname, "rb");
		if ((of = fopen(fname, "r")) != NULL) return -1;
	}

	// Histograms, Lists and Waveforms
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch]) {

				if (WDcfg.SaveHistograms & 0x1) {
					CreateOutputFileName(OUTPUTFILE_TYPE_EHISTO, b, ch, fname);
					OutputDataFile = fopen(fname, "rb");
					if ((of = fopen(fname, "r")) != NULL) return -1;
				}
				if (WDcfg.SaveHistograms & 0x2) {
					CreateOutputFileName(OUTPUTFILE_TYPE_THISTO, b, ch, fname);
					OutputDataFile = fopen(fname, "rb");
					if ((of = fopen(fname, "r")) != NULL) return -1;
				}
				if (WDcfg.SaveHistograms & 0x4) {
					CreateOutputFileName(OUTPUTFILE_TYPE_PSDHISTO, b, ch, fname);
					OutputDataFile = fopen(fname, "rb");
					if ((of = fopen(fname, "r")) != NULL) return -1;
				}
				if (WDcfg.SaveLists & 0x1) {
					CreateOutputFileName(OUTPUTFILE_TYPE_LIST, b, ch, fname);
					OutputDataFile = fopen(fname, "rb");
					if ((of = fopen(fname, "r")) != NULL) return -1;
				}
				if (WDcfg.SaveWaveforms) {
					CreateOutputFileName(OUTPUTFILE_TYPE_WAVE, b, ch, fname);
					OutputDataFile = fopen(fname, "rb");
					if ((of = fopen(fname, "r")) != NULL) return -1;
				}
			}
		}
	}
	return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: prepare output data files (file are actually opened when used for the 1st time)
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int OpenOutputDataFiles() 
{
	char c;
	char fname[200];
	char FileFormat = DATA_FILE_FORMAT;
	char *cfgimg;
	int b, ch;
	uint32_t nbcfg;
	uint32_t header[8];
	FILE *cfg;


	if (WDcfg.ConfirmFileOverwrite && (CheckOutputDataFilePresence() < 0)) {
		msg_printf(MsgLog, "\n\nWARINING: Output files for run %d already present in %s\n", WDcfg.RunNumber, WDcfg.DataFilePath);
		printf("Set ConfirmFileOverwrite=0 to prevent asking again\n\n");
		printf("Press 'q' to quit, any other key to continue\n");
		c = getch();
		if (tolower(c) == 'q')
			return -1;
	}

	list_merged = NULL;
	for(b=0; b<MAX_NBRD; b++) 
		for(ch=0; ch < MAX_NCH; ch++)
			list[b][ch] = NULL;

	if (WDcfg.SaveRawData) {
		CreateOutputFileName(OUTPUTFILE_TYPE_RAW, 0, 0, fname);
		OutputDataFile = fopen(fname, "wb");
		if (OutputDataFile == NULL) {
			msg_printf(MsgLog, "Can't open Output Data File %s\n", fname);
			return -1;
		}

		// Write data file format
		fwrite(&FileFormat, 1, 1, OutputDataFile);

		// write data file header (1st word = header size)
		header[0] = 8;
		header[1] = WDcfg.RecordLength; 
		header[2] = WDcfg.DigitizerModel;
		header[3] = WDcfg.DppType;
		header[4] = WDcfg.NumBrd;
		header[5] = WDcfg.NumPhyCh;
		header[6] = WDcfg.Tsampl;
		header[7] = WDcfg.Nbit;
		fwrite(header, sizeof(uint32_t), header[0], OutputDataFile);

		// write config files (appended) to the output data file
		cfgimg = (char *)malloc(1024*1024);
		cfg = fopen("_cfg.txt", "rb");
		if (cfg != NULL) {
			nbcfg = (int)fread(cfgimg, sizeof(char), 1024*1024, cfg);
			fwrite(&nbcfg, sizeof(uint32_t), 1, OutputDataFile);
			fwrite(cfgimg, sizeof(char), nbcfg, OutputDataFile);
		}
		fclose(cfg);
		free(cfgimg);
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: close output data files 
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CloseOutputDataFiles() 
{
	int b, ch;

	if (OutputDataFile != NULL)	fclose (OutputDataFile);
	if (list_merged != NULL)	fclose (list_merged);
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (list[b][ch] != NULL) fclose(list[b][ch]);
			if (wavefile[b][ch] != NULL) fclose(wavefile[b][ch]);
		}
	}
	return 0;

}


// --------------------------------------------------------------------------------------------------------- 
// Description: Save one event into the raw data fila
// Inputs:		b = board index
//				ch = channel
//				EventData = event data structure
//				Wfm = waveform data structure
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveRawData(int b, int ch, GenericDPPEvent_t evnt)
{
	uint16_t info[2];
	int i;
	Waveform_t *Wfm = evnt.Waveforms;

	if (OutputDataFile == NULL) 
		return -1;

	if (OutFileSize > (uint32_t)(SysVars.MaxOutputFileDataSize * 1024 * 1024)) {
		WDcfg.SaveRawData = 0;
		printf("Raw Data Saving Stopped\n");
	} else {
		uint32_t wfmt;
		info[0] = (uint16_t)((b<<8) | ch);
		if (Wfm != NULL)   
			info[1] = Wfm->Ns;
		else
			info[1] = 0;
		fwrite(info, sizeof(uint16_t), 2, OutputDataFile); // write board/channel number and nume of samples in the waveform
		fwrite(&evnt, sizeof(GenericDPPEvent_t), 1, OutputDataFile); // write event data (struct)
		OutFileSize += 2 * sizeof(uint16_t) + sizeof(GenericDPPEvent_t);
		if (Wfm != NULL) {  
			wfmt = (Wfm->DualTrace << 31);
			for(i=0; i<MAX_NTRACES; i++)
				wfmt |= (Wfm->TraceSet[i] & 0xF) << (i*4);
			fwrite(&wfmt, sizeof(wfmt), 1, OutputDataFile);
			fwrite(Wfm->AnalogTrace[0], sizeof(uint16_t), Wfm->Ns, OutputDataFile);
			if (Wfm->DualTrace)
				fwrite(Wfm->AnalogTrace[1], sizeof(uint16_t), Wfm->Ns, OutputDataFile);
			OutFileSize += sizeof(wfmt) + (Wfm->Ns)*(sizeof(uint16_t)) + Wfm->DualTrace*(Wfm->Ns)*(sizeof(uint16_t));
		}
	}
	return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: convert time into string
// --------------------------------------------------------------------------------------------------------- 
int n42_timetostr(double time_s, char *tstring)
{
	uint64_t dd, hh, mm, ss, cc;
	uint64_t tt = (uint64_t)floor(time_s); // time in s
	dd = tt / 86400; // days
	tt = tt % 86400;
	hh = tt / 3600;  // hours
	tt = tt % 3600;
	mm = tt / 60;    // minutes
	tt = tt % 60;
	ss = tt % 60;    // seconds
	cc = (int)((time_s - floor(time_s)) * 100 + 0.5);  // cents

	// P00Y00M00DT00H00M00.00S
	sprintf(tstring, "P00Y00M%02dDT%02dH%02dM%02d.%02dS", (int)dd, (int)hh, (int)mm, (int)ss, (int)cc);
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Save an histogram to output file in N42 format
// Inputs:		FileName = file name
//				Histo = histogram to save
//              SpectrumName = spectum name
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveSpectrum_n42(char *FileName, Histogram1D_t Histo, char *SpectrumName)
{
	FILE *n42;
	char tstr[100];
	int i;

	n42 = fopen(FileName, "w");
	if (n42 == NULL) return -1;

	fprintf( n42, "<?xml version=\"1.0\"?>\n" );
	fprintf( n42, "<?xml-model href=\"http://physics.nist.gov/N42/2011/N42/schematron/n42.sch\" type=\"application/xml\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"?>\n" );
	//fprintf( n42, "<RadInstrumentData xmlns=\"http://physics.nist.gov/N42/2011/N42\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:schemaLocation=\"http://physics.nist.gov/N42/2011/N42 " n42DocUUID=\"d72b7fa7-4a20-43d4-b1b2-7e3b8c6620c1\">\n" );
	fprintf( n42, "\t<RadInstrumentInformation id=\"RadInstrumentInformation-1\">\n" );
	fprintf( n42, "\t<RadInstrumentManufacturerName>CAEN</RadInstrumentManufacturerName>\n" );
	fprintf( n42, "\t<RadInstrumentModelName>DT5780M_368</RadInstrumentModelName>\n" );
	fprintf( n42, "\t<RadInstrumentClassCode>Radionuclide Identifier</RadInstrumentClassCode>\n" );
	fprintf( n42, "\t<RadInstrumentVersion>\n" );
	fprintf( n42, "\t\t<RadInstrumentComponentName>CAEN MC2 Software</RadInstrumentComponentName>\n" );
	fprintf( n42, "\t\t<RadInstrumentComponentVersion>1.0</RadInstrumentComponentVersion>\n" );
	fprintf( n42, "\t</RadInstrumentVersion>\n" );
			 
  	fprintf( n42, "</RadInstrumentInformation>\n" );
  	fprintf( n42, "\t<RadDetectorInformation id=\"RadDetectorInformation-1\">\n" );
    fprintf( n42, "\t\t<RadDetectorCategoryCode>Gamma</RadDetectorCategoryCode>\n" );
    fprintf( n42, "\t\t<RadDetectorKindCode>HPGe</RadDetectorKindCode>\n" );
  	fprintf( n42, "\t</RadDetectorInformation>\n" );
  	fprintf( n42, "\t<EnergyCalibration id=\"EnergyCalibration-1\">\n" );
    fprintf( n42, "\t\t<CoefficientValues>\n" );
	fprintf( n42, "\t\t%f %f %f\n", Histo.A[0], Histo.A[1], Histo.A[2]);
    fprintf( n42, "\t\t</CoefficientValues>\n" );
  	fprintf( n42, "\t</EnergyCalibration>\n" );

	fprintf( n42, "\t<RegionOfInterest>\n");
	for(i=0; i<Histo.n_ROI; i++) {
		fprintf( n42, "\t\t<ROI>\n");
		fprintf( n42, "\t\t\t<Start>%d</Start>\n", Histo.ROIbegin[i]);
		fprintf( n42, "\t\t\t<End>%d</End>\n", Histo.ROIend[i]);
		fprintf( n42, "\t\t</ROI>\n");
	}
	fprintf( n42, "\t</RegionOfInterest>\n");

  	fprintf( n42, "\t<SpectrumName>%s</SpectrumName>\n", SpectrumName);
  	fprintf( n42, "\t<RebinFactor>1</RebinFactor>\n" );
  	fprintf( n42, "\t<RadMeasurement id=\"RadMeasurement-1\">\n" );
    fprintf( n42, "\t\t<MeasurementClassCode>Foreground</MeasurementClassCode>\n" );
	//strftime(tstr, sizeof(tstr), "%Y-%m-%dT%H:%M:%S", gmtime(Stats.StartTime) );  HACK
	sprintf(tstr, "   ");
    fprintf( n42, "\t\t<StartDateTime>%s</StartDateTime>\n", tstr);
	n42_timetostr(Histo.real_time, tstr);
    fprintf( n42, "\t\t<RealTimeDuration>%s</RealTimeDuration>\n", tstr);
    fprintf( n42, "\t\t<Spectrum id=\"RadMeasurement-1Spectrum-1\" radDetectorInformationReference=\"RadDetectorInformation-1\" energyCalibrationReference=\"EnergyCalibration-1\">\n" );
	n42_timetostr(Histo.live_time, tstr);
    fprintf( n42, "\t\t\t<LiveTimeDuration>%s</LiveTimeDuration>\n", tstr);
    fprintf( n42, "\t\t\t<ChannelData compressionCode=\"None\">\n" ); 
	for (i=0; i<(int)Histo.Nbin; i++)
		fprintf(n42, "%d\n", Histo.H_data[i]);
	fprintf( n42, "\t\t\t</ChannelData>\n");
    fprintf( n42, "\t\t</Spectrum>\n");
	fprintf( n42, "\t</RadMeasurement>\n");
	fprintf( n42, "</RadInstrumentData>\n");
	fclose(n42);
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Save an histogram to output file
// Inputs:		FileName = filename 
//				Histo = histogram to save
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveHistogram(char *FileName, Histogram1D_t Histo, char *SpectrumName)
{
    FILE *fh;
    uint16_t i;

	if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_ANSI42) {
		return SaveSpectrum_n42(FileName, Histo, SpectrumName);
	} else {
		fh = fopen(FileName, "w");
		if (fh == NULL)
			return -1;
		if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_1COL) {
			for(i=0; i<Histo.Nbin; i++) 
				fprintf(fh, "%d\n", Histo.H_data[i]);
		} else if (WDcfg.HistoOutputFormat == HISTO_FILE_FORMAT_2COL) {
			for(i=0; i<Histo.Nbin; i++) 
				fprintf(fh, "%d %d\n", i, Histo.H_data[i]);
		}
	}
    fclose(fh);
    return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Save all histograms to output file
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveAllHistograms() 
{
	int b, ch, ret=0;
	char fname[300], sname[300];

	/* Save Histograms to file for each board/channel */
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch]) {
				if (WDcfg.SaveHistograms & 0x1) {
					Histos.EH[b][ch].real_time = Stats.AcqRealTime;
					Histos.EH[b][ch].live_time = Stats.AcqRealTime * (1 - Stats.DeadTime[b][ch]);
					sprintf(sname, "Energy_Spectrum_%d_%d", b, ch);
					CreateOutputFileName(OUTPUTFILE_TYPE_EHISTO, b, ch, fname);
					ret |= SaveHistogram(fname, Histos.EH[b][ch], sname);
				}
				if (WDcfg.SaveHistograms & 0x2) {
					sprintf(sname, "Timing_Spectrum_%d_%d", b, ch);
					CreateOutputFileName(OUTPUTFILE_TYPE_THISTO, b, ch, fname);
					ret |= SaveHistogram(fname, Histos.TH[b][ch], sname);
				}
				if (WDcfg.SaveHistograms & 0x4) {
					sprintf(sname, "PSD_Spectrum_%d_%d", b, ch);
					CreateOutputFileName(OUTPUTFILE_TYPE_PSDHISTO, b, ch, fname);
					ret |= SaveHistogram(fname, Histos.PSDH[b][ch], sname);
				}
			}
		}
	}
	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Save one event to the List file
// Inputs:		b = board index
//				ch = channel
//              evnt = event to save
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveList(int b, int ch, GenericDPPEvent_t evnt) 
{
	uint64_t tstamp = evnt.TimeStamp * 1000 + evnt.FineTimeStamp;
	uint16_t flags = WDcfg.FlagsInListFiles & evnt.Flags;
	char fname[100], str[1000], tstr[100];

	if (list[b][ch] == NULL) {
		CreateOutputFileName(OUTPUTFILE_TYPE_LIST, b, ch, fname);
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) 	list[b][ch] = fopen(fname, "w");
		else 										list[b][ch] = fopen(fname, "wb");
		if (list[b][ch] == NULL) return -1;
		if (WDcfg.HeaderInListFiles) {
			fprintf(list[b][ch], "# Header Format %08X\n", 0x00030007 | (WDcfg.OutFileTimeStampUnit << 12) | (WDcfg.FlagsInListFiles << 3));
			fprintf(list[b][ch], "# Board %2d - Channel %2d\n", b, ch);
			fprintf(list[b][ch], "#      Tstamp_%s   Energy    PSD", tu[WDcfg.OutFileTimeStampUnit]);
			if (WDcfg.FlagsInListFiles) fprintf(list[b][ch], "    Flags\n");
			else fprintf(list[b][ch], "\n");
		}
	}

	TimeString(tstamp, tstr);
	sprintf(str, "%s %8d %6.4f",  tstr, evnt.Energy, evnt.psd);
	if (flags) sprintf(str, "%s %04X", str, flags);

	if (ftell(list[b][ch]) < (uint64_t)SysVars.MaxOutputFileDataSize*1024*1024) {
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) {
			fprintf(list[b][ch], "%s\n", str);
		} else {
			fwrite(&tstamp, 1, sizeof(tstamp), list[b][ch]);
			fwrite(&evnt.Energy, 1, sizeof(evnt.Energy), list[b][ch]);
			fwrite(&evnt.psd, 1, sizeof(evnt.psd), list[b][ch]);
			if (flags) fwrite(&flags, 1, sizeof(flags), list[b][ch]);
		}
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Save one event to the Merged List file
// Inputs:		b = board index
//				ch = channel
//              evnt = event to save
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveMergedList(int b, int ch, GenericDPPEvent_t evnt) 
{
	uint64_t tstamp = evnt.TimeStamp * 1000 + evnt.FineTimeStamp;
	uint16_t flags = WDcfg.FlagsInListFiles & evnt.Flags;
	char fname[100], str[1000], tstr[100];

	if ((WDcfg.SaveLists & SAVELIST_MERGED) && (list_merged == NULL)) {
		CreateOutputFileName(OUTPUTFILE_TYPE_LIST_MERGED, 0, 0, fname);
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) 	list_merged = fopen(fname, "w");
		else 										list_merged = fopen(fname, "wb");
		if (list_merged == NULL) return -1;
		if (WDcfg.HeaderInListFiles) {
			fprintf(list_merged, "# Header Format %08X\n", 0x10030007 | (WDcfg.OutFileTimeStampUnit << 12) | (WDcfg.FlagsInListFiles << 3));
			fprintf(list_merged, "# Board %2d - Channel %2d\n", b, ch);
			fprintf(list_merged, "# Bd  Ch         Tstamp_%s   Energy    PSD", tu[WDcfg.OutFileTimeStampUnit]);
			if (WDcfg.FlagsInListFiles) fprintf(list_merged, "    Flags\n");
			else fprintf(list_merged, "\n");
		}
	}

	TimeString(tstamp, tstr);
	sprintf(str, "[%02d] [%02d] %s %8d %6.4f",  b, ch, tstr, evnt.Energy, evnt.psd);
	if (flags) sprintf(str, "%s %04X", str, flags);
	
	if (ftell(list_merged) < (SysVars.MaxOutputFileDataSize*1024*1024)) {
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) {
			fprintf(list_merged, "%s\n", str);
		} else {
			uint8_t b8 = b, ch8 = ch;
			fwrite(&b8, 1, sizeof(b8), list_merged);
			fwrite(&ch8, 1, sizeof(ch8), list_merged);
			fwrite(&tstamp, 1, sizeof(tstamp), list[b][ch]);
			fwrite(&evnt.Energy, 1, sizeof(evnt.Energy), list[b][ch]);
			fwrite(&evnt.psd, 1, sizeof(evnt.psd), list[b][ch]);
		}
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Save built events to the relevant List file
// Inputs:		evnt = built events
//				NumEv = number of events per channel
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveBuiltEventsList(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int NumEv[MAX_NBRD][MAX_NCH])
{
	char fname[100], tstr[100];
	int b, ch, br, cr, cc;
	uint64_t Tref;

	if ((!(WDcfg.SaveLists & SAVELIST_BUILTEVENTS)) || (WDcfg.BuildMode == EVBUILD_NONE)) return -1;

	switch(WDcfg.BuildMode) {
	// ----------------------------------------
	case EVBUILD_CHREF_AND_ANYOTHER:
		br = WDcfg.TOFstartBoard;
		cr = WDcfg.TOFstartChannel;
		if (blist[br][cr] == NULL) {
			CreateOutputFileName(OUTPUTFILE_TYPE_LIST_BUILT_EV, br, cr, fname);
			if (WDcfg.OutFileFormat == OUTFILE_ASCII) 	blist[br][cr] = fopen(fname, "w");
			else 										blist[br][cr] = fopen(fname, "wb");
			if (blist[br][cr] == NULL)	return -1;
			if (WDcfg.HeaderInListFiles) {
				fprintf(blist[br][cr], "# Header Format %08X\n", 0x20020003 | (WDcfg.OutFileTimeStampUnit << 12));
				fprintf(blist[br][cr], "# Ref Ch: Board %2d - Channel %2d\n", br, cr);
				fprintf(blist[br][cr], "# [Bref][Cref] Tstamp_%s Energy : [Bx][Cx] deltaT_ns Energy ... \n", tu[WDcfg.OutFileTimeStampUnit]);
			}
		}
		if (ftell(blist[br][cr]) > (SysVars.MaxOutputFileDataSize*1024*1024)) return 0;

		Tref = evnt[br][cr].TimeStamp * 1000 + evnt[br][cr].FineTimeStamp;  // in ps
		TimeString(Tref, tstr);
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) {
			fprintf(blist[br][cr], "[%02d][%02d] %s %8d : ", br, cr, tstr, evnt[br][cr].Energy);
		} else {
			fwrite(&br, 1, sizeof(char), blist[br][cr]);
			fwrite(&cr, 1, sizeof(char), blist[br][cr]);
			fwrite(&Tref, 1, sizeof(Tref), blist[br][cr]);
			fwrite(&evnt[br][cr].Energy, 1, sizeof(uint16_t), blist[br][cr]);
		}

		for(b=0; b<WDcfg.NumBrd; b++) {
			for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
				if ((NumEv[b][ch] == 0) || ((b == br) && (ch == cr))) continue;
				double deltaT = (double)evnt[b][ch].TimeStamp + 0.001*evnt[b][ch].FineTimeStamp - (double)Tref/1000;  // in ns
				if (WDcfg.OutFileFormat == OUTFILE_ASCII) {
					fprintf(blist[br][cr], "[%02d][%02d] %10.3f %8d ", b, ch, deltaT, evnt[b][ch].Energy);
				} else {
					fwrite(&b, 1, sizeof(char), blist[br][cr]);
					fwrite(&ch, 1, sizeof(char), blist[br][cr]);
					fwrite(&deltaT, 1, sizeof(deltaT), blist[br][cr]);
					fwrite(&evnt[b][ch].Energy, 1, sizeof(uint16_t), blist[br][cr]);
				}
			}
		}
		if (WDcfg.OutFileFormat == OUTFILE_ASCII) fprintf(blist[br][cr], "\n");
		break;

	// ----------------------------------------
	case EVBUILD_CLOVER:
		for(b=0; b<WDcfg.NumBrd; b++) {
			for(ch=0; ch<WDcfg.NumPhyCh; ch+=WDcfg.CloverNch) {
				uint64_t tm=0;
				int ncc=0, vch, i;
				for(cc=ch; cc < (ch+WDcfg.CloverNch); cc++) {
					if (NumEv[b][cc] > 0) {
						tm += evnt[b][cc].TimeStamp * 1000 + evnt[b][cc].TimeStamp;
						ncc++;
					}
				}
				if (ncc == 0) continue; // no data for this clover
				if (blist[b][ch] == NULL) {
					CreateOutputFileName(OUTPUTFILE_TYPE_LIST_BUILT_EV, b, ch, fname);
					if (WDcfg.OutFileFormat == OUTFILE_ASCII) 	blist[b][ch] = fopen(fname, "w");
					else 										blist[b][ch] = fopen(fname, "wb");
					if (blist[b][ch] == NULL) return -1;
					if (WDcfg.HeaderInListFiles) {
						fprintf(blist[b][ch], "# Header Format %08X\n", 0x21030003 | (WDcfg.OutFileTimeStampUnit << 12));
						fprintf(blist[b][ch], "# Board %d Channels from %d to %d\n", b, ch, ch + WDcfg.CloverNch - 1);
						fprintf(blist[b][ch], "#      Tstamp_%s", tu[WDcfg.OutFileTimeStampUnit]);
						for(i=0; i<WDcfg.CloverNch; i++)
							fprintf(blist[b][ch], "  Clv[%d]", i);
						fprintf(blist[b][ch], " AddBack  Flags\n");
					}
				}
				if (ftell(blist[b][ch]) > (SysVars.MaxOutputFileDataSize*1024*1024)) return 0;
				
				TimeString(tm/ncc, tstr);
				if (WDcfg.OutFileFormat == OUTFILE_ASCII)
					fprintf(blist[b][ch], "%s", tstr);
				else 
					fwrite(&tm, 1, sizeof(tm), blist[b][ch]);
				for(cc=ch; cc < (ch+WDcfg.CloverNch); cc++) {
					if (WDcfg.OutFileFormat == OUTFILE_ASCII) {
						if (NumEv[b][cc] > 0)
							fprintf(blist[b][ch], "%8d", evnt[b][cc].Energy);
						else
							fprintf(blist[b][ch], "       -");
					} else {
						uint16_t energy = (NumEv[b][cc] > 0) ? evnt[b][cc].Energy : 0;
						fwrite(&energy, 1, sizeof(energy), blist[b][cc]);
					}
				}
				vch = WDcfg.NumPhyCh + ch/WDcfg.CloverNch;
				if (vch < WDcfg.NumAcqCh) {
					fprintf(blist[b][ch], "%8d", evnt[b][vch].Energy);
					fprintf(blist[b][ch], "   %04X", evnt[b][vch].Flags);
				}
				if (WDcfg.OutFileFormat == OUTFILE_ASCII) fprintf(blist[b][ch], "\n");
			}
		}
		break;
	// ----------------------------------------
	default: break;
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Save a waveform (analog trace 0) to the Waveform file
// Inputs:		b = board index
//				ch = channel
//				evnt = event data
//				wfm = waveform data
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveWaveform(int b, int ch, GenericDPPEvent_t evnt) 
{
	int i;
	char fname[500], tstr[100];
	Waveform_t *wfm = evnt.Waveforms;
	uint64_t tstamp = evnt.TimeStamp*1000 + evnt.FineTimeStamp;


	if (wavefile[b][ch] == NULL) {
		CreateOutputFileName(OUTPUTFILE_TYPE_WAVE, b, ch, fname);
		if (WDcfg.OutFileFormat == OUTFILE_BINARY)	wavefile[b][ch] = fopen(fname, "wb");
		else										wavefile[b][ch] = fopen(fname, "w");
	}
	if (wavefile[b][ch] == NULL)
		return -1;
	if (ftell(wavefile[b][ch]) < (SysVars.MaxOutputFileDataSize*1024*1024)) {
		if (WDcfg.OutFileFormat == OUTFILE_BINARY) {
			fwrite(&evnt.TimeStamp, sizeof(evnt.TimeStamp), 1, wavefile[b][ch]);
			fwrite(&evnt.FineTimeStamp, sizeof(evnt.FineTimeStamp), 1, wavefile[b][ch]);
			fwrite(&evnt.Energy, sizeof(evnt.Energy), 1, wavefile[b][ch]);
			fwrite(&wfm->Ns, sizeof(wfm->Ns), 1, wavefile[b][ch]);
			fwrite(wfm->AnalogTrace[0], sizeof(uint16_t), wfm->Ns, wavefile[b][ch]);
		} else {
			TimeString(tstamp, tstr);
			fprintf(wavefile[b][ch], "%s %5d %6d  ", tstr, evnt.Energy, wfm->Ns);
			for(i=0; i<wfm->Ns; i++)
				fprintf(wavefile[b][ch], "%d ", (int16_t)(wfm->AnalogTrace[0][i]));
			fprintf(wavefile[b][ch], "\n");
		}
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Save run info
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveRunInfo()
{
	char fname[500], line[500];
	int b, ch;
	FILE *cfg;
	CAEN_DGTZ_BoardInfo_t BoardInfo;

	CreateOutputFileName(OUTPUTFILE_TYPE_RUN_INFO, 0, 0, fname);
	rinf = fopen(fname, "w");

	fprintf(rinf, "-----------------------------------------------------------------\n");
	if (strlen(WDcfg.RunDescription) > 0) fprintf(rinf, "Run n. %d : %s\n", WDcfg.RunNumber, WDcfg.RunDescription);
	else fprintf(rinf, "Run n. %d : No run description available\n", WDcfg.RunNumber);
	if (WDcfg.JobStopRun > 0) fprintf(rinf, "Job %d of %d\n", WDcfg.RunNumber - WDcfg.JobStartRun, WDcfg.JobStopRun - WDcfg.JobStartRun);
	fprintf(rinf, "-----------------------------------------------------------------\n\n");

	fprintf(rinf, "-----------------------------------------------------------------\n");
	fprintf(rinf, "Boards\n");
	fprintf(rinf, "-----------------------------------------------------------------\n");
	for(b=0; b<WDcfg.NumBrd; b++) {
		if (CAEN_DGTZ_GetInfo(handle[b], &BoardInfo) != CAEN_DGTZ_Success) continue;
		fprintf(rinf, "Board %d:\n", b);
		fprintf(rinf, " CAEN Digitizer Model %s (s.n. %d)\n", BoardInfo.ModelName, BoardInfo.SerialNumber);
		fprintf(rinf, " ROC FPGA: %s\n", BoardInfo.ROC_FirmwareRel);
		fprintf(rinf, " AMC FPGA: %s (%s)\n", BoardInfo.AMC_FirmwareRel, WDcfg.FwTypeString);
	}

	fprintf(rinf, "\n\n-----------------------------------------------------------------\n");
	fprintf(rinf, "Statistics\n");
	fprintf(rinf, "-----------------------------------------------------------------\n");
	fprintf(rinf, "Acquisition started at %s\n", Stats.AcqStartTimeString);
	fprintf(rinf, "Acquisition stopped after %.2f s (RealTime)\n", Stats.AcqStopTime/1000);
	fprintf(rinf, "Total processed events = %ld\n", Stats.TotEvRead_cnt);
	fprintf(rinf, "Total bytes = %.4f MB\n", (float)Stats.RxByte_cnt/(1024*1024));
	for(b=0; b<WDcfg.NumBrd; b++) {
		fprintf(rinf, "Board %2d : LastTstamp(s)   NumEvents      Rate(KHz)\n", b);
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			float rate = (Stats.LatestProcTstamp[b][ch] > 0) ? (float)Stats.EvProcessed_cnt[b][ch]/(float)(Stats.LatestProcTstamp[b][ch]/1e6) : 0;
			fprintf(rinf, "   Ch %2d:  %10.2f   %12lu  %12.4f\n", ch, (float)Stats.LatestProcTstamp[b][ch]/1e9, Stats.EvProcessed_cnt[b][ch], rate);
		}
	}


	fprintf(rinf, "\n\n-----------------------------------------------------------------\n");
	fprintf(rinf, "Configuration File\n");
	fprintf(rinf, "-----------------------------------------------------------------\n");
	cfg = fopen("_cfg.txt", "r");
	if (cfg != NULL) {
		while(!feof(cfg)) {
			fgets(line, 500, cfg);
			fputs(line, rinf);
		}
	}
	fclose(rinf);
	return 0;
}
