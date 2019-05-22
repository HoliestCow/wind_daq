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

#ifndef _DIGITES_H
#define _DIGITES_H                    // Protect against multiple inclusion


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CAENDigitizer.h"

#ifdef WIN32
	#define popen _popen
	#define DEFAULT_GNUPLOT_PATH	"pgnuplot.exe"
	#define PATH_SEPARATOR "\\"
	#define WAIT_ON_EXIT	10000  // 10 sec before closing the console (only in case of error)
	#define CONFIG_FILE_PATH ""
	#define DATA_FILE_PATH ".\\DataFiles\\"
	#define WORKING_DIR ""
	#define EDIT_CFG_FILE "start notepad digiTES_Config.txt"
#else
    #include <unistd.h>
    #include <stdint.h>   /* C99 compliant compilers: uint64_t */
    #include <ctype.h>    /* toupper() */
    #include <sys/time.h>
	#define DEFAULT_GNUPLOT_PATH	"gnuplot"
	#define PATH_SEPARATOR "/"
	#define EDIT_CFG_FILE "gedit digiTES_Config.txt &"
	#ifndef Sleep
		#define Sleep(x) usleep((x)*1000)
	#endif
	#define WAIT_ON_EXIT	0
	#ifdef _ROOT_
		#define CONFIG_FILE_PATH _ROOT_"/digiTES/config/"
		#define DATA_FILE_PATH _ROOT_"/digiTES/data/"
		#define WORKING_DIR _ROOT_"/digiTES/"
	#else
		#define CONFIG_FILE_PATH ""
		#define DATA_FILE_PATH "./DataFiles"
		#define WORKING_DIR ""
	#endif
#endif


#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min 
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CH2KEV
#define CH2KEV(e, c0, c1, c2, c3)   ((e)*(e)*(e)*(c3) + (e)*(e)*(c2) + (e)*(c1) + (c0))
#endif

//****************************************************************************
// Log Files (for debugging)
//****************************************************************************
#define ENROLOG						0  // Enanble Readout log file
#define ENQLOG						0  // Enanble Queue log file


//****************************************************************************
// Definition of limits and sizes
//****************************************************************************
#define MAX_NBRD					8	// max. number of boards 
#define MAX_NCH						32	// max. number of channels (physical+virtual)

#define EMAXNBITS					(1<<14)	// Max num of bits for the Energy histograms 
#define TMAXNBITS					(1<<14)	// Max num of bits for the Time histograms 

#define HISTO2D_NBINX				512		// Num of bins in the X axes of the scatter plot
#define HISTO2D_NBINY				512		// Num of bins in the y axes of the scatter plot

#define MAX_GW						20		// max. number of generic write commads

#define MAX_NTRACES					6		// max. number of traces in the waveform plot 
#define MAX_NTRSETS					16		// max. number of trace settings (probes) 

#define RTSET_VERSION				2

#define MAX_RUN_DESCR_LENGTH		200

#define MAX_NUM_ROI					20

#define VIRTUAL_BOARD_TYPE          99

//****************************************************************************
// Data File options 
//****************************************************************************
#define DATA_FILE_FORMAT			0		// File format (must be changed when data file format or structures size change)


//****************************************************************************
// Event data Flags
//****************************************************************************
#define EVFLAGS_DEADTIME			0x0001	// dead time (memory full or input saturation) occurred between this event and the previous one
#define EVFLAGS_SATUR				0x0002	// input voltage saturation (clipping)  HACK: can indicate also energy overrange?
#define EVFLAGS_FAKE				0x0008	// fake event (e.g. to track the tstamp rollover)
#define EVFLAGS_PILEUP				0x0010	// pile-up (energy not good)
#define EVFLAGS_TTRESET             0x0040	// Time stamp cleared from SIN/GPI  
#define EVFLAGS_E_OVR               0x0100	// Energy Overrange
#define EVFLAGS_E_UNR               0x0200	// Energy Underrange
#define EVFLAGS_VETOED              0x0400	// Vetoed Event (tagged instead of discarded)
#define EVFLAGS_UNCORREL            0x0800	// Uncorrelated Event (tagged instead of discarded)

//****************************************************************************
// Definition of some options
//****************************************************************************
// Firmware Type
#define DPP_CI						0x0000
#define DPP_PSD_720					0x0001
#define DPP_PSD_751					0x0002
#define DPP_PSD_730					0x0003
#define DPP_PHA_724					0x0004
#define DPP_PHA_730					0x0005
#define DPP_nPHA_724				0x0006
#define STD_730						0x1000

#define IS_STD_FW(fw)				(fw & 0x1000)
#define IS_DPP_FW(fw)				(!(fw & 0x1000))

#define WPLOT_DISABLED				0
#define WPLOT_WAVEFORMS				1
#define WPLOT_FFT					2
#define WPLOT_HISTOGRAM				3

#define HPLOT_DISABLED				0
#define HPLOT_ENERGY				1
#define HPLOT_TIME					2
#define HPLOT_PSD					3
#define HPLOT_MCS					4
#define HPLOT_ENERGYvsPSD			5
#define HPLOT_SAMPLES				6

#define TAC_SPECTRUM_COMMON_START	0
#define TAC_SPECTRUM_INTERVALS		1

#define SCATTER_PSD_HORIZONTAL		0
#define SCATTER_PSD_DIAGONAL		1
#define SCATTER_E_VS_DELTAE			2

#define ACQ_MODE_LIST				1
#define ACQ_MODE_MIXED				2
#define ACQ_MODE_OFFLINE			3
#define ACQ_MODE_EMULATOR_MIXED		4
#define ACQ_MODE_EMULATOR_LIST		5
#define ACQ_MODE_PLUGGED(x)			((x == ACQ_MODE_LIST) || (x == ACQ_MODE_MIXED))
#define ACQ_MODE_UNPLUGGED(x)		((x == ACQ_MODE_OFFLINE) || (x == ACQ_MODE_EMULATOR_LIST) || (x == ACQ_MODE_EMULATOR_MIXED))

#define START_MODE_INDEP_SW			0
#define START_MODE_SYNCIN_1ST_SW	1
#define START_MODE_SYNCIN_1ST_HW	2
#define START_MODE_TRGIN_1ST_SW		3
#define START_MODE_TRGIN_1ST_HW		4

#define TRGIN_MODE_DISABLED			0
#define TRGIN_MODE_GLOBAL_TRG		1
#define TRGIN_MODE_VETO				2
#define TRGIN_MODE_GATE				3
#define TRGIN_MODE_COINC			4

#define SYNCIN_MODE_DISABLED		0
#define SYNCIN_MODE_TSTAMP_RESET	1
#define SYNCIN_MODE_RUN_CTRL		2

#define TRGOUT_MODE_DISABLED		0
#define TRGOUT_MODE_CH_TRG			1
#define TRGOUT_MODE_PROP_TRGIN      2
#define TRGOUT_MODE_SYNC_OUT		3
#define TRGOUT_MODE_SQR_1KHZ		4
#define TRGOUT_MODE_PLS_1KHZ		5
#define TRGOUT_MODE_SQR_10KHZ		6
#define TRGOUT_MODE_PLS_10KHZ		7
#define TRGOUT_CLOCK				8
#define TRGOUT_SIGSCOPE				9

#define DISCR_MODE_DISABLED			-1 // self trigger disabled
#define DISCR_MODE_LED_PSD			0  // default for PSD
#define DISCR_MODE_RCCR2_PHA		0  // default for PHA
#define DISCR_MODE_CFD_PSD			1
#define DISCR_MODE_CFD_PHA			2
#define DISCR_MODE_LED_PHA			3

// options for HW coincidences/anticoincidences and trigger propagation
#define COINC_DISABLED				0x00  // no hardware trigger logic is applied (acquire singles)
#define COINC_MAJORITY				0x01  // acquire when the num of fired channels >= Majority Threshold
#define COINC_PAIRED_AND			0x02  // both channels of a couple acquire when both are fired
#define COINC_CH0_AND_ANY			0x03  // coincidence between a common ref channel (typ. ch 0) and any other channel
#define COINC_AND_ALL				0x04  // acquire when all enabled channels are fired
#define COINC_MINORITY	 			0x81  // acquire when the num of fired channels < Majority Threshold
#define COINC_OR_ALL 				0x0A  // all enabled channels acquire if any channel is fired (trigger propagation from any channel to all)
#define COINC_CH0_TO_ALL		 	0x0B  // all enabled channels acquire if channel 0 is fired (trigger propagation from channel 0 to all)
#define COINC_PAIRED_OR		 		0x0C  // both channels of a couple acquire when one at least on of the two is fired (trigger propagation in the couple)
#define COINC_PAIRED_NAND			0x82  // one channel of a couple acquires when the other one is not fired (anti coincidence)
#define COINC_CH0_NAND_ANY 			0x83  // any channel acquires is there is no coincidence with the ref. channel

// options for the event building
#define EVBUILD_NONE				0	// No correlation
#define EVBUILD_CHREF_AND_ANYOTHER	1	// One reference channel in coincidence with any N channels, N>=1
#define EVBUILD_PAIRED_AND			2	// Couples with coincidence between ch N and ch N+1
#define EVBUILD_CLOVER				3	// at least one channel between N (typ N=4); no more than 1 event per channel


#define HISTO_FILE_FORMAT_1COL		0  // ascii 1 coloumn
#define HISTO_FILE_FORMAT_2COL		1  // ascii 1 coloumn
#define HISTO_FILE_FORMAT_ANSI42	2  // xml ANSI42

#define SAVELIST_INDIVIDUAL			0x01
#define SAVELIST_MERGED				0x02
#define SAVELIST_BUILTEVENTS		0x04

#define OUTFILE_BINARY				0
#define OUTFILE_ASCII				1

#define INPUTFILE_RAW				0
#define INPUTFILE_BINARY_LIST		1
#define INPUTFILE_ASCII_LIST		2

#define REALTIME_FROM_BOARDS		0
#define REALTIME_FROM_COMPUTER		1


//****************************************************************************
// Definition of Start of Run Modes
//****************************************************************************
// start on software command 
#define RUN_START_ON_SOFTWARE_COMMAND     0xC 
// start on S-IN level (logical high = run; logical low = stop)
#define RUN_START_ON_SIN_LEVEL            0xD
// start on first TRG-IN or Software Trigger 
#define RUN_START_ON_TRGIN_RISING_EDGE    0xE
// start on LVDS I/O level
#define RUN_START_ON_LVDS_IO              0xF


//****************************************************************************
// Waveform Data Structure
//****************************************************************************
typedef struct
{
    int32_t Ns;						// Num of samples
    uint16_t *AnalogTrace[2];		// Analog traces (samples); the 2nd trace is enabled if DualTrace=1
    uint8_t  *DigitalTraces;		// Digital traces (1 bit = 1 trace)
	int8_t DualTrace;				// Dual Analog Traces
	int8_t TraceSet[MAX_NTRACES];	// Traces Setting
} Waveform_t;

//****************************************************************************
// Generic Event Data Structure 
//****************************************************************************
typedef struct
{
	uint64_t TimeStamp;			// 64 bit coarse time stamp (in ns)
	uint16_t FineTimeStamp;		// Fine time stamp (interpolation) in ps
	uint16_t Energy;			// Energy (charge for DPP_PSD/CI or pulse height for DPP_PHA)
	float psd;					// Pulse Shape Discrimination (PSD = (Qlong-Qshort)/Qlong). Only for DPP_PSD. Range 0 to 1023.
	Waveform_t *Waveforms;		// Pointer to waveform data (NULL if the waveform mode is not enabled)
	uint16_t Flags;				// Event Flags (e.g. pile-up, saturation, etc...)
} GenericDPPEvent_t;


//****************************************************************************
// Histogram Data Structure (1 dimension)
//****************************************************************************
typedef struct
{
	uint32_t *H_data;			// pointer to the histogram data
	uint32_t Nbin;				// Number of bins (channels) in the histogram
	uint32_t H_cnt;				// Number of entries (sum of the counts in each bin)
	uint32_t Ovf_cnt;			// Overflow counter
	uint32_t Unf_cnt;			// Underflow counter
	double rms;					// rms
	double mean;				// mean
	double real_time;			// real time in s
	double live_time;			// live time in s
	double A[4];				// calibration coefficients
	int n_ROI;					// number of ROIs
	int ROIbegin[MAX_NUM_ROI];	// first channel of the ROI
	int ROIend[MAX_NUM_ROI];	// last channel of the ROI
} Histogram1D_t;

//****************************************************************************
// Histogram Data Structure (2 dimensions)
//****************************************************************************
typedef struct
{
	uint32_t *H_data;			// pointer to the histogram data
	uint32_t NbinX;				// Number of bins (channels) in the X axis
	uint32_t NbinY;				// Number of bins (channels) in the Y axis
	uint32_t H_cnt;				// Number of entries (sum of the counts in each bin)
	uint32_t Ovf_cnt;			// Overflow counter
	uint32_t Unf_cnt;			// Underflow counter
} Histogram2D_t;

//****************************************************************************
// Struct containing the histograms (Energy, Time, PSD, etc...)
//****************************************************************************
typedef struct
{
	Histogram1D_t EH[MAX_NBRD][MAX_NCH];		// Energy Histograms 
	Histogram1D_t TH[MAX_NBRD][MAX_NCH];		// Time Histograms 
	Histogram1D_t SH[MAX_NBRD][MAX_NCH];		// Sample Histograms 
	Histogram1D_t MCSH[MAX_NBRD][MAX_NCH];		// MCS Histograms 
	Histogram1D_t PSDH[MAX_NBRD][MAX_NCH];		// PSD Histograms 
	Histogram2D_t PSDvsE[MAX_NBRD][MAX_NCH];	// PSD vs Energy 2D Histograms 
} Histos_t;



//****************************************************************************
// digiTES System Variables (not defined by the user)
//****************************************************************************
typedef struct
{
	int CheckHeaderErrorBit;		// check error bit in the data header
	int ImmediateStart;				// Start acquisition without waitng for 's' pressed
	int AutoReloadPlotsettings;		// Automatic re-load settings for plots (channel, traces, etc...)
	int AutoRestartOnCfgChange;		// Automatic restart when the config file is changed
	int UseRollOverFakeEvents;		// Use roll-over fake events to extend the time stamp
	int FineTstampMode;				// 0=disabled, 1=on-board interpolation, 2=ZCpos/ZCneg
	int HistoAutoSave;				// Autosave time (in seconds) for the histogram; 0=save at end of run only
	int MaxOutputFileDataSize;		// Maximum size (in MB) of the putput data files
	int InactiveTimeout;			// Time for a silent channel to be declared inactive (excluded from correlation filters)
	int ZCcorr_Ncnt;				// Num of counts acquired for the ZC correction
	char ZCcalibrFileName[500];		// ZC calibration file name
	int HVmax;						// Maximum High Voltage
	char GnuplotCmd[500];			// path and command to open gnuplot
} SysVars_t;


//****************************************************************************
// Struct containing variables for the statistics (counters, times, etc...)
//****************************************************************************
typedef struct
{
	// ----------------------------------
	// Counters and Rates
	// ----------------------------------
	// Each counter has two instances: total counter and previous value used to calculate the rate: Rate = (Counter - PrevCounter)/elapsed_time
	// after the rate calculation, PrevCounter is updated to the value of Counter
	uint64_t RxByte_cnt;						// Received Byte counter (data from board)
	uint64_t RxByte_pcnt;						// Previous value
	float RxByte_rate;							// Data Throughput in KB/s (from boards or from files)
	uint64_t BlockRead_cnt;						// BlockRead counter

	uint64_t EvRead_cnt[MAX_NBRD][MAX_NCH];		// Event Counter at board output (throughput); number of event read from the channel
	uint64_t EvRead_pcnt[MAX_NBRD][MAX_NCH];	
	uint64_t EvRead_dcnt[MAX_NBRD][MAX_NCH];	// cnt - pcnt 
	float EvRead_rate[MAX_NBRD][MAX_NCH];	
	uint64_t EvProcessed_cnt[MAX_NBRD][MAX_NCH];// Event Processed (extracted from the queues). It is almost the same as EvRead_cnt but delayed (it doesn't count the events still in the queues)
	uint64_t EvProcessed_pcnt[MAX_NBRD][MAX_NCH];
	//float EvProcessed_rate[MAX_NBRD][MAX_NCH];
	uint64_t EvInput_cnt[MAX_NBRD][MAX_NCH];	// Event Counter at board input (ICR): number of event detected by the input discriminator
	uint64_t EvInput_pcnt[MAX_NBRD][MAX_NCH];	// Note: ICR may differ from ICRB because of the inability of the discriminator to distinguish two close pulses
	float EvInput_rate[MAX_NBRD][MAX_NCH];	
	uint64_t EvFilt_cnt[MAX_NBRD][MAX_NCH];		// Event Counter after the software filters (cuts & correlation; PUR is counted separately)
	uint64_t EvFilt_pcnt[MAX_NBRD][MAX_NCH];	
	float EvFilt_rate[MAX_NBRD][MAX_NCH];
	uint64_t EvPileUp_cnt[MAX_NBRD][MAX_NCH];	// Counter of the events tagged as pile-up;
	uint64_t EvPileUp_pcnt[MAX_NBRD][MAX_NCH];	
	float EvPileUp_rate[MAX_NBRD][MAX_NCH];	
	uint64_t EvOvf_cnt[MAX_NBRD][MAX_NCH];		// Counter of the events tagged as overflow;
	uint64_t EvOvf_pcnt[MAX_NBRD][MAX_NCH];	
	float EvOvf_rate[MAX_NBRD][MAX_NCH];	
	uint64_t EvUncorrel_cnt[MAX_NBRD][MAX_NCH];	// Counter of the events tagged as "uncorrelated" (either vetoed or not matching the coicidence criteria)
	uint64_t EvUncorrel_pcnt[MAX_NBRD][MAX_NCH];// NOTE: typically these events are discarded by the board; in some FW, there is an option to save and tag them)
	float EvUncorrel_rate[MAX_NBRD][MAX_NCH];	
	uint64_t EvLost_cnt[MAX_NBRD][MAX_NCH];		// Counter of the events lost (either in the HW or in the SW queues)
	uint64_t EvLost_pcnt[MAX_NBRD][MAX_NCH];	
	float EvLost_rate[MAX_NBRD][MAX_NCH];	
	uint64_t Satur_cnt[MAX_NBRD][MAX_NCH];		// Counter of the saturated events
	uint64_t Satur_pcnt[MAX_NBRD][MAX_NCH];	
	float Satur_rate[MAX_NBRD][MAX_NCH];
	float DeadTime[MAX_NBRD][MAX_NCH];			// Dead Time (float number in the range 0 to 1)
	float MatchingRatio[MAX_NBRD][MAX_NCH];		// Matching Ratio after the filters (cuts & correlation)
	float EvOutput_rate[MAX_NBRD][MAX_NCH];		// OCR

	uint64_t BusyTimeGap[MAX_NBRD][MAX_NCH];	// Sum of the DeadTime Gaps (saturation or busy); this is a real dead time (loss of triggers); it doesn't include dead time for pile-ups
	float BusyTime[MAX_NBRD][MAX_NCH];			// Percent of BusyTimeGap 
	
	uint64_t TotEvRead_cnt;						// Total Event read from the boards (sum of all channels)

	// Times
	uint64_t StartTime;							// Computer time at the start of the acquisition in ms
	uint64_t LastUpdateTime;					// Computer time at the last statistics update
	float AcqRealTime;							// Acquisition time (from the start) in ms
	float AcqStopTime;							// Acquisition Stop time (from the start) in ms
	int RealTimeSource;							// 0: real time from the time stamps; 1: real time from the computer
	char AcqStartTimeString[100];				// Start Time in the format %Y-%m-%d %H:%M:%S

	uint64_t LatestProcTstampAll;					// Latest event time stamp (= acquisition time taken from the boards) in ns
	uint64_t PrevProcTstampAll;						// Previous event time stamp (used to calculate the elapsed time from the last update) in ns
	uint64_t LatestReadTstamp[MAX_NBRD][MAX_NCH];	// Newest time stamp in ns at queue input
	uint64_t PrevReadTstamp[MAX_NBRD][MAX_NCH];		// Previous value of LatestReadTstamp (used to calculate elapsed time)
	uint64_t LatestProcTstamp[MAX_NBRD][MAX_NCH];	// Newest time stamp in ns at queue output
	uint64_t PrevProcTstamp[MAX_NBRD][MAX_NCH];		// Previous value of LatestProcTstamp (used to calculate elapsed time)
	uint64_t ICRUpdateTime[MAX_NBRD][MAX_NCH];		// Time stamp of the event containg ICR information (1K flag)
	uint64_t PrevICRUpdateTime[MAX_NBRD][MAX_NCH];	// Previous value of ICRUpdateTime (used to calculate elapsed time for ICR)
	uint64_t LostTrgUpdateTime[MAX_NBRD][MAX_NCH];	// Time stamp of the event containg Lost Trigger information (1K flag)
	uint64_t PrevLostTrgUpdateTime[MAX_NBRD][MAX_NCH];	// Previous value of TrgLostUpdateTime (used to calculate elapsed time for LostTriggers)


} Stats_t;




//****************************************************************************
// struct that contains the configuration parameters (HW and SW)
//****************************************************************************
typedef struct Config_t {

	// ----------------------------------------------------------------------
	// System Setup (SW settings)
	// ----------------------------------------------------------------------
	int AcquisitionMode;		// LIST (on line, list only), MIXED (on line with waveforms), OFF_LINE (off line from file)

	// Parameters for the board connection (one per board)
	int LinkType[MAX_NBRD];
    int LinkNum[MAX_NBRD];
    int ConetNode[MAX_NBRD];
    uint32_t BaseAddress[MAX_NBRD];	// for VME boards only

	int StatUpdateTime;				// Update time in ms for the statistics (= averaging time) 

	// System info (to be filled after digitizers have been opened), assuming boards of the same type
	int DigitizerModel;				// Type of Digitizer (751, 720...)
	int DppType;                    // Type of DPP (PSD or CI)
	int FWrev;						// FW revision (minor number)
	int NumBrd;                     // Tot number of boards
	int NumPhyCh;                   // Num of physical channels per board
	int NumAcqCh;                   // Total num of channel (physical + virtual) per board
	int Tsampl;                     // Sampling Period
	int Nbit;                       // Number of bits
	char FwTypeString[40];			// String with the firmware type
	uint32_t EnableMask[MAX_NBRD];	// Channel enable mask (bit n => channel n) for each board

	// ----------------------------------------------------------------------
	// Input/Output files (SW settings)
	// ----------------------------------------------------------------------
	// Enable Output file saving
	int SaveRawData;				// Save raw data (events before selection)
	int SaveHistograms;				// Save Histograms (Enabling Mask: bit 0 = Energy, bit 1 = Start-Stop Time, bit 2 = PSD)
	int SaveWaveforms;				// Save Waveforms (events after selection)
	int SaveLists;					// Save 3 column lists with timestamp, energy, psd. (events after selection)
	int SaveRunInfo;				// Save Run Info file with Run Description and a copy of the config file
	// Data and List Files information
	char DataFilePath[200];			// path to the folder where output data files are written
	char InputDataFilePath[200];	// path to the folder where input data files are read (if not set, DataFilePath is used)
	char InputDataFileName[200];	// name of the input data file for the off-line run
	int OutFileFormat;				// 0=BINARY or 1=ASCII (only for list and waveforms files; raw data files are always binary)
	int OutFileTimeStampUnit;		// 0=ps, 1=ns, 2=us, 3=ms, 4=s; numbers are integer for option 0, float for other options
	int HeaderInListFiles;			// Add (1) or not (1) a header in the list files (containing channel number, format, etc...)
	int FlagsInListFiles;			// Mask for the flags saved in the output list files (if 0, no flags are saved)
	int NoBadEventsInListFiles;			// Don't save bad events (saturation, pile-up) in list files
	int HistoOutputFormat;			// 0=ASCII 1 column, 1= ASCII 2 column, 2=ANSI42
	int InputFileType;				// Input data file type: 0=RAW, 1=LIST (binary), 2=LIST (ASCII)
	int LoopInputFile;				// when 1, the off-line run loops over the input data file forever
	int ConfirmFileOverwrite;		// ask before overwriting output data file

	// ----------------------------------------------------------------------
	// Jobs and Runs (SW settings)
	// ----------------------------------------------------------------------
	// Run Number (used in output file names)
	int RunNumber;					// Run Number (taken from command line, from file or from job scheduler)
	int AutoRunNumber;				// If enabled, the run number is automatically increased (this option is ignored when running jobs)
	int JobStartRun;				// Run number of the fisrt job 
	int JobStopRun;					// Run number of the last job 
	int JobSleep;					// Sleep between jobs (in ms)
	char RunDescription[MAX_RUN_DESCR_LENGTH];		// description of the run

	// ----------------------------------------------------------------------
	// Stopping criteria (SW settings)
	// ----------------------------------------------------------------------
	int StopOnTime;					// Stop after N msec real time (never stop when 0)
	int StopOnLiveTime;				// Stop after N msec live time
	int StopOnTotalEvents;			// Stop after N total events acquired (never stop when 0)
	int StopOnEnergyEvents;			// Stop after N energy events acquired (never stop when 0)
	int StopOnTimeEvents;			// Stop after N time events acquired (never stop when 0)

	// ----------------------------------------------------------------------
	// Acquisition Setup (HW settings)
	// ----------------------------------------------------------------------
	// Some Board Settings
    int FPIOtype;					// Front Panel IOtype (NIM/TTL)
	int FanSpeed;                   // Fan Speed (0=low, 1=high)

	// Acquisition modes 
	int WaveformEnabled;			// 0 = list mode, 1 = list+waveforms (MIXED mode)
	int StartMode;					// Defines the way to start the acquisition (SW controlled, SYNCIN, 1st Trigger, etc...)
	int TrginMode;					// Defines the use of the TRGIN input (external trigger, veto, gate, coincidence)
	int SyncinMode;					// Defines the use of the SIN/GPI input (reset of time stamp, run start/stop control)
	int TrgoutMode;					// 0 = OR of channel triggers; 1 = 1KHz square wave; 2 = 1KHz pulses; 3 = 10KHz square wave; 4 = 10KHz pulses
	int TrgoutMask;					// Mask for the channels that send triggers to TRGOUT

	// HACK: the following 3 parameters could be indiviual for some boards, but this software doesn't manage it...
    int RecordLength;				// Num of samples in the waveform 
    int PreTrigger;					// Num of samples in the pre trigger 
    int TrgHoldOff;					// Trigger Hold off (in ns) 

	int EventBuffering;				// Events are saved in memory buffers as aggregates of N events, N=0 (default) means auto-setting

	// Coincidences and trigger logic implemented in FPGA 
	int CoincWindow;							// Coincidence Window (in ns)
    int CoincMode;								// Coincidence Mode (0=disabled, 1 = majority, 2=couples, 3=one_to_all, 4=ext_trg)
	int AntiCoincidence;						// When 1, it negates coincide logic (invert accepted and rejected events)
	int MajorityLevel;							// Min number of channels triggered to validate the majority

	// Channel Settings 
	int EnableInput[MAX_NBRD][MAX_NCH];         // Enable input 
    int TrgThreshold[MAX_NBRD][MAX_NCH];		// Trigger Threshold
	int PulsePolarity[MAX_NBRD][MAX_NCH];		// Pulse Polarity (0=pos, 1=neg)
	int NsBaseline[MAX_NBRD][MAX_NCH];			// Num of Samples for the input baseline calculation (for PSD)
    int TrapNSBaseline[MAX_NBRD][MAX_NCH];		// Num of Samples for the trapezoid baseline calculation (for PHA)
	int FixedBaseline[MAX_NBRD][MAX_NCH];		// Fixed baseline when NsBaseline=0
	int GateWidth[MAX_NBRD][MAX_NCH];			// Gate Width (in ns) (it's the long gate for DPP_PSD)
	int ShortGateWidth[MAX_NBRD][MAX_NCH];		// Short Gate Width (in ns)
	int PreGate[MAX_NBRD][MAX_NCH];				// Pre Gate (num of samples the gate starts before the trigger)
	int ChargeSensitivity[MAX_NBRD][MAX_NCH];	// Charge sensitivity (value depends on the FW type)
	int ChargeLLD[MAX_NBRD][MAX_NCH];			// Charge Cut (low level discr based on charge)
	int InputDynamicRange[MAX_NBRD][MAX_NCH];	// for x730 only: 0=2Vpp, 1=0.5Vpp
	int PurGap[MAX_NBRD][MAX_NCH];				// Threhsold for peak-valley-peak detection
	int PileUpMode[MAX_NBRD][MAX_NCH];			// Pile Up mode: 0=ignore, 1=reject, 2=retrigger (x751 only)
    int DCoffset[MAX_NBRD][MAX_NCH];			// input DC offset (from 0 to 65535)
    int ACcoupling[MAX_NBRD][MAX_NCH];			// enable AC coupling (Hexagon only). 0=disabled, 1=1.8us, 2=4.7us, 3=10us
    int ACPZcomp[MAX_NBRD][MAX_NCH];			// Pole Zero compensation for the AC coupling
    float BaselineDCoffset[MAX_NBRD][MAX_NCH];	// another way to set the input DC offset (0 to 100)
    float PSDcutThr[MAX_NBRD][MAX_NCH];	    	// on board PSD cut from 0 to 1 (0=disabled)
    int PSDcutType[MAX_NBRD][MAX_NCH];			// 0=cut gammas (cut if PSD<thr), 1=cut neutrons (cut if PSD>thr)
    int EnablePedestal[MAX_NBRD][MAX_NCH];	    // Enable Charge pedestal (add a fixed offset of 1024 to the charge value)
    int CFDdelay[MAX_NBRD][MAX_NCH];			// CFD delay in sampling periods
    int CFDfraction[MAX_NBRD][MAX_NCH];			// CFD fraction 0=25%, 1=50%, 2=75%, 3=100%
    int CFDinterp[MAX_NBRD][MAX_NCH];			// Interpolation Mode 0=ZC+/-1, 1=ZC+/-2, 2=ZC+/-3 2=ZC+/-4
    int DiscrMode[MAX_NBRD][MAX_NCH];			// Discriminator Mode: 0=LED or RCCR2, 1=CFD, 2=CFD_PHS 3=LED_PHA
    int TrapRiseTime[MAX_NBRD][MAX_NCH];		// Trapezoid Rise Time (DPP_PHA)
    int TrapFlatTop[MAX_NBRD][MAX_NCH];			// Trapezoid Flat Top (DPP_PHA)
    int TrapPoleZero[MAX_NBRD][MAX_NCH];		// Trapezoid Pole-Zero cancellation (input exponential decay time) (DPP_PHA)
	float TrapDigitalGain[MAX_NBRD][MAX_NCH];	// Trapezoid digital gain 
    int PeakingTime[MAX_NBRD][MAX_NCH];			// Trapezoid Peaking Time (DPP_PHA)
    int NSPeak[MAX_NBRD][MAX_NCH];				// Numer of samples averaged in the flat top to get the peak value = energy (DPP_PHA)
    int PeakHoldOff[MAX_NBRD][MAX_NCH];			// Minimum distance between Trapezoids (below this, the PUR will discard them) (DPP_PHA)
    int TTFsmoothing[MAX_NBRD][MAX_NCH];		// Smoothing factor in the trigger and timing filter (DPP_PHA)
	int SmoothedForCharge[MAX_NBRD][MAX_NCH];	// Use smoothed input for the charge integration
    int TTFdelay[MAX_NBRD][MAX_NCH];			// Delay in the trigger and timing filter (DPP_PHA)
	int Decimation[MAX_NBRD][MAX_NCH];			// Decimation 0=disabled, 1=1/2, 2=1/4, 3 = 1/8 (DPP_PHA)
	float VetoWindow[MAX_NBRD][MAX_NCH];		// Veto Window Width in ns
	float HV_Vset[MAX_NBRD][MAX_NCH];			// High Voltage Vset (Volts)
	float HV_Iset[MAX_NBRD][MAX_NCH];			// High Voltage Iset (uA)
	float HV_RampUp[MAX_NBRD][MAX_NCH];			// High Voltage RampUp (V/s)
	float HV_RampDown[MAX_NBRD][MAX_NCH];		// High Voltage RampDown (V/s)

	int EnableIPE[MAX_NBRD][MAX_NCH];			// Enable/Disable Internal pulse emulator (replacing real data from input ADC)
	int IPEfrequency[MAX_NBRD][MAX_NCH];		// IPE frequency: 0=100Hz, 1=1KHz, 2=10KHz, 3=100KHz (approx.)
	int IPEdecay[MAX_NBRD][MAX_NCH];			// IPE decay time in us: 
	int IPEaddnoise[MAX_NBRD][MAX_NCH];			// IPE add random noise (0=no noise, 1=noise added)
	int IPEamplitude[MAX_NBRD][MAX_NCH];		// IPE pulse amplitude in ADC LSB
	int IPErisetime[MAX_NBRD][MAX_NCH];			// IPE rise time (low pass filter); 0 to 7, 0 is the fastest
	int IPErandom[MAX_NBRD][MAX_NCH];			// IPE random mode: 0=periodic, 1=pseudo poissonian 

	// Generic write accesses 
	int GWn;
    uint32_t GWbrd[MAX_GW];						// Board Number (-1 = all)
    uint32_t GWaddr[MAX_GW];					// Register Address
    uint32_t GWdata[MAX_GW];					// Data to write
    uint32_t GWmask[MAX_GW];					// Bit Mask

	// ----------------------------------------------------------------------
	// Calibration Parameters (SW settings)
	// ----------------------------------------------------------------------
	// Zero Crossing Calibration
	int CalibrationRun;
	int EnableZCcalibr[MAX_NBRD][MAX_NCH];

	// Voltage Calibration
	int ZeroVoltLevel[MAX_NBRD][MAX_NCH];		// Position of the baseline (zero volt) in ADC channels

	// Energy Gain and Calibration
	float EnergyCoarseGain[MAX_NBRD][MAX_NCH];	// Energy Coarse Gain (requested by the user); can be a power of two (1, 2, 4, 8...) or a fraction (0.5, 0.25, 0.125...)
	float EnergyFineGain[MAX_NBRD][MAX_NCH];	// Energy Fine Gain (requested by the user)
	uint16_t EnergyDiv[MAX_NBRD][MAX_NCH];		// Energy Divisor to get the requested CoarseGain and Rebinning
	float ECalibration_c0[MAX_NBRD][MAX_NCH];	// Energy Calibration c0, c1, c2, c3 (Ekev = Ech*c3^3 + Ech*c2^2 + Ech*c1 + c0)
	float ECalibration_c1[MAX_NBRD][MAX_NCH];	
	float ECalibration_c2[MAX_NBRD][MAX_NCH];	
	float ECalibration_c3[MAX_NBRD][MAX_NCH];
	float AddBackFullScale;

	// Time calibration
	float TstampOffset[MAX_NBRD][MAX_NCH];		// Delay line in ns (add a delay to each time stamps)
	int TspectrumMode;							// Timing spectrum (TAC): 0=start-stop, 1=intervals (time difference between consecutive events)

	// scatter plot mode
	int ScatterPlotMode;

	// ----------------------------------------------------------------------
	// Event selection and building criteria (SW settings)
	// ----------------------------------------------------------------------
	float EnergyLCut[MAX_NBRD][MAX_NCH];		// lower Level energy cut for the event selection
	float EnergyUCut[MAX_NBRD][MAX_NCH];		// upper Level energy cut for the event selection
	float PsdLCut[MAX_NBRD][MAX_NCH];			// lower Level PSD cut for the event selection
	float PsdUCut[MAX_NBRD][MAX_NCH];			// lower Level PSD cut for the event selection
	float TimeCorrelWindow;						// Time interval for two events to be considered correlated
	int BuildMode;								// Build Mode: SINGLES, PAIRED AND/OR, CHREF_AND_ANY, CLOVER
	int CloverNch;								// Number of channels (crystals) in a clover
	int CloverMajority;							// Minimum number of fired crystals 
	int TOFstartChannel;						// Channel used as a start in the TOF measurements
	int TOFstartBoard;							// Board to which the TOFstartChannel belongs
	int EnableEnergyFilter;						// 1=Enable energy filter (EnergyLCut <= Energy <= EnergyUCut)
	int EnableEnergySkim;						// 1=Enable energy filter in the FPGA (EnergyLCut <= Energy <= EnergyUCut)
	int EnablePSDFilter;						// 1=Enable PSD filter (PsdLCut <= PSD <= PsdUCut)
	int WaveformProcessor;						// 0=disabled, bit0 = timing interpolation, bit1 = charge + psd

	// ----------------------------------------------------------------------
	// Histogrammer settings (SW settings)
	// ----------------------------------------------------------------------
	int THnbin;									// Number of bins in the T histograms
	int EHnbin;									// Number of bins in the E histograms
	int MCSHnbin;								// Number of bins in the MCS histograms
	int DwellTime;								// Dwell time for the MCS (bin size in us)
	float THmin[MAX_NBRD][MAX_NCH];				// lower time value used to make time histograms 
	float THmax[MAX_NBRD][MAX_NCH];				// upper time value used to make time histograms 
	
} Config_t;



//****************************************************************************
// Global Variables
//****************************************************************************
extern Config_t		WDcfg;		// struct containing all acquisition parameters
extern Stats_t		Stats;		// struct containing variables for the statistics
extern Histos_t		Histos;		// struct containing the histograms
extern SysVars_t    SysVars;	// systema variables


extern int handle[MAX_NBRD];							// board handles (for the CAEN_DGTZ library)
extern int TraceSet[MAX_NTRACES];						// Probe Settings
extern char TraceNames[MAX_NTRACES][MAX_NTRSETS][20];	// Trace Name
extern int ChToPlot, BrdToPlot;							// Board and Channel active in the plot
extern uint32_t OutFileSize;							// Output file size
extern FILE *MsgLog;									// Message Log
extern int AcqRun;										// Acquisition running
extern int Failure;										// Severe error => stop the program
extern int IntegratedRates;								// 0=istantaneous rates; 1=integrated rates
extern int StopCh[MAX_NBRD][MAX_NCH];					// Individual Stop Acquisition (based on time or counts)

#endif