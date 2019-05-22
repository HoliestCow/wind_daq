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


#include <stdio.h>
#include <CAENDigitizer.h>

#include "digiTES.h"
#include "Statistics.h"
#include "BoardUtils.h"
#include "ZCcal.h"
#include "Queues.h"
#include "Histograms.h"
#include "Console.h"

/* ###########################################################################
*  Functions
*  ########################################################################### */

// iterative solution of the formula ocr = icr * exp(-icr*deadtime)
double CalulateICR(double ocr, double DeadTime)
{
	double icr, ocrx, err = 0.01;
	int i;
	icr = ocr;
	for(i=0; i<1000; i++) {
		ocrx = icr * exp(-icr*DeadTime);
		if ((fabs(ocrx - ocr)/ocr) < err)
			break;
		icr += (ocr - ocrx)/2;
	} 
	return icr;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Reset all the counters, histograms, etc...
// Return: 0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ResetStatistics()
{
	if (AcqRun)	StopAcquisition();
	memset(&Stats, 0, sizeof(Stats));
	ResetHistograms();
	if (AcqRun)	{
		StartAcquisition();
		Stats.StartTime = get_time();
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Calculate some statistcs (rates, etc...) in the Stats struct
// Return: 0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int UpdateStatistics(uint64_t CurrentTime)
{
	int b, ch;
	// Calculate Real Time (i.e. total acquisition time from the start of run).
	// If possible, the real time is taken form the most recent time stamp (coming from any channel),
	// otherwise it is taken from the computer time (much less precise)
	if (Stats.LatestProcTstampAll > Stats.PrevProcTstampAll) {
		Stats.AcqRealTime = (float)(Stats.LatestProcTstampAll/1e6f);	// Acquisition Real time from board in ms
		Stats.RealTimeSource = REALTIME_FROM_BOARDS;
	} else {
		Stats.AcqRealTime = (float)(CurrentTime - Stats.StartTime);	// time from computer in ms
		Stats.RealTimeSource = REALTIME_FROM_COMPUTER;
	}
	// Calculate the data throughput rate from the boards to the computer
	if (IntegratedRates) Stats.RxByte_rate = (float)(Stats.RxByte_cnt)/(Stats.AcqRealTime*1048.576f);
	else Stats.RxByte_rate = (float)(Stats.RxByte_cnt - Stats.RxByte_pcnt)/((float)(CurrentTime - Stats.LastUpdateTime)*1048.576f);
	Stats.RxByte_pcnt = Stats.RxByte_cnt;
	Stats.BlockRead_cnt = 0;
	Stats.LastUpdateTime = CurrentTime;

	// calculate counts and rater for each channel
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumAcqCh; ch++) {
			if (WDcfg.EnableInput[b][ch])  {
				Stats.EvRead_rate[b][ch] = 0;
				Stats.EvFilt_rate[b][ch] = 0;
				Stats.EvPileUp_rate[b][ch] = 0;
				Stats.EvOvf_rate[b][ch] = 0;
				Stats.EvUncorrel_rate[b][ch] = 0;
				Stats.Satur_rate[b][ch] = 0;
				Stats.EvOutput_rate[b][ch] = 0;
				if (IntegratedRates && (Stats.LatestReadTstamp[b][ch] > 0)) {
					float elapsed = (float)(Stats.LatestReadTstamp[b][ch]) / 1e9f;  // elapsed time (in seconds) of read events
					Stats.EvRead_rate[b][ch] = Stats.EvRead_cnt[b][ch]/elapsed;  // events read from the board
					Stats.EvFilt_rate[b][ch] = Stats.EvFilt_cnt[b][ch]/elapsed;  // events that passed the SW filters (cuts and correlation)
					Stats.EvPileUp_rate[b][ch] = Stats.EvPileUp_cnt[b][ch]/elapsed;  // pile-up events 
					Stats.EvOvf_rate[b][ch] = Stats.EvOvf_cnt[b][ch]/elapsed;  // overflow events 
					Stats.EvUncorrel_rate[b][ch] = Stats.EvUncorrel_cnt[b][ch]/elapsed;  // Uncorrelated events (vetoed or not matching the coincidence criteria)
					Stats.Satur_rate[b][ch] = Stats.Satur_cnt[b][ch]/elapsed;  // saturated events (input voltage saturation => clipping effects)
				} else if (Stats.LatestReadTstamp[b][ch] > Stats.PrevReadTstamp[b][ch]) {
					float elapsed = (float)(Stats.LatestReadTstamp[b][ch] - Stats.PrevReadTstamp[b][ch]) / 1e9f;  // elapsed time (in seconds) of read events
					Stats.EvRead_rate[b][ch] = (Stats.EvRead_cnt[b][ch] - Stats.EvRead_pcnt[b][ch])/elapsed;  // events read from the board
					Stats.EvFilt_rate[b][ch] = (Stats.EvFilt_cnt[b][ch] - Stats.EvFilt_pcnt[b][ch])/elapsed;  // events that passed the SW filters (cuts and correlation)
					Stats.EvPileUp_rate[b][ch] = (Stats.EvPileUp_cnt[b][ch] - Stats.EvPileUp_pcnt[b][ch])/elapsed;  // pile-up events 
					Stats.EvOvf_rate[b][ch] = (Stats.EvOvf_cnt[b][ch] - Stats.EvOvf_pcnt[b][ch])/elapsed;  // overflow events 
					Stats.EvUncorrel_rate[b][ch] = (Stats.EvUncorrel_cnt[b][ch] - Stats.EvUncorrel_pcnt[b][ch])/elapsed;  // Uncorrelated events (vetoed or not matching the coincidence criteria)
					Stats.Satur_rate[b][ch] = (Stats.Satur_cnt[b][ch] - Stats.Satur_pcnt[b][ch])/elapsed;  // saturated events (input voltage saturation => clipping effects)
				}

				if ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
					Stats.EvOutput_rate[b][ch] = Stats.EvFilt_rate[b][ch] - Stats.EvUncorrel_rate[b][ch] - Stats.EvPileUp_rate[b][ch];
				else
					Stats.EvOutput_rate[b][ch] = Stats.EvFilt_rate[b][ch] - Stats.EvUncorrel_rate[b][ch] - Stats.Satur_rate[b][ch];

				// correct for temporary errors due to asynchronous readings
				if (Stats.EvFilt_rate[b][ch] > Stats.EvRead_rate[b][ch])   Stats.EvFilt_rate[b][ch] = Stats.EvRead_rate[b][ch]; 
				if (Stats.EvPileUp_rate[b][ch] > Stats.EvRead_rate[b][ch]) Stats.EvPileUp_rate[b][ch] = Stats.EvRead_rate[b][ch]; 
				if (Stats.EvOutput_rate[b][ch] < 0) Stats.EvOutput_rate[b][ch] = 0;

				// Total number of pulses (triggers) at the board input. If Stats.EvInput_cnt<0, then the board doesn't provide this trigger counter
				if (!AcqRun) {
					Stats.EvInput_rate[b][ch] = 0;
				} else if (Stats.EvInput_cnt[b][ch] == 0xFFFFFFFFFFFFFFFF) {  
					Stats.EvInput_rate[b][ch] = -1;  // Not available
				} else { 
					if ((Stats.EvRead_rate[b][ch] == 0) || (Stats.ICRUpdateTime[b][ch] == 0)) {
						Stats.EvInput_rate[b][ch] = 0;
					} else if (IntegratedRates) {
						Stats.EvInput_rate[b][ch] = Stats.EvInput_cnt[b][ch]/((float)Stats.ICRUpdateTime[b][ch]/1e9f);
						Stats.EvInput_pcnt[b][ch] = Stats.EvInput_cnt[b][ch];
						Stats.PrevICRUpdateTime[b][ch] = Stats.ICRUpdateTime[b][ch];
					} else if (Stats.ICRUpdateTime[b][ch] > Stats.PrevICRUpdateTime[b][ch]) {
						Stats.EvInput_rate[b][ch] = (Stats.EvInput_cnt[b][ch] - Stats.EvInput_pcnt[b][ch])/((float)(Stats.ICRUpdateTime[b][ch] - Stats.PrevICRUpdateTime[b][ch])/1000000000);
						Stats.EvInput_pcnt[b][ch] = Stats.EvInput_cnt[b][ch];
						Stats.PrevICRUpdateTime[b][ch] = Stats.ICRUpdateTime[b][ch];
					} else if (((float)Stats.PrevICRUpdateTime[b][ch]/1e6) < (Stats.AcqRealTime-10000)) {  // if there is no ICR update after 10 sec, assume ICR=Read Rate
						Stats.EvInput_rate[b][ch] = Stats.EvRead_rate[b][ch];
					}
				}
				if ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
					Stats.EvInput_rate[b][ch] = (float)CalulateICR(Stats.EvInput_rate[b][ch], (1.4 * WDcfg.TTFdelay[b][ch])/1e9);  // compensate for paralyzable dead-time (i.e. inability to get pulses too close)
				if ((WDcfg.DppType == DPP_PSD_751) || (WDcfg.DppType == DPP_PSD_720) || ((WDcfg.DppType == DPP_PSD_730) && (WDcfg.PileUpMode == 0)))
					Stats.EvInput_rate[b][ch] = (float)CalulateICR(Stats.EvInput_rate[b][ch], (WDcfg.GateWidth[b][ch])/1e9);  // compensate for paralyzable dead-time (i.e. inability to get pulses too close)
				if ((Stats.EvInput_rate[b][ch] != -1) && (Stats.EvInput_rate[b][ch] < Stats.EvRead_rate[b][ch]))  // the ICR is updated every N triggers (typ N=1024) so it may result affected by an error that makes it
					Stats.EvInput_rate[b][ch] = Stats.EvRead_rate[b][ch];  // smaller than the event trhoughput; in this case, it is forced equal to event throughput to avoid negative dead-time

				// Number of lost pulses (triggers). If Stats.EvLost_cnt<0, then the board doesn't provide this trigger counter
				if (Stats.EvLost_cnt[b][ch] < 0) {  
					Stats.EvLost_rate[b][ch] = -1;  // Not available
				} else { 
					if (IntegratedRates && (Stats.LostTrgUpdateTime[b][ch] > 0)) {
						Stats.EvLost_rate[b][ch] = Stats.EvLost_cnt[b][ch]/((float)Stats.LostTrgUpdateTime[b][ch]/1e9f);
						Stats.EvLost_pcnt[b][ch] = Stats.EvLost_cnt[b][ch];
						Stats.PrevLostTrgUpdateTime[b][ch] = Stats.LostTrgUpdateTime[b][ch];
					} else if (Stats.LostTrgUpdateTime[b][ch] > Stats.PrevLostTrgUpdateTime[b][ch]) {
						Stats.EvLost_rate[b][ch] = (Stats.EvLost_cnt[b][ch] - Stats.EvLost_pcnt[b][ch])/((float)(Stats.LostTrgUpdateTime[b][ch] - Stats.PrevLostTrgUpdateTime[b][ch])/1e9f);
						Stats.EvLost_pcnt[b][ch] = Stats.EvLost_cnt[b][ch];
						Stats.PrevLostTrgUpdateTime[b][ch] = Stats.LostTrgUpdateTime[b][ch];
					} else {
						Stats.EvLost_rate[b][ch] = 0;	// HACK: what to do here ???
					}
				}
				if (Stats.EvLost_rate[b][ch] > Stats.EvInput_rate[b][ch])  
					Stats.EvLost_rate[b][ch] = Stats.EvInput_rate[b][ch];  

				// Dead Time
				if ((Stats.EvInput_rate[b][ch] > 0) && (Stats.EvLost_rate[b][ch] >= 0)) {
					if ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
						//Stats.DeadTime[b][ch] = 1 - (Stats.EvRead_rate[b][ch] - Stats.EvPileUp_rate[b][ch])/Stats.EvInput_rate[b][ch];
						Stats.DeadTime[b][ch] = 1 - (Stats.EvInput_rate[b][ch] - Stats.EvLost_rate[b][ch] - Stats.EvPileUp_rate[b][ch])/Stats.EvInput_rate[b][ch];
					else
						Stats.DeadTime[b][ch] = 1 - (Stats.EvInput_rate[b][ch] - Stats.EvLost_rate[b][ch])/Stats.EvInput_rate[b][ch];
				} else {
					Stats.DeadTime[b][ch] = 0;
				}
				if (Stats.DeadTime[b][ch] < 0)	Stats.DeadTime[b][ch] = 0;
				if (Stats.DeadTime[b][ch] > 1) 	Stats.DeadTime[b][ch] = 1;

				// Percent of Busy time in the board (memory full; during this time the board is not able to accept triggers)
				Stats.BusyTime[b][ch] = 0;
				if (Stats.LatestReadTstamp[b][ch] > Stats.PrevReadTstamp[b][ch]) {
					float period=0;
					if (Stats.EvInput_rate[b][ch] > 0)
						period = 1e9f/Stats.EvInput_rate[b][ch];  // in ns
					Stats.BusyTime[b][ch] = ((float)Stats.BusyTimeGap[b][ch] - period)/(Stats.LatestReadTstamp[b][ch] - Stats.PrevReadTstamp[b][ch]);
				}
				if (Stats.BusyTime[b][ch] < 0) Stats.BusyTime[b][ch] = 0;
				if (Stats.BusyTime[b][ch] > 1) Stats.BusyTime[b][ch] = 1;


				// matching ratio (out_rate / in_rate) of the filters applied by the software
				if (Stats.EvProcessed_cnt[b][ch] > Stats.EvProcessed_pcnt[b][ch])
					Stats.MatchingRatio[b][ch] = (float)(Stats.EvFilt_cnt[b][ch] - Stats.EvFilt_pcnt[b][ch])/(Stats.EvProcessed_cnt[b][ch] - Stats.EvProcessed_pcnt[b][ch]);
				else
					Stats.MatchingRatio[b][ch] = 0;

				// save current counters to prev_counters
				Stats.EvRead_dcnt[b][ch] = Stats.EvRead_cnt[b][ch] - Stats.EvRead_pcnt[b][ch];
				Stats.EvRead_pcnt[b][ch] = Stats.EvRead_cnt[b][ch];
				Stats.EvFilt_pcnt[b][ch] = Stats.EvFilt_cnt[b][ch];
				Stats.Satur_pcnt[b][ch] = Stats.Satur_cnt[b][ch];
				Stats.EvPileUp_pcnt[b][ch] = Stats.EvPileUp_cnt[b][ch];
				Stats.EvOvf_pcnt[b][ch] = Stats.EvOvf_cnt[b][ch];
				Stats.EvUncorrel_pcnt[b][ch] = Stats.EvUncorrel_cnt[b][ch];
				Stats.EvProcessed_pcnt[b][ch] = Stats.EvProcessed_cnt[b][ch];
				Stats.PrevReadTstamp[b][ch] = Stats.LatestReadTstamp[b][ch];
				Stats.PrevProcTstamp[b][ch] = Stats.LatestProcTstamp[b][ch];
				Stats.BusyTimeGap[b][ch] = 0;
			}
		}
	}
	Stats.PrevProcTstampAll = Stats.LatestProcTstampAll;
	return 0;
}

