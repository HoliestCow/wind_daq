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
#include "Queues.h"
#include "Readout.h"

#include <stdio.h>

// Event Queues and pointers 
GenericDPPEvent_t *EvQ[MAX_NBRD][MAX_NCH];    // Event queues
uint32_t EvWpnt[MAX_NBRD][MAX_NCH];           // Write pointer to EvQ
uint32_t EvRpnt[MAX_NBRD][MAX_NCH];           // Read pointer to EvQ
uint32_t Qnev[MAX_NBRD][MAX_NCH];             // Number of events in the queues
uint32_t AlmostFullFlags[MAX_NBRD];			  // Queue Almost Full flags (one bit per channel)
uint32_t EmptyFlags[MAX_NBRD];				  // Queue Empty flags (one bit per channel)
int Inactive[MAX_NBRD][MAX_NCH];              // Enabled channels that are not giving data
int AllQueuesAreEmpty;
int AllQueuesHaveData;
int OneQueueIsAlmostFull;
uint64_t TimeLine = 0;
int QueuesCreated = 0;
int InitCloverVars = 1;

// Readout Log File
FILE *qlog = NULL;

// --------------------------------------------------------------------------------------------------------- 
// Description: Initialize variables and allocate memory buffers for queues
// Inputs:		-
// Outputs:		AllocatedMemSize = total num of bytes allocated
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CreateQueues(uint32_t *AllocatedMemSize)
{
	int b, ch;
	if (ENQLOG) {
		qlog = fopen("QueueLog.txt", "w");
		printf("WARNING: Queue Log File enabled!!!\n");
	}

	*AllocatedMemSize=0;

	ClearQueues();

	// Allocate event queues
	for(b=0; b<MAX_NBRD; b++) {
		for (ch=0; ch<MAX_NCH; ch++) {
			if (WDcfg.EnableInput[b][ch]) {
				EvQ[b][ch] = (GenericDPPEvent_t *)malloc(EV_QUEUE_SIZE * sizeof(GenericDPPEvent_t));
				*AllocatedMemSize += EV_QUEUE_SIZE * sizeof(GenericDPPEvent_t);
				if (EvQ[b][ch] == NULL) {
					printf("Can't allocate memory buffers for the Queues\n");
					return -99;
				}
			} else {
				EvQ[b][ch] = NULL; 
			}
		}
	}
	QueuesCreated = 1;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: clear queues and init variables
// Inputs:		-
// Outputs:		AllocatedMemSize = total num of bytes allocated
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ClearQueues()
{
	int b, ch;
	// Init global variables
	for(b=0; b<MAX_NBRD; b++) {
		EmptyFlags[b] = 0xFFFFFFFF;
		for (ch=0; ch<MAX_NCH; ch++) {
			EvWpnt[b][ch] = 0;
			EvRpnt[b][ch] = 0;
			EvRpnt[b][ch] = 0;
			EvWpnt[b][ch] = 0;
			Qnev[b][ch] = 0;
			Inactive[b][ch] = 0;
			AlmostFullFlags[b] = 0;
		}
	}
	AllQueuesAreEmpty=1;
	AllQueuesHaveData=0;
	OneQueueIsAlmostFull=0;
	TimeLine = 0;
	InitCloverVars = 1;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: free memory 
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int DestroyQueues()
{
	int b, ch;
	if (!QueuesCreated) return 0;
	for(b=0; b<MAX_NBRD; b++) {
		for (ch=0; ch<MAX_NCH; ch++) {
			if (EvQ[b][ch] != NULL)
				free(EvQ[b][ch]);
		}
	}
	if (qlog != NULL) {
		fprintf(qlog, "Closing Queue Log\n");
		fclose(qlog);
	}
	QueuesCreated = 0;
	InitCloverVars = 1;
	return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: write one event into the queue 
// Inputs:	b = board number
//			ch = channel number
// Outputs:	evnt = event data structure
// Return:	1=OK, 0=queue is full, -1=error
// --------------------------------------------------------------------------------------------------------- 
int PushEvent(int b, int ch, GenericDPPEvent_t *evnt)
{
	int bb, cc;

	Stats.TotEvRead_cnt++;
	Stats.EvRead_cnt[b][ch]++;
	Stats.LatestReadTstamp[b][ch] = evnt->TimeStamp;
	Inactive[b][ch] = 0;
	if (TimeLine < evnt->TimeStamp)
		TimeLine = evnt->TimeStamp;

	if (EvQ[b][ch] == NULL)  // the queue is not allocated
		return -1;
	if (Qnev[b][ch] == EV_QUEUE_SIZE)  { // the queue is full
		Stats.EvLost_cnt[b][ch]++;
		FreeWaveform(evnt->Waveforms);
		return 0;
	}
	if (ENQLOG) fprintf(qlog, "Queue[%d][%d] Push Event: WP=%d RP=%d; TS=%llu ", b, ch, EvWpnt[b][ch], EvRpnt[b][ch], (unsigned long long)EvQ[b][ch][EvWpnt[b][ch]].TimeStamp);
	memcpy(&EvQ[b][ch][EvWpnt[b][ch]], evnt, sizeof(GenericDPPEvent_t));
	EvWpnt[b][ch] = (EvWpnt[b][ch] + 1) % EV_QUEUE_SIZE;
	Qnev[b][ch]++;
	EmptyFlags[b] &= ~(1 << ch);
	AllQueuesAreEmpty = 0;
	if (Qnev[b][ch] == EV_QUEUE_ALMFULL_LEVEL) { // the queue is almost full
		OneQueueIsAlmostFull = 1;
		AlmostFullFlags[b] |= (1 << ch);
		if (ENQLOG) fprintf(qlog, "Queue[%d][%d] is almost full:", b, ch);
	}
	if (Qnev[b][ch] == 1) { // this is no longer empty => check other queues
		AllQueuesHaveData = 1;
		for(bb=0; bb<WDcfg.NumBrd; bb++) {
			for(cc=0; cc<WDcfg.NumPhyCh; cc++) {
				if (WDcfg.EnableInput[bb][cc] && !Inactive[bb][cc] && (Qnev[bb][cc] == 0)) { 
					AllQueuesHaveData = 0;
					break;
				}
			}
			if (AllQueuesHaveData == 0)	break;
		}
	}

	return 1;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: Read one event from the queue without removing it
// Inputs:	b = board number
//			ch = channel number
// Outputs:	evnt = event data structure
// Return:	1=OK, 0=queue is empty, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int PeekEvent(int b, int ch, GenericDPPEvent_t **evnt)
{
	*evnt = NULL;
	if (Qnev[b][ch] == 0)
		return 0;
	*evnt = &EvQ[b][ch][(EvRpnt[b][ch]) % EV_QUEUE_SIZE];
	return 1;
}

// -------------------------------------------------------------------------------------------------------- 
// Description: Read (and remove) one event from the queue 
// Inputs:	b = board number
//			ch = channel number
// Outputs:	evnt = event data structure
// Return:	1=OK, 0=queue is empty, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int PopEvent(int b, int ch, GenericDPPEvent_t *evnt)
{
	int bb, cc;
	if (Qnev[b][ch] == 0) 
		return 0;
	if (evnt != NULL)
		memcpy(evnt, &EvQ[b][ch][EvRpnt[b][ch]], sizeof(GenericDPPEvent_t));
	else 
		FreeWaveform(EvQ[b][ch][EvRpnt[b][ch]].Waveforms);
	if (ENQLOG) fprintf(qlog, "Pop Event: brd %d, ch %d: WP=%d RP=%d; TS=%llu ", b, ch, EvWpnt[b][ch], EvRpnt[b][ch], (unsigned long long)EvQ[b][ch][EvWpnt[b][ch]].TimeStamp);
	Qnev[b][ch]--;
	EvRpnt[b][ch] = (EvRpnt[b][ch] + 1) % EV_QUEUE_SIZE;
	if (Qnev[b][ch] == (EV_QUEUE_ALMFULL_LEVEL-1)) { // the queue is no longer almost full => check other queues
		OneQueueIsAlmostFull = 0;
		AlmostFullFlags[b] &= ~(1 << ch);
		for(bb=0; bb<WDcfg.NumBrd; bb++) {
			for(cc=0; cc<WDcfg.NumPhyCh; cc++) {
				if (WDcfg.EnableInput[bb][cc] && (Qnev[bb][cc] >= EV_QUEUE_ALMFULL_LEVEL)) { 
					OneQueueIsAlmostFull = 1;
					break;
				}
			}
			if (OneQueueIsAlmostFull) break;
		}
	}
	if (Qnev[b][ch] == 0) { // the queue is now empty => check other queues
		EmptyFlags[b] |= (1 << ch);
		AllQueuesHaveData = 0;
		AllQueuesAreEmpty = 1;
		for(bb=0; bb<WDcfg.NumBrd; bb++) {
			for(cc=0; cc<WDcfg.NumPhyCh; cc++) {
				if (WDcfg.EnableInput[b][ch] && (Qnev[bb][cc] > 0)) { 
					AllQueuesAreEmpty = 0;
					break;
				}
			}
			if (AllQueuesAreEmpty == 0)	break;
		}
	}
	Stats.EvProcessed_cnt[b][ch]++;
	return 1;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Get the percentage of occupancy of a queue
// Inputs:		b = board number
//				ch = channel number
// Outputs:		-
// Return:		occupancy
// --------------------------------------------------------------------------------------------------------- 
float GetQueueOccupancy(int b, int ch)
{
	return (float)(100.0*Qnev[b][ch]/EV_QUEUE_SIZE);
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Check if a queue is full
// Inputs:		b = board number
//				ch = channel number
// Outputs:		-
// Return:		1=full, 0=not full
// --------------------------------------------------------------------------------------------------------- 
int QueueIsFull(int b, int ch)
{
	return(Qnev[b][ch] == EV_QUEUE_SIZE);
}


// -------------------------------------------------------------------------------------------------------- 
// Description: Get array of event (one per channel); uncorrelated mode (singles)
// Inputs:	-
// Outputs:	evnt = event array
//          EvRdy = event ready flags (0 means no event available for that channel)
// Return:	total number of events present in the array; -1=error
// --------------------------------------------------------------------------------------------------------- 
int BuildEvents_Singles(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int EvRdy[MAX_NBRD][MAX_NCH])
{
	int b, ch, ret = 0;
	int nevFound = 0;

	memset(EvRdy, 0, MAX_NBRD*MAX_NCH*sizeof(int));

	// When running with uncorrelated events, data from board are read only when all the queues are empty in order to guarantee
	// that the queue with more data are read until they are empty
	if (AllQueuesAreEmpty) { 
		if (ReadData() < 0)
			return -1;
	}

	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
			EvRdy[b][ch] = 0;
			if (!WDcfg.EnableInput[b][ch])
				continue;
			while (!EvRdy[b][ch]) {
				ret = PopEvent(b, ch, &evnt[b][ch]);
				// Apply energy and PSD filters
				if (ret) {
					int egood = ((float)evnt[b][ch].Energy >= WDcfg.EnergyLCut[b][ch]) && ((float)evnt[b][ch].Energy <= WDcfg.EnergyUCut[b][ch]);
					int pgood = (evnt[b][ch].psd >= WDcfg.PsdLCut[b][ch]) && (evnt[b][ch].psd <= WDcfg.PsdUCut[b][ch]);
					if ((egood || !WDcfg.EnableEnergyFilter) && (pgood || !WDcfg.EnablePSDFilter)) {
						EvRdy[b][ch] = 1;
						Stats.EvFilt_cnt[b][ch]++;
					} else {
						FreeWaveform(evnt[b][ch].Waveforms);
					}
				} else {
					break; // no data
				}
			}
			if (ret == 0) // no data from this channel
				continue;
			nevFound++;
		}
	}
	return nevFound;
}



// -------------------------------------------------------------------------------------------------------- 
// Description: Build events (one per channel) in One2All mode: one channel in coincidence with any aother
// Inputs:	-
// Outputs:	evnt = event array
//          EvRdy = event ready flags (0 means no event available for that channel)
// Return:	total number of events present in the array; -1=error
// --------------------------------------------------------------------------------------------------------- 
int BuildEvents_One2All(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int EvRdy[MAX_NBRD][MAX_NCH])
{
	int ret = 0, NeedNewData = 0;
	int b, ch;
	int sb, sc;
	int CopyStartToArray;
	int nevFound = 0;
	GenericDPPEvent_t StartEvent, *StopEvent;
	uint64_t StartTime, StopTime, timeout;

	sb = WDcfg.TOFstartBoard;
	sc = WDcfg.TOFstartChannel;

	memset(EvRdy, 0, MAX_NBRD*MAX_NCH*sizeof(int));

	if (!AllQueuesHaveData && !OneQueueIsAlmostFull) {
		if (ReadData() < 0)
			return -1;
		// check for inactive channels (enabled but not giving data)
		timeout = (uint64_t)SysVars.InactiveTimeout * 1000000;  // in ns
		if (TimeLine > timeout) {
			AllQueuesHaveData = 1;
			for(b=0; b<WDcfg.NumBrd; b++) {
				for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
					if ((Qnev[b][ch] == 0) && (Stats.LatestReadTstamp[b][ch] < (TimeLine - timeout))) 
						Inactive[b][ch] = 1;
					if (WDcfg.EnableInput[b][ch] && !Inactive[b][ch] && (Qnev[b][ch] == 0))
						AllQueuesHaveData = 0;
				}
			}
		}
	}

	// get Start Event
	if (PopEvent(sb, sc, &StartEvent) <= 0) {  // no start available
		if (OneQueueIsAlmostFull) {
			for(b=0; b<WDcfg.NumBrd; b++) {
				for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
					if (Qnev[b][ch] >= EV_QUEUE_ALMFULL_LEVEL) { // this queue is almost full and there is no start => remove events until the queue is at 50%
						while(Qnev[b][ch] > (EV_QUEUE_SIZE/2)) {
							Stats.EvLost_cnt[b][ch]++;
							PopEvent(b, ch, NULL);
						}
					}
				}
			}
		}
		return 0;
	}
	// Apply energy and PSD filters to the start event
	if ((WDcfg.EnableEnergyFilter && (((float)StartEvent.Energy < WDcfg.EnergyLCut[sb][sc]) || (((float)StartEvent.Energy > WDcfg.EnergyUCut[sb][sc])))) ||
		(WDcfg.EnablePSDFilter    && ((StartEvent.psd < WDcfg.PsdLCut[sb][sc]) || ((StartEvent.psd > WDcfg.PsdUCut[sb][sc]))))) {
		FreeWaveform(StartEvent.Waveforms);
		return 0;
	}

	StartTime = 1000 * StartEvent.TimeStamp + StartEvent.FineTimeStamp;  // in ps
	nevFound = 1;
	CopyStartToArray = 1;

	// search stops
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
			if ((b == WDcfg.TOFstartBoard) && (ch == WDcfg.TOFstartChannel)) 
				continue;
			if (Qnev[b][ch] == 0) 
				continue;

			while(1) {
				if (!PeekEvent(b, ch, &StopEvent))
					break;
				// Apply energy and PSD filters
				if ((WDcfg.EnableEnergyFilter && (((float)StopEvent->Energy < WDcfg.EnergyLCut[b][ch]) || (((float)StopEvent->Energy > WDcfg.EnergyUCut[b][ch])))) ||
				    (WDcfg.EnablePSDFilter    && ((StopEvent->psd < WDcfg.PsdLCut[b][ch]) || ((StopEvent->psd > WDcfg.PsdUCut[b][ch]))))) {
					PopEvent(b, ch, NULL);
					continue;
				}
				StopTime = 1000 * StopEvent->TimeStamp + StopEvent->FineTimeStamp;  // in ps
				// Apply correlation filter (discard stop events that are too old for the start time)
				if (StopTime < (StartTime - (uint64_t)(WDcfg.TimeCorrelWindow*1000))) { 
					PopEvent(b, ch, NULL);
					continue;
				}
				if (StopTime < (StartTime + (uint64_t)(WDcfg.TimeCorrelWindow*1000))) {
					int64_t tt = StartTime - StopEvent->TimeStamp;
					PopEvent(b, ch, &evnt[b][ch]);
					EvRdy[b][ch] = 1;
					nevFound++;
					if (CopyStartToArray) {
						memcpy(&evnt[WDcfg.TOFstartBoard][WDcfg.TOFstartChannel], &StartEvent, sizeof(GenericDPPEvent_t));
						Stats.EvFilt_cnt[WDcfg.TOFstartBoard][WDcfg.TOFstartChannel]++;
						EvRdy[WDcfg.TOFstartBoard][WDcfg.TOFstartChannel] = 1;
						CopyStartToArray = 0;
					}
					Stats.EvFilt_cnt[b][ch]++;
					break;
				}
				break;
			}
		}
	}

	if (nevFound == 1) {  // only start found
		FreeWaveform(StartEvent.Waveforms);
		return 0;
	}

	return nevFound;
}



// -------------------------------------------------------------------------------------------------------- 
// Description: Build events (one per channel) in clover mode: take events of 4 channels in the coinc window
// Inputs:	-
// Outputs:	evnt = event array
//          EvRdy = event ready flags (0 means no event available for that channel)
// Return:	total number of events present in the array; -1=error
// --------------------------------------------------------------------------------------------------------- 
int BuildEvents_Clover(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int EvRdy[MAX_NBRD][MAX_NCH])
{
	int Nc=WDcfg.CloverNch; // Num of crystals in the clover (typ. 4)
	int ret = 0;
	int b, ch, cc, chmin, NCrdy, vch, pileup;
	int nevFound = 0;
	static int CCmask[MAX_NBRD][MAX_NCH];			// Channel enable mask for the clover
	static uint64_t CloverOldestTstamp[MAX_NBRD][MAX_NCH];	// Oldest Time stamp of the built events 
	static uint32_t MaxEvQueue; // max number of events in a queue while waiting for idle channels
	static float Clover_ch2e[MAX_NBRD][MAX_NCH];
	int CloverRdy[MAX_NBRD][MAX_NCH];		// Clover ready for event building 
	int CloverCoincMask[MAX_NBRD][MAX_NCH]; // mask of the fired crystals in the coinc. window
	int CloverMult[MAX_NBRD][MAX_NCH];		// clover multiplicity (num of fired cristals in the coinc. window)
	float AddBackEnergy = 0;
	float Ech;
	uint16_t Orflags;
	GenericDPPEvent_t *tmp;
	uint64_t Tmin, tps;
		
	memset(EvRdy, 0, MAX_NBRD*MAX_NCH*sizeof(int));

	if (InitCloverVars) {
		for(b=0; b<WDcfg.NumBrd; b++) {
			for(ch=0; ch<WDcfg.NumPhyCh; ch+=Nc) {
				CCmask[b][ch] = (WDcfg.EnableMask[b] >> ch) & ((1<<Nc)-1); 
				CloverOldestTstamp[b][ch] = 0;
				Clover_ch2e[b][ch] = 0;
				for(cc=ch; cc<(ch+Nc); cc++)
					if (Clover_ch2e[b][ch] < WDcfg.ECalibration_c1[b][cc]) Clover_ch2e[b][ch] = WDcfg.ECalibration_c1[b][cc];
			}
		}
		MaxEvQueue = (WDcfg.EventBuffering > 0) ? 10*WDcfg.EventBuffering : 256;
		InitCloverVars = 0;
	}

	// Find at least one clover with all queues not empty. Clover with at least one empty queue cannot be processed.
	// Check for idle channels blocking other channels
	NCrdy = 0;
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumPhyCh; ch+=Nc) {
			int CCrdy; // channel data ready mask in the clover
			CloverRdy[b][ch] = 0;
			if (CCmask[b][ch] == 0) continue;  // all channels in the clover are disabled
			CCrdy = (~EmptyFlags[b] >> ch) & CCmask[b][ch];
			//CloverDRmask[b][ch] = (~EmptyFlags[b] >> ch) & CCmask[b][ch];
			if (CCrdy == CCmask[b][ch]) { // all enabled channels of the clover have data
				CloverRdy[b][ch] = 1;
				NCrdy++;
			} else if (CCrdy > 0) {  // some channels have data and are waiting for idle channels
				for(cc=ch; cc<(ch+Nc); cc++) {
					if (Qnev[b][cc] > MaxEvQueue) {  // too many event waiting in the queue => proceed 
						CloverRdy[b][ch] = 1;
						NCrdy++;
						break;
					}
				}
			}
		}
	}
	
	// If there are no clover ready, then read new data from the board(s) and return
	if (NCrdy == 0) return ReadData();


	// For each clover, find events belonging to the coincidence window (only one event per crystal is allowed)
	for(b=0; b<WDcfg.NumBrd; b++) {
		for(ch=0; ch<WDcfg.NumPhyCh; ch+=Nc) {
			if (CloverRdy[b][ch] == 0) continue;
			// Find minimum time stamp in the queues of the clover
			Tmin = 0;
			for(cc=ch; cc<(ch+Nc); cc++) {
				if (Qnev[b][cc] == 0) continue;
				PeekEvent(b, cc, &tmp);
				tps = tmp->TimeStamp*1000 + tmp->FineTimeStamp;
				if ((Tmin==0) || (Tmin > tps)) {
					Tmin = tps;
					chmin = cc;
				}
			}
			if (Tmin == 0) continue;  // No data for this clover (maybe it's a redundant check)
			// Pop all events belonging to the coincience window (from Tmin to Tmin + CoincWin)
			CloverMult[b][ch] = 0;
			CloverCoincMask[b][ch] = 0;
			Orflags = 0;
			for(cc=ch; cc<(ch+Nc); cc++) {
				int FirstCoinc = 1; // tag the 1st coincidence (subsequent coincideces in the same channel will be discarded)
				while(Qnev[b][cc] > 0) {
					PeekEvent(b, cc, &tmp);
					tps = tmp->TimeStamp*1000 + tmp->FineTimeStamp;
					if (tps <= (Tmin + (uint64_t)(WDcfg.TimeCorrelWindow*1000))) {
						if (FirstCoinc) {
							PopEvent(b, cc, &evnt[b][cc]);
							CloverCoincMask[b][ch] |= (1 << (cc-ch));
							CloverMult[b][ch]++;
							FirstCoinc = 0;
						} else {
							PopEvent(b, cc, NULL);  // discard multiple coincidences
						}
					} else {
						break;
					}
				}
			}

			// Check acceptance criteria (multiplicity, opposite crystals, etc...)
			if (CloverMult[b][ch] < WDcfg.CloverMajority) continue;
			//if ((CloverCoincMask[b][ch] == 0x5) || (CloverCoincMask[b][ch] == 0xA)) continue;  // exclude diagonal coincidences 
			if (Tmin < CloverOldestTstamp[b][ch]) continue;  // late events remained somewhere in the queues

			// Built Event is good: calculate Add Back energy and add one event into the virtual channel
			AddBackEnergy = 0;
			CloverOldestTstamp[b][ch] = Tmin + (uint64_t)(WDcfg.TimeCorrelWindow*1000) - 1;
			pileup = 0;
			for(cc=ch; cc<(ch+Nc); cc++) {
				if (CloverCoincMask[b][ch] & (1 << (cc-ch))) { 
					EvRdy[b][cc] = 1;
					nevFound++;
					Stats.EvFilt_cnt[b][cc]++;
					Orflags |= evnt[b][cc].Flags;
					if ((evnt[b][cc].Flags & EVFLAGS_PILEUP) || (evnt[b][cc].Energy == 0))
						pileup = 1;
					Ech = (float)evnt[b][cc].Energy + (float)rand()/RAND_MAX - (float)0.5;
					AddBackEnergy += CH2KEV(Ech, WDcfg.ECalibration_c0[b][cc], WDcfg.ECalibration_c1[b][cc], WDcfg.ECalibration_c2[b][cc], WDcfg.ECalibration_c3[b][cc]);
				}
			}

			vch = WDcfg.NumPhyCh + ch/Nc;  // virtual channel number
			if ((vch < WDcfg.NumAcqCh) && (!pileup)) {
				if (WDcfg.AddBackFullScale > 0)  // binning of the addback energy forced by user (set full scale)
					evnt[b][vch].Energy = (uint16_t)(WDcfg.EHnbin*AddBackEnergy/WDcfg.AddBackFullScale);
				else if (Clover_ch2e[b][ch] > 0)  // binning taken from one crystal 
					evnt[b][vch].Energy = (uint16_t)(AddBackEnergy/Clover_ch2e[b][ch]);
				else  // force 1 bin = 1 keV (should never take this branch; added for safety)
					evnt[b][vch].Energy = (uint16_t)AddBackEnergy;
				evnt[b][vch].TimeStamp = evnt[b][chmin].TimeStamp;
				evnt[b][vch].FineTimeStamp = evnt[b][chmin].FineTimeStamp;
				evnt[b][vch].psd = 0;
				evnt[b][vch].Waveforms = NULL;
				evnt[b][vch].Flags = Orflags;
				EvRdy[b][vch] = 1;
				Stats.LatestReadTstamp[b][vch] = evnt[b][vch].TimeStamp;
				Stats.EvProcessed_cnt[b][vch]++;
				Stats.EvRead_cnt[b][vch]++;
				Stats.EvFilt_cnt[b][vch]++;
			}
		}
	}
	return nevFound;
}



// -------------------------------------------------------------------------------------------------------- 
// Description: Get array of event (one per channel); correlated mode: paired
// Inputs:	-
// Outputs:	evnt = event array
//          EvRdy = event ready flags (0 means no event available for that channel)
// Return:	total number of events present in the array; -1=error
// --------------------------------------------------------------------------------------------------------- 
int GetEvents_Correlated_Paired(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int EvRdy[MAX_NBRD][MAX_NCH])
{
	int nevFound = 0;

	memset(EvRdy, 0, MAX_NBRD*MAX_NCH*sizeof(int));

	// TO DO....

	return nevFound;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: Build one event array
// Inputs:		-
// Outputs:		evnt = event array
//				NumEv = event of event for that channel 
// Return:		total number of events after building
// --------------------------------------------------------------------------------------------------------- 
int GetBuiltEvents(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int NumEv[MAX_NBRD][MAX_NCH])
{
	int TotEvntCont = 0;

	if (WDcfg.BuildMode == EVBUILD_CHREF_AND_ANYOTHER) 
		TotEvntCont = BuildEvents_One2All(evnt, NumEv);
	else if (WDcfg.BuildMode == EVBUILD_CLOVER) 
		TotEvntCont = BuildEvents_Clover(evnt, NumEv);
	else
		TotEvntCont = BuildEvents_Singles(evnt, NumEv);
	return TotEvntCont;
}

