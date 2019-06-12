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
#include "Readout.h"
#include "Queues.h"
#include "PreProcess.h"
#include "DataFiles.h"
#include "Console.h"

// buffers for the readout
char *buffer;								// readout buffer
void *Events[MAX_NCH];						// memory buffers for the event decoding
CAEN_DGTZ_UINT16_EVENT_t *EventStd;			// generic event struct for Std Firmware

int emptycyc = 0;

int ReadoutOpen = 0;

uint64_t emtstamp = 0;
uint64_t lastemtstamp = 0;

uint32_t AllocatedBoardHandleIndex;

// Readout Log File
FILE *rolog = NULL;

// Waveform buffers
char *WfmBuffer;
Waveform_t *WBwfm[MAX_NUM_WAVEFORMS];

#define ALLOCATE_BUFFER_MANUALLY	0


// --------------------------------------------------------------------------------------------------------- 
// Description: Allocate memory for the waveform data
// Inputs:		Waveform = pointer to the waveform to allocate
//              ns = number of samples in the waveform
// Return: 0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int AllocateWaveform(Waveform_t **Waveform, int ns) 
{
	int i, wsize, nw, wbpnt=0, ret=-1;

	*Waveform = NULL;
	wsize = sizeof(Waveform_t) + ns * (2 * sizeof(uint16_t) + sizeof(uint8_t));
	nw = WFM_BUFFER_SIZE / wsize;
	if (nw > MAX_NUM_WAVEFORMS)
		nw = MAX_NUM_WAVEFORMS;
	if (WBwfm[0] == NULL) {
		for(i=0; i<MAX_NUM_WAVEFORMS; i++) {
			if (i < nw) {
				wbpnt = i*wsize;
				WBwfm[i] = (Waveform_t *)(WfmBuffer + wbpnt);
				wbpnt += sizeof(Waveform_t);
				WBwfm[i]->AnalogTrace[0] = (uint16_t *)(WfmBuffer + wbpnt);
				wbpnt += ns * sizeof(uint16_t);
				WBwfm[i]->AnalogTrace[1] = (uint16_t *)(WfmBuffer + wbpnt);
				wbpnt += ns * sizeof(uint16_t);
				WBwfm[i]->DigitalTraces = (uint8_t *)(WfmBuffer + wbpnt);
				WBwfm[i]->Ns = 0;  // Ns = 0 means that the slot is empty
			} else {
				WBwfm[i] = NULL;
			}
		}
	}

	// search for an empty slot
	for(i=0; i<nw; i++) {
		if (WBwfm[i]->Ns == 0) {
			*Waveform = WBwfm[i];
			WBwfm[i]->Ns = 1; 
			ret = 0;
			break;
		}
	}

	return ret;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Free memory allocated for the waveform data
// Inputs:		Waveform = pointer to the waveform to allocate
// Return: 0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int FreeWaveform(Waveform_t *Waveform) 
{
	int i;
	if (Waveform == NULL)
		return 0;
	for(i=0; i<MAX_NUM_WAVEFORMS; i++) {
		if (WBwfm[i] == NULL)
			return -1;
		if (WBwfm[i] == Waveform) {
			WBwfm[i]->Ns = 0;
			break;
		}
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Initialize variables and allocate memory buffers for the readout and the event queues
// Inputs:		-
// Outputs:		AllocatedMemSize = total num of bytes allocated
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int InitReadout(uint32_t *AllocatedMemSize)
{
	int ret=0, b;
	uint32_t AllocatedSize; 
	
	*AllocatedMemSize=0;

	if (ENROLOG) {
		rolog = fopen("ReadoutLog.txt", "w");
		printf("WARNING: Readout Log File enabled!!!\n");
	}

	// Create Queues
	ret = CreateQueues(&AllocatedSize);
	if (ret < 0)
		return ret;
	*AllocatedMemSize += AllocatedSize;

	ret = InitPreProcess(&AllocatedSize);
	if (ret < 0)
		return ret;
	*AllocatedMemSize += AllocatedSize;


	/* Allocate memory buffers and init statistics                                             */
	/* WARNING: The mallocs MUST be done after the digitizer programming, because the following functions 
	needs to know the digitizer configuration to allocate the right memory amount */
	if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
		uint32_t tmpSize = 0;
		int b,u;
		uint32_t EnableMask;
		void *tmpEvents[MAX_NCH];
		char *tbuf = NULL;

		if (ALLOCATE_BUFFER_MANUALLY) {
			AllocatedSize = 1024*1024*8;	// Fixed size allocation (for debug)
			buffer = (char *)malloc(AllocatedSize);
			msg_printf(MsgLog, "WARNING: readout buffer allocated manually (8 MB)!\n");
		} else {
			// allocate readout buffer for all boards, then keep the largest only
			for (b=0; b<WDcfg.NumBrd; b++) {
				if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE) continue;
				// HACK: workaround to prevent memory allocation bug in the library: allocate for all channels
				ret |= CAEN_DGTZ_GetChannelEnableMask(handle[b], &EnableMask);
				ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8120, 0xFFFF);
				ret |= CAEN_DGTZ_MallocReadoutBuffer(handle[b], &tbuf, (uint32_t *)&AllocatedSize);
				ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8120, EnableMask);
				ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8120, EnableMask);
				if (AllocatedSize > tmpSize) {
					if (tmpSize != 0)
						CAEN_DGTZ_FreeReadoutBuffer(&buffer);
					buffer = tbuf;
					tmpSize = AllocatedSize;
				}
			}
		}
		
		*AllocatedMemSize += AllocatedSize;
		AllocatedBoardHandleIndex = -1;
		tmpSize = 0;
		for (b=0; b<WDcfg.NumBrd; b++) {
			if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE) continue;
			if (IS_DPP_FW(WDcfg.DppType)) {
				ret |= CAEN_DGTZ_GetChannelEnableMask(handle[b], &EnableMask);
				//ret |= CAEN_DGTZ_SetChannelEnableMask(handle[b], 0xFFFF);
				ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8120, 0xFFFF);
				ret |= CAEN_DGTZ_MallocDPPEvents(handle[b], tmpEvents, (uint32_t *)&AllocatedSize);
				//ret |= CAEN_DGTZ_SetChannelEnableMask(handle[b], EnableMask);
				ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8120, EnableMask);
				if (AllocatedSize > tmpSize) {
					if (tmpSize != 0) 
						CAEN_DGTZ_FreeDPPEvents(handle[AllocatedBoardHandleIndex], Events);
					AllocatedBoardHandleIndex = b;
					for (u=0; u<MAX_NCH; u++) 
						Events[u] = tmpEvents[u];
					tmpSize = AllocatedSize;
				}
			} else {
				ret = CAEN_DGTZ_AllocateEvent(handle[b] ,(void **)&EventStd);
				for (u=0; u<MAX_NCH; u++) 
					Events[u] = malloc(1024 * sizeof(uint32_t));
				tmpSize = MAX_NCH * 1024 * sizeof(uint32_t); 
			}
		}
		*AllocatedMemSize += tmpSize;
	}

	if (WDcfg.WaveformEnabled) {
		WfmBuffer = (char *)malloc(WFM_BUFFER_SIZE);
		*AllocatedMemSize += WFM_BUFFER_SIZE;
	} else {
		WfmBuffer = NULL;
	}

	// Clear memory (HACK check)
	if (WDcfg.DigitizerModel == 5000) {
		for (b=0; b<WDcfg.NumBrd; b++) {
			if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE) continue;
			ret |= CAEN_DGTZ_WriteRegister(handle[b], 0x8018, 1);
		}
	}
	ReadoutOpen = 1;
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: free memory 
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CloseReadout()
{
	if (!ReadoutOpen) return 0;
	DestroyQueues();
	ClosePreProcess();

	if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
		if (ALLOCATE_BUFFER_MANUALLY) free(buffer);
		else CAEN_DGTZ_FreeReadoutBuffer(&buffer);
		if (IS_DPP_FW(WDcfg.DppType)) CAEN_DGTZ_FreeDPPEvents(handle[0], Events);
		else CAEN_DGTZ_FreeEvent(handle[0], (void**)&EventStd);
	}
	if (WfmBuffer != NULL) free(WfmBuffer);
	WBwfm[0] = NULL;
	if (rolog != NULL) {
		fprintf(rolog, "Closing Readout Log\n");
		fclose(rolog);
	}
	ReadoutOpen = 0;
	return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: flush incomplete aggregates
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int FlushBuffers(int b)
{
	if (WDcfg.DppType == DPP_PSD_730)
		return CAEN_DGTZ_WriteRegister(handle[b], 0x8040, 0);
	else if ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
		return CAEN_DGTZ_WriteRegister(handle[b], 0x803C, 0);
	else if (WDcfg.DppType == DPP_PSD_751)
		return CAEN_DGTZ_WriteRegister(handle[b], 0x803A, 0);
	else
		return -1;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Read a data block from one board and put the events into the queue
// Inputs:		b = board to read
// Outputs		nb = num of bytes read
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int ReadDataFromBoard(int b, uint32_t *nb)
{ 
	int ret = 0, r, qfull=0;
	uint32_t ch, ev;
	static uint32_t NumEvents[MAX_NCH]; 
	static GenericDPPEvent_t evnt;
	static int evpnt[MAX_NBRD][MAX_NCH];
	static int new_buffer=1;
	Waveform_t *Wfm;

	if (WDcfg.LinkType[b] == VIRTUAL_BOARD_TYPE) {
		*nb = 0;
		return 0;
	}
	if (new_buffer) {
		// Read a Data Block from the board
		ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, nb);
		if (ENROLOG) fprintf(rolog, "\n\nData Readout: %d bytes\n", *nb);
		if (ret) 
			return ret;  // readout error
		if (*nb == 0) {  // no data available from the board => return 0
			emptycyc++;
			if (emptycyc > 10)  // after 10 cycles with no data, make a sleep to prevent heavy CPU load
				Sleep(1);
			// after 100 cycles with no data, try to flush the buffer to get incomplete ones
			if ((emptycyc > 100) && (WDcfg.DigitizerModel == 730)) { // HACK: extend to other models?
				FlushBuffers(b);
				emptycyc = 0;
			}
			return 0;  
		}

		Stats.BlockRead_cnt++;
		Stats.RxByte_cnt += *nb;
		emptycyc = 0;

		// check error bit in the 1st header  // HACK: check all headers in the buffer
		if (SysVars.CheckHeaderErrorBit) {
			uint32_t *header, d32;
			header = (uint32_t *)buffer;
			if (header[1] & 0x04000000) {
				msg_printf(MsgLog, "Severe Error Bit in header (board %d)!!!\n", b);
				CAEN_DGTZ_ReadRegister(handle[b], 0x8178, &d32);
				CAEN_DGTZ_ReadRegister(handle[b], 0x8178, &d32);
			}
		}
	}

	// decode a data block into an array of events (using the CAENDigitizer library); 
	// the event structure is firmware dependent and will be made uniform after the PreProcessor
	if (IS_DPP_FW(WDcfg.DppType)) {
		if (new_buffer) {
			memset(NumEvents, 0, sizeof(uint32_t)*MAX_NCH);
			memset(evpnt, 0, MAX_NBRD*MAX_NCH*sizeof(int));
			ret = CAEN_DGTZ_GetDPPEvents(handle[b], buffer, *nb, Events, NumEvents);
		}
		qfull=0;
		// Convert events into a unique format (GenericDPPEvent_t) and copy them into the queue
		for(ch=0; ch<MAX_NCH; ch++) {
			if ((ENROLOG) && (NumEvents[ch]>0)) fprintf(rolog, "%d Events read for Brd %d - ch %d\n", NumEvents[ch], b, ch);
			for(ev=evpnt[b][ch]; ev<(int)NumEvents[ch]; ev++) { 
				/*if (QueueIsFull(b, ch)) {  // HACK: manage qfull here.
					qfull=1;
					break;
				}*/
				// Pre process event data and fill the GenericDPPEvent_t struct
				if (WDcfg.AcquisitionMode == ACQ_MODE_MIXED) {
					if (AllocateWaveform(&Wfm, WDcfg.RecordLength) < 0)
						Wfm = NULL;  // HACK: no space for the waveform. How to manage??? 
					PreProcessEvent(b, ch, ev, Events[ch], &evnt, Wfm);
				} else {
					PreProcessEvent(b, ch, ev, Events[ch], &evnt, NULL);
				}
				if ((evnt.Flags & EVFLAGS_FAKE) || (evnt.Flags & EVFLAGS_TTRESET)) { // fake event => don't push into queue
					if (evnt.Waveforms != NULL)
						FreeWaveform(evnt.Waveforms);
					continue;
				}
				if (WDcfg.SaveRawData)
					SaveRawData(b, ch, evnt);
				r = PushEvent(b, ch, &evnt);
			}
			evpnt[b][ch] = ev;
		}

	} else {
		char *EventPtr = NULL;
		uint32_t nev, TimeStamp;
		CAEN_DGTZ_EventInfo_t EventInfo;

		ret = CAEN_DGTZ_GetNumEvents(handle[b], buffer, *nb, &nev);
		
		for (ev=0; ev<nev; ev++) {
            ret = CAEN_DGTZ_GetEventInfo(handle[b], buffer, *nb, ev, &EventInfo, &EventPtr);
			ret |= CAEN_DGTZ_DecodeEvent(handle[b], EventPtr, (void**)&EventStd);
			for (ch=0; ch<MAX_NCH; ch++) {
				if ((EventInfo.ChannelMask & (1<<ch)) == 0)
					continue;
				if (AllocateWaveform(&Wfm, WDcfg.RecordLength) < 0)
					Wfm = NULL;  // HACK: no space for the waveform. How to manage??? 
				TimeStamp = (WDcfg.DigitizerModel == 724) ? EventInfo.TriggerTimeTag * 10 : EventInfo.TriggerTimeTag * 8;
				Wfm->Ns = (EventStd->ChSize[ch] > (uint32_t)WDcfg.RecordLength) ? WDcfg.RecordLength : EventStd->ChSize[ch]; // the 751 can give more samples
				Wfm->DualTrace = 0;
				memcpy(Wfm->AnalogTrace[0], EventStd->DataChannel[ch], sizeof(uint16_t) * Wfm->Ns);
				PreProcessEvent(b, ch, ev, (void *)&TimeStamp, &evnt, Wfm);  // event info is TimeStamp only
				if (WDcfg.SaveRawData) 
					SaveRawData(b, ch, evnt);
				r = PushEvent(b, ch, &evnt);
			}
		}
	}
	new_buffer = (qfull) ? 0 : 1;
	if (ret) return ret;

	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Read a data block from data file and put the events into the queue
// Outputs:		nb = num of bytes read
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int ReadDataFromFile(uint32_t *nb)
{
	int i, ret = 0;
	int ch, b;
	uint32_t reclen, wfmt;
	uint16_t info[2];
	int naggr = 0;
	GenericDPPEvent_t evnt;

	*nb=0;
	if (InputDataFile == NULL) 	
		return 0;

	while (1) {
		int nbr, r;
		Waveform_t *Wfm;
		uint64_t ts;

		if (feof(InputDataFile)) {  // reached end of data file
			if (WDcfg.LoopInputFile) {
				OpenInputDataFile();  // restart from begin
				emtstamp = lastemtstamp;
			} else {
				AcqRun = 0;
				break;  // quit loop
			}
		}
		if (WDcfg.InputFileType == INPUTFILE_RAW) {  // raw data
			// Read board and channel number
			nbr = (int)fread(info, sizeof(uint16_t), 2, InputDataFile);  // read baord/channel number and the num of samples (4 bytes)
			if (nbr != 2) break;
			b = (int)((info[0] >> 8) && 0xFF);
			ch = (int)(info[0] & 0xFF);
			reclen = info[1]; // Num of 32 bit words for the waveform (0 if waveforms are disabled)
			if ((ch < 0) || ((int)ch >= MAX_NCH) || (b < 0) || ((int)b >= MAX_NBRD))
				return -1;
			// Read one event from the file
			if ((int)fread(&evnt, sizeof(GenericDPPEvent_t), 1, InputDataFile) < 1)
				return -1;
			evnt.TimeStamp += emtstamp;
			if (lastemtstamp < evnt.TimeStamp)
				lastemtstamp = evnt.TimeStamp;

			// read the waveform and format and write them to the buffer 
			if ((reclen > 0) && WDcfg.WaveformEnabled)  { 
				AllocateWaveform(&Wfm, reclen);
				Wfm->Ns = reclen;
				if ((int)fread(&wfmt, sizeof(uint32_t), 1, InputDataFile) < 1)
					return -1;
				Wfm->DualTrace = (wfmt >> 31) & 0x01;
				for(i=0; i<MAX_NTRACES; i++) {
					Wfm->TraceSet[i] = (wfmt >> (i*4)) & 0xF;
					if (Wfm->TraceSet[i] == 0xF)
						Wfm->TraceSet[i] = -1;
				}
				if ((int)fread(Wfm->AnalogTrace[0], sizeof(uint32_t), reclen/2, InputDataFile) < (reclen/2)) 
					return -1;
				if (Wfm->DualTrace) {
					if ((int)fread(Wfm->AnalogTrace[1], sizeof(uint32_t), reclen/2, InputDataFile) < (reclen/2)) 
						return -1;
				}
				evnt.Waveforms = Wfm;
			} else {
				evnt.Waveforms = NULL;
			}
			*nb += 2 * sizeof(uint16_t) + sizeof(GenericDPPEvent_t);
		} else if (WDcfg.InputFileType == INPUTFILE_BINARY_LIST) {  // list file (BINARY)
			b = 0;
			ch = 0;
			*nb += (uint32_t)(sizeof(uint64_t) * fread(&ts, sizeof(uint64_t), 1, InputDataFile));
			evnt.TimeStamp = ts/1000;
			evnt.FineTimeStamp = ts % 1000;
			*nb += (uint32_t)(sizeof(uint16_t) * fread(&evnt.Energy, sizeof(uint16_t), 1, InputDataFile));
			*nb += (uint32_t)(sizeof(float) * fread(&evnt.psd, sizeof(float), 1, InputDataFile));
			evnt.Waveforms = NULL;
			evnt.Flags = 0;
		} else if (WDcfg.InputFileType == INPUTFILE_ASCII_LIST) {  // list file (ASCII)
			b = 0;
			ch = 0;
			*nb += fscanf(InputDataFile, "%llu", (unsigned long long *)(&ts));
			evnt.TimeStamp = ts/1000;
			evnt.FineTimeStamp = ts % 1000;
			*nb += fscanf(InputDataFile, "%u",	(unsigned int *)(&evnt.Energy));
			*nb += fscanf(InputDataFile, "%f", &evnt.psd);
			evnt.Waveforms = NULL;
			evnt.Flags = 0;
		}
		r = PushEvent(b, ch, &evnt);
		naggr++;
		if ((r == 0) || (naggr == 64))
			break;
	}
	Stats.RxByte_cnt += *nb;
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: create an event (emulation mode) and put it into the queue
// Outputs:		nb = num of bytes created
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
static int DataEmulator(uint32_t *nb)
{
	int i, ret = 0, Coinc;
	int b=0, ch=0;
	uint16_t qs, ql;
	GenericDPPEvent_t evnt;
	Waveform_t *Wfm;
	static int InitEmul = 1;
	static double timestamp[MAX_NCH];
	static uint16_t bsl, ampl;
	static float tau, freq, noise, trise, CoincRatio;  

	if (InitEmul) {
		for(i=0; i<MAX_NCH; i++)
			timestamp[i] = 0.1 + (double)rand()/RAND_MAX;
		WDcfg.DigitizerModel = 0;
		WDcfg.DppType        = DPP_PSD_730;
		//WDcfg.NumBrd         = 1;
		WDcfg.NumPhyCh        = 8;
		WDcfg.Tsampl         = 2;
		WDcfg.Nbit           = 14;	
		WDcfg.WaveformEnabled = (WDcfg.AcquisitionMode == ACQ_MODE_EMULATOR_MIXED) ? 1 : 0;
		bsl = 14000;
		ampl = 4000;
		freq = 10;	// KHz
		tau = 15;	// ns
		noise = 8;	// LSB
		trise = 5;	// ns
		CoincRatio = 30;  // in %
		InitEmul = 0;
	}

	Coinc = (((float)rand()/RAND_MAX) < (CoincRatio/100)) ? 1 : 0;
	for(ch=0; ch<WDcfg.NumPhyCh; ch++) {
		if (!WDcfg.EnableInput[0][ch]) continue;
		evnt.TimeStamp = (uint64_t)(1000*timestamp[ch]);
		evnt.FineTimeStamp = ((uint64_t)(timestamp[ch]/1000000)) % 1024;
		evnt.Flags = 0;
		if (WDcfg.WaveformEnabled)  { 
			qs=0;
			ql=0;
			AllocateWaveform(&Wfm, WDcfg.RecordLength);
			Wfm->Ns = WDcfg.RecordLength;
			Wfm->DualTrace = 0;
			for(i=0; i<WDcfg.RecordLength; i++) {
				uint8_t lg=0, sg=0, trg=0;
				double smp;

				if (i<WDcfg.PreTrigger) 
					smp = bsl;
				else 
					smp = bsl - ampl * (exp(-(i-WDcfg.PreTrigger)/(tau/WDcfg.Tsampl)) - exp(-(i-WDcfg.PreTrigger)/(trise/WDcfg.Tsampl)));
				// Gaussian Noise Generator
				if (noise > 0) {
					double v1, v2, s, x;
					do {
					v1 = 2 * ((double)rand() / RAND_MAX) - 1;           
					v2 = 2 * ((double)rand() / RAND_MAX) - 1;           
						s = v1*v1 + v2*v2;
					}
					while (s >= 1);
					x = sqrt(-2 * log(s) / s) * v1;
					smp += (x * sqrt(noise));
				} 
				Wfm->AnalogTrace[0][i] = (uint16_t)smp;
				Wfm->AnalogTrace[1][i] = 0;


				if (i == WDcfg.PreTrigger) trg = 1;
				if ((i >= (WDcfg.PreTrigger - WDcfg.PreGate[b][ch])) && (i < (WDcfg.PreTrigger - WDcfg.PreGate[b][ch] + WDcfg.GateWidth[b][ch]))) {
					lg = 1;
					ql += bsl - Wfm->AnalogTrace[0][i];
				}
				if ((i >= (WDcfg.PreTrigger - WDcfg.PreGate[b][ch])) && (i < (WDcfg.PreTrigger - WDcfg.PreGate[b][ch] + WDcfg.ShortGateWidth[b][ch]))) {
					sg = 1;
					qs += bsl - Wfm->AnalogTrace[0][i];
				}
				Wfm->DigitalTraces[i] = trg | lg<<1 | sg<<2;
			}
			for(i=0; i<MAX_NTRACES; i++)
				Wfm->TraceSet[i] = 0;
			evnt.Waveforms = Wfm;
			evnt.Energy = ql / 16;
			evnt.psd = (float)(ql-qs)/ql;
			*nb = 12 + 2*WDcfg.RecordLength;
		} else {
			evnt.Waveforms = NULL;
			evnt.Energy = ampl;
			evnt.psd = 0.5;
			*nb = 12;
		}
		PushEvent(0, ch, &evnt);
		if ((ch>0) && Coinc)
			timestamp[ch] = timestamp[0];
		else
			timestamp[ch] += (1000/freq) * (0.5 + (double)rand()/RAND_MAX); 
		Stats.RxByte_cnt += *nb;
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Read data (from all boards) and push events into the queues
// Inputs:		-
// Outputs:		-
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ReadData() 
{
	uint32_t nb;
	int b, ret=0;

	if (ACQ_MODE_PLUGGED(WDcfg.AcquisitionMode)) {
		for(b=0; b<WDcfg.NumBrd; b++) {
			ret = ReadDataFromBoard(b, &nb);
			if (ret < 0) {
				if (!Failure)
					msg_printf(MsgLog, "Data Readout Error\n");
				Failure = 1;
				return -1;    
			}
		}
	} else if (WDcfg.AcquisitionMode == ACQ_MODE_OFFLINE) {
		ret = ReadDataFromFile(&nb);
	} else {
		ret = DataEmulator(&nb);
	}
	return 0;
}

