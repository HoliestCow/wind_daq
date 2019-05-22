/*******************************************************************************
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
#include "Configure.h"
#include "Console.h"


// --------------------------------------------------------------------------------------------------------- 
// Description: Program the registers of the digitizer with the relevant parameters
// Inputs:		brd = board number
// Outputs:		-
// Return:		Error code (0=success) 
// --------------------------------------------------------------------------------------------------------- 
int ProgramDigitizer(int brd, int SkipCalibration)
{
	int i, ret = 0, stu;  
	int FirstPass = 1;
	char CfgStep[100];
	uint32_t d32;

	// ########################################################################################################
	sprintf(CfgStep, "Reset Digitizer");
	// ########################################################################################################
	ret = CAEN_DGTZ_Reset(handle[brd]);
	if (ret != 0) {
		msg_printf(MsgLog, "ERROR: can't reset the digitizer %d\n", brd);
		goto abortcfg;
	}

	fprintf(MsgLog, "INFO: Configuring Board %d\n", brd);

	if ((WDcfg.DigitizerModel == 724) || (WDcfg.DigitizerModel == 780) || (WDcfg.DigitizerModel == 781)) stu = 10;
	else if (WDcfg.DigitizerModel == 725) stu = 16;
	else stu = 8;
	// correct Tsampl (i.e. sampling period) for decimation 
	if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724)) {
		WDcfg.Tsampl *= (1 << WDcfg.Decimation[0][0]);  // HACK: assuming the same for all channels
		stu *= (1 << WDcfg.Decimation[0][0]);
	}


	// ########################################################################################################
	sprintf(CfgStep, "General System Configuration and Acquisition Mode");
	// ########################################################################################################
	// set the enable mask (16 bit)
	if (WDcfg.EnableMask[brd] == 0) {
		msg_printf(MsgLog, "WARNING: all channels of board %d are disabled!\n", brd);
		if (WDcfg.NumBrd == 1)
			return -1;
		else
			return 0;
	}
	ret |= CAEN_DGTZ_SetChannelEnableMask(handle[brd], WDcfg.EnableMask[brd]);
	WDcfg.TrgoutMask &= WDcfg.EnableMask[brd];  // HACK: must be board by board!

	// Set Fan speed (desktop versions)
	if (WDcfg.DigitizerModel != 5000)
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8168, (WDcfg.FanSpeed<<3) & 0x8);  

	if (WDcfg.DigitizerModel == 5000)
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8000, 0);  // Channel Control Reg (indiv trg, seq readout) ??
	else if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8000, 0x01000114);  // Channel Control Reg (indiv trg, seq readout) ??

	if (WDcfg.DigitizerModel == 751)
		ret |= RegisterSetBits(handle[brd], 0x8000, 31, 31, 1);   // Enable new format

	// settings for DPP firmware only
	if (IS_DPP_FW(WDcfg.DppType)) {
		// Acquisition mode (LIST, OSCILLOSCOPE or MIXED)
		ret |= CAEN_DGTZ_SetDPPAcquisitionMode(handle[brd], (CAEN_DGTZ_DPP_AcqMode_t)WDcfg.AcquisitionMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime); 
		if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725) || (WDcfg.DigitizerModel == 751))
			CAEN_DGTZ_WriteRegister(handle[brd], 0x8004, 1<<17); // Enable Extra Word
		// some specific settings in the global CTRL register
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8004, (1<<2));   // set trgout bidir
		if (WDcfg.DppType == DPP_PSD_751)
			CAEN_DGTZ_WriteRegister(handle[brd], 0x8004, 1<<26); // Set new probe mode in x751 models
		
		if ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PSD_730) || (WDcfg.DppType == DPP_nPHA_724))
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8004, 1);  // Enable auto-flush

		if (WDcfg.DppType == DPP_PSD_720) {
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8004, 1<<17);  // Enable baseline or Extended Time stamp
			ret |= RegisterSetBits(handle[brd], 0x1080, 7, 7, 1);  // Enable ext time stamp
		}
	}

	// Set the number of samples in the waveform (same for all channels)
	ret |= CAEN_DGTZ_SetRecordLength(handle[brd], WDcfg.RecordLength, -1); 
	if ((WDcfg.RecordLength > 30000) && ((WDcfg.DppType == DPP_PHA_730) || (WDcfg.DppType == DPP_PSD_730))) // HACK: Bug Fix: the library doesn't support length > 32K
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8020, WDcfg.RecordLength/8); 

	if (ret) goto abortcfg;


	// ########################################################################################################
	sprintf(CfgStep, "Front Panel I/O, Trigger Modes and Synchronization");
	// ########################################################################################################
	// Set the I/O level (NIM or TTL)
	ret |= CAEN_DGTZ_SetIOLevel(handle[brd], (CAEN_DGTZ_IOLevel_t)WDcfg.FPIOtype);

	// Use external clock in the desktop version
	if (0) {
		msg_printf(MsgLog, "WARNING: using external clock in the desktop/NIM version\n");
		ret |= CAEN_DGTZ_ReadRegister(handle[brd], 0x8100, &d32);   
		ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8100, d32 | (1<<6));
		Sleep (500);
	}

	// settings for the synchronization of the start
	switch (WDcfg.StartMode) {
		case START_MODE_INDEP_SW : 
			ret |= CAEN_DGTZ_SetAcquisitionMode(handle[brd], CAEN_DGTZ_SW_CONTROLLED);
			break;
		case START_MODE_SYNCIN_1ST_SW : 
		case START_MODE_SYNCIN_1ST_HW : 
			if (WDcfg.SyncinMode != SYNCIN_MODE_RUN_CTRL) {
				msg_printf(MsgLog, "WARNING: SyncinMode must be set as RUN_CTRL; forced option\n");
				WDcfg.SyncinMode = SYNCIN_MODE_RUN_CTRL;
			}
			if (WDcfg.TrgoutMode != TRGOUT_MODE_SYNC_OUT) {
				msg_printf(MsgLog, "WARNING: TrgoutMode must be set as SYNC_OUT; forced option\n");
				WDcfg.TrgoutMode = TRGOUT_MODE_SYNC_OUT;
			}
			ret |= CAEN_DGTZ_ReadRegister(handle[brd], CAEN_DGTZ_ACQ_CONTROL_ADD, &d32);    
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_ACQ_CONTROL_ADD, (d32 & 0xFFFFFFF0) | RUN_START_ON_SIN_LEVEL);    // Arm acquisition (Run will start when SIN goes high)
			// Run Delay to deskew the start of acquisition
			if (brd == 0)	ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8170, (WDcfg.NumBrd - 1) * 3 + 1);
			else 			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8170, (WDcfg.NumBrd - brd - 1) * 3);
			break;
		case START_MODE_TRGIN_1ST_SW :
		case START_MODE_TRGIN_1ST_HW :
			ret |= CAEN_DGTZ_ReadRegister(handle[brd], CAEN_DGTZ_ACQ_CONTROL_ADD, &d32);    
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_ACQ_CONTROL_ADD, (d32 & 0xFFFFFFF0) | RUN_START_ON_TRGIN_RISING_EDGE);    // Arm acquisition (Run will start with 1st trigger)
			// Run Delay to deskew the start of acquisition
			if (brd == 0)	ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8170, (WDcfg.NumBrd - 1) * 3 + 1);   
			else 			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8170, (WDcfg.NumBrd - brd - 1) * 3);   
			break;
	}

	// SyncinMode
	if (WDcfg.SyncinMode == SYNCIN_MODE_DISABLED) {
		// there is not any way to disable SIN in the firmware!!! 
	}


	// Trgin mode
	if (WDcfg.DigitizerModel != 5000) {
		if (WDcfg.TrginMode == TRGIN_MODE_DISABLED) {
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, 0x80000000);   // accept SW trg only (ext trig is disabled)
		} else if (WDcfg.TrginMode == TRGIN_MODE_GLOBAL_TRG) {
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, 0xC0000000);   // accept ext trg_in or SW trg
		} else if ((WDcfg.TrginMode == TRGIN_MODE_VETO) || (WDcfg.TrginMode == TRGIN_MODE_GATE)) {
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, 0x80000000);   // accept SW trg only (ext trig is used for the veto/gate)
			ret |= RegisterSetBits(handle[brd], 0x811C, 10, 11, 3);   // propagate ext-trg "as is" to channels (will be used as a validation for the self triggers)
			for (i=0; i<8; i++) 
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + i*4, 0);  // not used
			if (WDcfg.DppType == DPP_PSD_730)
				ret |= RegisterSetBits(handle[brd], 0x8084, 4, 5, 1);	// set individual trgin mode = from MB (not used, masks are disabled)
			if (WDcfg.DppType == DPP_PHA_730)
				ret |= RegisterSetBits(handle[brd], 0x80A0, 4, 5, 1);	// set individual trgin mode = from MB (not used, masks are disabled)

		} else if (WDcfg.TrginMode == TRGIN_MODE_COINC) {  // TrgIn fan out to each channel (individual trg validation)
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, 0x80000000);   // accept SW trg only 
			for (i=0; i<8; i++)
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + i*4, 0x40000000);
		}
	} else {
		if (WDcfg.TrginMode == TRGIN_MODE_DISABLED) {
			//ret |= RegisterSetBits(handle[brd], 0x8048, 2, 2, 0);   // disable external trigger
		} else if (WDcfg.TrginMode == TRGIN_MODE_GLOBAL_TRG)  {
			ret |= RegisterSetBits(handle[brd], 0x8048, 2, 2, 1);   // enable external trigger
			// set trigger mode = propagate (To do)
		} else if (WDcfg.TrginMode == TRGIN_MODE_COINC) {  // 
			ret |= RegisterSetBits(handle[brd], 0x8048, 2, 2, 1);   // enable external trigger
			// set trigger mode = coincidence (To do)
		}
	}


	// TRGOUT/GPO mode
	if (WDcfg.DigitizerModel != 5000) {
		if (WDcfg.TrgoutMode == TRGOUT_MODE_DISABLED) {  // disabled (OR mask=0)
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8110, 0);
		} else if (WDcfg.TrgoutMode == TRGOUT_MODE_PROP_TRGIN) {  // propagate trgin
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8110, 0x40000000);
		} else if (WDcfg.TrgoutMode == TRGOUT_MODE_CH_TRG) {  // propagate self triggers (with mask) 
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8110, WDcfg.TrgoutMask);
		} else if (WDcfg.TrgoutMode == TRGOUT_MODE_SYNC_OUT) {  // propagate sync signal (start/stop)
			ret |= RegisterSetBits(handle[brd], 0x811C, 16, 17, 0x1);    
			ret |= RegisterSetBits(handle[brd], 0x811C, 18, 19, 0x0);    
		} else if (WDcfg.TrgoutMode == TRGOUT_CLOCK) {  // propagate internal clock
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD, 0x00050000);
		} else if (WDcfg.TrgoutMode == TRGOUT_SIGSCOPE) {  // 
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8110, WDcfg.TrgoutMask);
			if (WDcfg.DppType == DPP_PSD_730) {  // replace selftrg with an internal digital probe for debugging (on x730/x725 PSD only)
				ret |= RegisterSetBits(handle[brd], 0x8084, 20, 23, 7); // 1  => overth;
																		// 2  => selftrg;
																		// 3  => pu_trg;
        																// 4  => pu_trg or selftrg;
        																// 5  => veto;
        																// 6  => coincp;
        																// 7  => trgval;
        																// 8  => tvaw;
        																// 9  => npulse;
        																// 10 => gpulse;
			}
		} else {  // Use TRGOUT/GPO as a test pulser
			ret |= RegisterSetBits(handle[brd], 0x811C, 15, 15, 1);
			if (WDcfg.TrgoutMode == TRGOUT_MODE_SQR_1KHZ)  ret |= RegisterSetBits(handle[brd], 0x8168, 0, 2, 1);  // 1 KHz square wave
			if (WDcfg.TrgoutMode == TRGOUT_MODE_PLS_1KHZ)  ret |= RegisterSetBits(handle[brd], 0x8168, 0, 2, 2);  // 1 KHz pulses
			if (WDcfg.TrgoutMode == TRGOUT_MODE_SQR_10KHZ) ret |= RegisterSetBits(handle[brd], 0x8168, 0, 2, 3);  // 10 KHz square wave
			if (WDcfg.TrgoutMode == TRGOUT_MODE_PLS_10KHZ) ret |= RegisterSetBits(handle[brd], 0x8168, 0, 2, 4);  // 10 KHz pulses
		}
	} else {  // Hegagon
		// nothing to do for the moment
	}
	if (ret) goto abortcfg;


	// ########################################################################################################
	sprintf(CfgStep, "Buffer Organization");
	// ########################################################################################################
	// Set Aggregation Mode
	if (IS_DPP_FW(WDcfg.DppType)) {
		ret |= CAEN_DGTZ_SetDPPEventAggregation(handle[brd], WDcfg.EventBuffering, 0);
		ret |= CAEN_DGTZ_SetMaxNumAggregatesBLT(handle[brd], 1023);	// Number of buffers per BLT

	} else {
		ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle[brd], 1023);
	}
	if (0) {  	// manual settings (for debug)
		ret |= CAEN_DGTZ_SetMaxNumAggregatesBLT(handle[brd], 1);	// Number of buffers per BLT
		msg_printf(MsgLog, "WARNING: manual settings of memory segmentation\n");
		if (WDcfg.AcquisitionMode == ACQ_MODE_MIXED) {
			CAEN_DGTZ_WriteRegister(handle[brd], 0x800C, 0x3);		// Buffer organization (8 buffers)
			CAEN_DGTZ_WriteRegister(handle[brd], 0x8034, 1);		// Number of events per buffer
		} else {
			CAEN_DGTZ_WriteRegister(handle[brd], 0x800C, 0xA);		// Buffer organization (1024 buffers)
			CAEN_DGTZ_WriteRegister(handle[brd], 0x8034, 32);		// Number of events per buffer
		}
	}

	if (WDcfg.DigitizerModel != 5000) {
		CAEN_DGTZ_ReadRegister(handle[brd], 0x8000, &d32);
		fprintf(MsgLog, "INFO: 0x8000: Ctrl Reg = %08X\n", d32);
		CAEN_DGTZ_ReadRegister(handle[brd], 0x1020, &d32);
		fprintf(MsgLog, "INFO: 0x1020: RecLength = %d\n", d32);
		CAEN_DGTZ_ReadRegister(handle[brd], 0x100C, &d32);
		fprintf(MsgLog, "INFO: 0x100C: Buffer Organization = %d\n", d32);
		CAEN_DGTZ_ReadRegister(handle[brd], 0x1034, &d32);
		fprintf(MsgLog, "INFO: 0x1034: Num Events x Buff = %d\n", d32);
		CAEN_DGTZ_ReadRegister(handle[brd], 0xEF1C, &d32);
		fprintf(MsgLog, "INFO: 0xEF1C: Num Aggr x BLT = %d\n", d32);
	}

	if (ret) goto abortcfg;



	// ########################################################################################################
	sprintf(CfgStep, "HW Coincidence Mode");
	// ########################################################################################################
	// Hardware coincidence; HACK: this section has been tested only for few cases and board models!!!
	// Principle of operation: when the coincidence are enabled in hardware (FPGA), each channel makes its own trigger (self-trigger) which must be validated
	// by a trigger validation signal coming from the central FPGA in the mother board. There is a trigger mask (one per channel) that defines the logic
	// used to generate the trigger validations on channel basis. This can be the OR, AND or Majority of the self triggers coming from the participating channels.
	// it is also possible to include the external trigger (TRG-IN connector) and the LVDS I/Os.
	// NOTE1: in the x730, two channels (couple) share the same trigger mask and trigger validation signal, so the coincidence takes place on couple basis;
	//        it is also possible to use a local cross-validation (odd channel validates even channel and viceversa)
	// NOTE2: it is possible to negate the coincidence logic, that is use the validation signal to reject events intead of accept (anti-coincidence). The 
	//        logic of the trigger mask does not change
	// NOTE3: besides the individual trigger validation signals, there is a common signal (typically coming from TRG-IN) that can be used as a global inhibit 
	//        or gate for the self triggers. This option is not managed here.

	// INDIVIDUAL_TRIGGER_MASK_REGISTER (address 0x8080 + i*4, i=channel/couple number)
	// [ 7: 0] = participating channel mask (refers to couples in the x730): defines which self-triggers (i.e. from which channels) go into the combination
	// [ 9: 8] = combination logic: 0=OR, 1=AND, 2=MAJORITY
	// [12:10] = majority level
	// [29] = enable LVDS(i) to be ORed in the trigger validation
	// [30] = enable external trigger to be ORed in the trigger validation

	if (((WDcfg.CoincMode > 0) || (WDcfg.TrginMode == TRGIN_MODE_VETO) || (WDcfg.TrginMode == TRGIN_MODE_GATE)) && IS_DPP_FW(WDcfg.DppType)) {
		char Msg[500];
		int paired_channels;  // x730 and x725 boards have paired channel triggers
		int coincmode = 1;
		uint32_t maskindex, vwreg;
		uint32_t DppCtrl2;
		uint32_t ChTrgMask = 0;  // Mask of the channel self-triggers to be used in the coincidence logic (individual trigger validation masks)
		if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) {  // in the x730, 2 channels go in one bit of the mask
			paired_channels = 1;
			for(i=0; i<MAX_NCH; i++) 
				if (WDcfg.EnableInput[brd][i] && (i < WDcfg.NumPhyCh)) ChTrgMask |= (1 << (i/2));
		} else {
			paired_channels = 0;
			ChTrgMask = WDcfg.EnableMask[brd];
		}
		if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
			coincmode = 2;

		for(i=0; i<MAX_NCH; i++) {
			if (WDcfg.EnableInput[brd][i] && (i < WDcfg.NumPhyCh)) { 
				// index of the channel trigger mask for the trigger validation
				maskindex = ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) ? i/2 : i;
				DppCtrl2 =  (WDcfg.DppType == DPP_PHA_730) ? 0x10A0 : 0x1084;

				if (paired_channels) {
					ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 2, 2, 1);  // enable couple trgout
					ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 6, 6, 1);  // enable trg validation input
				}

				// Set Extra Coinc Window Width (for PHA firmware, the extra window width is hardcoded)
				if ((WDcfg.DppType != DPP_PHA_730) && (WDcfg.DppType != DPP_PHA_724) && (WDcfg.DppType != DPP_nPHA_724)) {
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x106C + (i<<8), 9); 
				}

				// Veto Window Register (for x730/x725)
				if (WDcfg.VetoWindow[brd][i] < (float)(65535 * 8))
					vwreg = (uint32_t)(WDcfg.VetoWindow[brd][i]/stu);
				else if (WDcfg.VetoWindow[brd][i] < (float)(65535 * 2000))
					vwreg = 0x10000 | (uint32_t)(WDcfg.VetoWindow[brd][i]/2000);
				else if (WDcfg.VetoWindow[brd][i] < (float)((uint64_t)65535 * 524000))
					vwreg = 0x20000 | (uint32_t)(WDcfg.VetoWindow[brd][i]/524000);
				else
					vwreg = 0x30000 | (uint32_t)(WDcfg.VetoWindow[brd][i]/134000000);


				// program trigger and coincidence logic
				if (WDcfg.TrginMode == TRGIN_MODE_VETO) {  // TRGIN = VETO
					sprintf(Msg, "INFO: external TRGIN acts as a VETO\n");
					ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, 3); // enable anti-coincindence mode (wait for trg validation to reject)
					if ((WDcfg.DppType == DPP_PHA_730) && (WDcfg.FWrev >= 4)) {
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D4 + (i<<8), vwreg);	// gate window width (0 = as long as input signal)
						ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 14, 15, 1);		// set veto source = TRGIN
					} else if (WDcfg.DppType == DPP_PSD_730) {
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D4 + (i<<8), vwreg);	
						ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 18, 19, 1);		// set veto source = TRGIN
					}
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0);
				} else if (WDcfg.TrginMode == TRGIN_MODE_GATE) {  // TRGIN = GATE (= anti-veto)
					sprintf(Msg, "INFO: external TRGIN acts as a GATE\n");
					ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, coincmode); // enable coincindence mode (wait for trg validation to reject)
					if ((WDcfg.DppType == DPP_PHA_730) && (WDcfg.FWrev >= 4)) {
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D4 + (i<<8), vwreg);	// gate window width (0 = as long as input signal)
						ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 14, 15, 1);		// set veto source = TRGIN (antiveto = gate)
					} else if (WDcfg.DppType == DPP_PSD_730) {
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D4 + (i<<8), vwreg);	
					}
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0);
				} else {
					// Channel Trigout Width (it determines the coincidence window). 
					if ((WDcfg.DigitizerModel == 724) || (WDcfg.DigitizerModel == 780) || (WDcfg.DigitizerModel == 781))
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1084 + (i<<8), WDcfg.CoincWindow/10); 
					else if (WDcfg.DppType == DPP_PHA_730)
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1084 + (i<<8), WDcfg.CoincWindow/stu); 
					else
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1070 + (i<<8), WDcfg.CoincWindow/stu);

					switch (WDcfg.CoincMode) {

						case COINC_MAJORITY:  // acquire when the num of fired channels >= Majority Threshold
							if (paired_channels) {
								sprintf(Msg, "INFO: Coincidence with Majority (fired couples >= %d)\n", WDcfg.MajorityLevel);
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);   // couple trgout = OR
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);   // trigger validation comes from the other couples
							} else {
								sprintf(Msg, "INFO: Coincidence with Majority (fired channels >= %d)\n", WDcfg.MajorityLevel);
							}
							ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, coincmode); // enable coincindence mode (wait for trg validation to acquire)
							ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x200 | ChTrgMask | ((WDcfg.MajorityLevel-1) << 10));  
							break;

						case COINC_MINORITY:  // acquire when the num of fired channels < Majority Threshold
							if (paired_channels) {
								sprintf(Msg, "INFO: Coincidence with Minority (fired couples < %d)\n", WDcfg.MajorityLevel);
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);   // couple trgout = OR
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);   // trigger validation comes from the other couples
							} else {
								sprintf(Msg, "INFO: Coincidence with Minority (fired channels < %d)\n", WDcfg.MajorityLevel);
							}
							ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, 3); // enable anti-coincindence mode (wait for trg validation to reject)
							ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x200 | ChTrgMask | ((WDcfg.MajorityLevel-1) << 10));  
							break;

						case COINC_PAIRED_AND: // both channels of a couple acquire when both are fired (0 and 1, 2 and 3, etc...)
							sprintf(Msg, "INFO: Coincidence in Paired Mode: CH[even] AND CH[odd])  \n");
							ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, coincmode); // enable coincindence mode (wait for trg validation to acquire)
							if (paired_channels) {
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0);   // signals from mother-board (inter-couple) are not used
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 2);        // trigger validation comes from the other channel in the couple
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 0);        // couple trgout = AND
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x106C + (i<<8), 2);        // extra-time window shorter incase of paired channels
							} else {
								if ((WDcfg.DigitizerModel == 780) || (WDcfg.DigitizerModel == 790))
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8188 + maskindex*4, 0x10C );
								else
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x100 | (3 << (i & 0xFFFE)) );
							}
							WDcfg.TrgHoldOff = max((2*WDcfg.CoincWindow), WDcfg.TrgHoldOff);
							break;

						case COINC_PAIRED_NAND : // one channel of a couple acquires when the other one is not fired (anti coincidence)
							sprintf(Msg, "INFO: Anti-Coincidence in Paired Mode: Not(CH[even] AND CH[odd])\n");
							ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, 3); // enable anti-coincindence mode (wait for trg validation to reject)
							if (paired_channels) {
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0);   // signals from mother-board (inter-couple) are not used
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 2);        // trigger validation comes from the other channel in the couple
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);        // couple trgout = OR (NOTE: doesn't reflect the acq. trigger!!!)
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x106C + (i<<8), 0);        // don't need extra-time for the coincidence window
							} else {
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x100 | (3 << (i/2)) );
							}
							WDcfg.TrgHoldOff = max((2*WDcfg.CoincWindow), WDcfg.TrgHoldOff);
							break;

						case COINC_PAIRED_OR: // both channels of a couple acquire when one at least on of the two is fired (trigger propagation in the couple) 
							if (WDcfg.DppType == DPP_PHA_724) {
								sprintf(Msg, "WARNING: PAIRED_OR mode is not possible for this DPP_PHA\n");
							} else {
								sprintf(Msg, "INFO: Trigger propagation in Paired Mode: CH[even] OR CH[odd]\n");
								if (WDcfg.DppType != DPP_PHA_724)
									ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // disable self trg
								if (paired_channels) {
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0);   // signals from mother-board (inter-couple) are not used
									ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 3);        // trigger propagation between the channels in the couple
									ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);        // couple trgout = OR (NOTE: doesn't reflect the acq. trigger!!!)
								} else {
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 3 << (i/2) );
									/*if ((i & 1) == 0)
										ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x100 | 2 << i);  // even 
									else
										ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x100 | 1 << (i-1));  // odd*/
								}
							}
							break;

						case COINC_CH0_AND_ANY:  // coinc/anticoinc between a common ref channel (for now, hardcoded to ch 0) and any other channel (but ch 1)
						case COINC_CH0_NAND_ANY:  
							if (WDcfg.CoincMode == COINC_CH0_AND_ANY) {
								ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, coincmode); // enable coincindence mode (wait for trg validation to acquire)
								sprintf(Msg, "INFO: Coincidence one to all: CH[0] AND CH[any]\n");
							} else {
								ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, 3); // enable anti-coincindence mode (wait for trg validation to reject)
								sprintf(Msg, "INFO: Anti-Coincidence one to all: Not(CH[0] AND CH[any])\n");
							}
							if (paired_channels) {
								if (i == 0) {
									ret |= RegisterSetBits(handle[brd], DppCtrl2, 0, 1, 1);  // couple trgout = ch0 only (for couple 0)
									ret |= RegisterSetBits(handle[brd], DppCtrl2, 4, 5, 1);  // trigger validation comes from other couples
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180, 0xFE & ChTrgMask);
								} else if (i == 1) {
									ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // disable self trg (ch1 cannot be used in this coinc mode)
									if (WDcfg.EnableInput[brd][i]) {
										strcat(Msg, "WARNING: Channel 1 cannot be used in COMMON_REFCH mode; disabled\n");
									}
								} else {
									/*ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);  // couple trgout = OR
									ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);  // trigger validation comes from other couples
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x01);*/
									if (WDcfg.CoincMode == COINC_CH0_AND_ANY) {
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);  // couple trgout = OR
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);  // trigger validation comes from other couples
										ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x01);
									} else {
										ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 18, 19, 0); // other channels acquires in singles
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);  // couple trgout = OR
										ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x00);
									}
								}
							} else {
								if (i == 0)	ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180, 0xFE & ChTrgMask);
								else        ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x01);
							}
							WDcfg.TrgHoldOff = max((2*WDcfg.CoincWindow), WDcfg.TrgHoldOff);
							break;

						case COINC_AND_ALL:  // coincidence between all the enabled channels
							sprintf(Msg, "INFO: Coincidence between all enabled channels\n");
							ret |= RegisterSetBits(handle[brd], 0x8080, 18, 19, coincmode); // enable coincindence mode (wait for trg validation to acquire)
							ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x100 | ChTrgMask);
							if (paired_channels) {
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);  // trigger validation comes from other couples
								if  ((i&1) == 0) {
									if (WDcfg.EnableInput[brd][i+1])
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 0);  // couple trgout = AND
									else
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 1);  // couple trgout = even only
								} else {
									if (WDcfg.EnableInput[brd][i-1])
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 0);  // couple trgout = AND
									else
										ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 2);  // couple trgout = odd only
								}
							}
							WDcfg.TrgHoldOff = max((2*WDcfg.CoincWindow), WDcfg.TrgHoldOff);
							break;

						case COINC_OR_ALL:  // any self trigger makes all channels to acquire
							if (WDcfg.DppType == DPP_PHA_724) {
								sprintf(Msg, "WARNING: OR_ALL mode is not possible for this DPP_PHA\n");
							} else {
								sprintf(Msg, "INFO: Trigger propagation from any channel to all\n");
								ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // disable self trg
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, ChTrgMask);
								if (paired_channels) {
									ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);  // trigger propagation comes from other couples
									ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);  // couple trgout = OR
								}
							}
							break;

						case COINC_CH0_TO_ALL:  // all enabled channels acquire if channel 0 is fired (trigger propagation from channel 0 to all)
							if (WDcfg.DppType != DPP_PHA_724) {
								ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // disable self trg
								ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x01);
							} else {
								if (i > 0) {
									ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8180 + maskindex*4, 0x01);
									ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // disable self trg
								}
							}
							if (paired_channels) {
								sprintf(Msg, "INFO: Trigger propagation from channel 0/1 to all\n");
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 0, 1, 3);  // couple trgout = OR
								ret |= RegisterSetBits(handle[brd], DppCtrl2 + (i<<8), 4, 5, 1);  // trigger propagation comes from other couples
							} else {
								sprintf(Msg, "INFO: Trigger propagation from channel 0 to all\n");
							}
							break;
					
						default:
							break;
					}
				}
			}
		}
		msg_printf(MsgLog, Msg);

	}
	if (ret) goto abortcfg;


	// ########################################################################################################
	sprintf(CfgStep, "Channel settings");
	// ########################################################################################################
	FirstPass = 1;
	for(i=0; i<MAX_NCH; i++) {
		if ((WDcfg.EnableInput[brd][i]) && (i < WDcfg.NumPhyCh)) { 

			// DC offset
			// Old setting: WDcfg.DCoffset goes from -50% to 50%; pulse polarity is not taken into account
			// New setting: WDcfg.BaselineDCoffset goes from 0% to 100%, considering pulse polarity
			if (WDcfg.BaselineDCoffset[brd][i] >= 0) {
				uint16_t offs;
				if (WDcfg.PulsePolarity[brd][i] == 1) // negative
					offs = (uint16_t)((WDcfg.BaselineDCoffset[brd][i] * 65535) / 100);
				else  // positive
					offs = (uint16_t)(((100-WDcfg.BaselineDCoffset[brd][i]) * 65535) / 100);
				ret |= CAEN_DGTZ_SetChannelDCOffset(handle[brd], i, offs);
			} else {
				ret |= CAEN_DGTZ_SetChannelDCOffset(handle[brd], i, WDcfg.DCoffset[brd][i]);
			}

			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			// PHA_730/725
			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			if (WDcfg.DppType == DPP_PHA_730) {
				int MM, m, k, b, a, shf, ftd, thr, trgho, tu, tnorm;
				float Tg;

				ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle[brd], i, WDcfg.PreTrigger);  

				tu = (WDcfg.Tsampl * 4) * (1 << WDcfg.Decimation[brd][i]);
				MM =  (int)(WDcfg.TrapPoleZero[brd][i]/tu);
				k =   (int)(WDcfg.TrapRiseTime[brd][i]/tu);
				m =   (int)(WDcfg.TrapFlatTop[brd][i]/tu);
				ftd = (int)(WDcfg.PeakingTime[brd][i]/tu);
				if (ftd > m) {
					msg_printf(MsgLog, "WARNING: PeakingTime > FlatTop; forced PeakingTime = FlatTop (%d ns)\n", WDcfg.TrapFlatTop);
					ftd = m;
				}
				b =   (int)(WDcfg.TTFdelay[brd][i]/(WDcfg.Tsampl * 4));  // NOTE: timing parameters of the RCCR2 filters are not affected by the decimation
				a =   (int)(WDcfg.TTFsmoothing[brd][i]);
				thr = (int)(WDcfg.TrgThreshold[brd][i]);
				trgho =  (int)(WDcfg.TrgHoldOff/stu);
				Tg = (MM * k)/(WDcfg.EnergyCoarseGain[brd][i] * WDcfg.EnergyFineGain[brd][i]);
				// set trapezoid gain and nroma factor
				for (shf=0; shf<32; shf++) {
					if ((1<<shf) > (int)(Tg)) 
					break;
				}
				shf--;
				// normalize trapezoid height
				if (WDcfg.FWrev >= 3) {
					tnorm = (uint32_t)(65535.0 * (1<<shf) / Tg);
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10C4 + (i<<8), tnorm); 
				}


				if (WDcfg.EHnbin > 16384) {
					WDcfg.EHnbin = 16384;
					if (FirstPass)	msg_printf(MsgLog, "WARNING: Can't use 32K energy spectrum for x730/x725: set 16K\n");
				}
				WDcfg.EnergyDiv[brd][i] = (1<<14) / WDcfg.EHnbin;  // HACK: for the moment, the CAENDigitizer library divides the Energy by two to express it as a 14 bit number

				//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1034 + (i<<8), 1);	// nevbuf
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1038 + (i<<8), WDcfg.PreTrigger/4);	// pretrg
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1054 + (i<<8), a);	// a
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1058 + (i<<8), b);	// b
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x105C + (i<<8), k);	// k
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1060 + (i<<8), m);	// m
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1064 + (i<<8), ftd);	// ftd
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1068 + (i<<8), MM);	// MM
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x106C + (i<<8), thr);  // thr
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1070 + (i<<8), 0);	// tww
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1074 + (i<<8), trgho);// trgho
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1078 + (i<<8), 8);	// pkho
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x107C + (i<<8), 4);	// blho
				//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1084 + (i<<8), 4);	// tvaw

				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 0, 5, shf);	// shf
				if (WDcfg.NSPeak[brd][i] > 3) {
					WDcfg.NSPeak[brd][i] = 3;
					msg_printf(MsgLog, "WARNING: option %d for NSPeak is not allowed. Forced to 3 (=64 samples)\n", WDcfg.NSPeak[brd][i]);
				}
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 12, 13, WDcfg.NSPeak[brd][i]);	// nspk
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 16, 16, WDcfg.PulsePolarity[brd][i]);	// inv
				if (WDcfg.TrapNSBaseline[brd][i] > 6) {
					WDcfg.TrapNSBaseline[brd][i] = 6;
					msg_printf(MsgLog, "WARNING: option %d for NsBaseline is not allowed. Forced to 6 (=16384 samples)\n", WDcfg.TrapNSBaseline);
				}
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 20, 22, WDcfg.TrapNSBaseline[brd][i]);	// nsbl
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 8, 9, WDcfg.Decimation[brd][i]);	// decimation
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 26, 26, SysVars.UseRollOverFakeEvents); // enable rollover tracing

				// Discr Mode
				if (WDcfg.FWrev >= 4) {
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D0 + (i<<8), WDcfg.ZeroVoltLevel[brd][i]);	// input baseline level in ADC counts
					if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_DISABLED) {  // Disable self trigger
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1); 
					} else if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_CFD_PHA) {  // CFD_PHA
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 6, 6, 0);	
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 17, 17, 1);
						ret |= RegisterSetBits(handle[brd], 0x10A0 + (i<<8), 12, 13, WDcfg.CFDfraction[brd][i]);
					} else if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_LED_PHA) {  // LED_PHA
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 6, 6, 1);	
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 17, 17, 1);
					} 
				} else if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_DISABLED) {  // Disable self trigger
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1); 
				}

				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1028 + (i<<8), WDcfg.InputDynamicRange[brd][i] & 1);  // 0=2Vpp, 1=0.5Vpp

				// Save and tag vetoed or uncorrelated events (instead of discard)
				if (0)	{
					ret |= RegisterSetBits(handle[brd], 0x10A0 + (i<<8), 19, 19, 1);   
					if (FirstPass) msg_printf(MsgLog, "WARNING: uncorrelated events are saved with tag (not discarded)\n");
				}

				// Set Extra Word = CFD zero crossing samples
				if (SysVars.FineTstampMode == 2)
					ret |= RegisterSetBits(handle[brd], 0x10A0 + (i<<8), 8, 10, 5);   // Extra Word = samples before and after ZC
				else
					ret |= RegisterSetBits(handle[brd], 0x10A0 + (i<<8), 8, 10, 2);   // Extra Word = extended Tstamp + fine Tstamp
				//ret |= RegisterSetBits(handle[brd], 0x10A0 + (i<<8), 8, 10, 7);   // Extra Word = 0x12345678  (debug)

				// Internal Pulse Emulator 
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 14, 14, WDcfg.EnableIPE[brd][i] & 1);	// enable test pulse
				if (WDcfg.FWrev <= 4) {
					uint32_t IPEperiod = WDcfg.IPErandom[brd][i] ? 0xFFE0 : (uint32_t)(10000000/WDcfg.IPEfrequency[brd][i]);
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1044 + (i<<8), (WDcfg.IPErandom[brd][i] << 16) | (IPEperiod & 0xFFFF));  
					//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1048 + (i<<8), (uint32_t)(65536*exp(-1/(WDcfg.IPEdecay[brd][i]*125))) | (WDcfg.IPErisetime[brd][i]<<16) | (WDcfg.IPEaddnoise[brd][i]<<19));
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x104C + (i<<8), WDcfg.IPEamplitude[brd][i]);	  // amplitude
				} else {
					uint32_t rise = WDcfg.IPErisetime[brd][i] & 3;
					uint32_t rand = WDcfg.IPErandom[brd][i] & 1;
					uint32_t decay = WDcfg.IPEdecay[brd][i] & 3;
					uint32_t ampl = WDcfg.IPEamplitude[brd][i] & 3;
					uint32_t noise = WDcfg.IPEaddnoise[brd][i] & 1;
					uint32_t freq = WDcfg.IPEfrequency[brd][i];
					uint32_t reg;
					reg = ampl | (decay << 4) | (rise << 12) | (rand << 14) | (noise << 15) | (freq << 16);
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x8044, reg);	
				}


			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			// PHA_724/780/781/Hexagon
			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			} else if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724)) {
				uint32_t tu, shf, tnorm, dcomp, tau, traprise, ttf_delay;
				float Tg;

				ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle[brd], i, WDcfg.PreTrigger);  // individual

				tu = 10 * (1 << WDcfg.Decimation[brd][i]);
				tau = WDcfg.TrapPoleZero[brd][i]/tu;
				traprise = WDcfg.TrapRiseTime[brd][i]/tu;
				ttf_delay = WDcfg.TTFdelay[brd][i]/tu;
				//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1038 + (i<<8), (WDcfg.PreTrigger));	// pretrg (in samples)
				if (WDcfg.TTFsmoothing[brd][i] == 0) WDcfg.TTFsmoothing[brd][i] = 1;
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1054 + (i<<8), (uint32_t)(WDcfg.TTFsmoothing[brd][i]));	  // a
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1058 + (i<<8), ttf_delay); // b
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x105C + (i<<8), traprise);  // k
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1060 + (i<<8), (uint32_t)(WDcfg.TrapFlatTop[brd][i]/tu));	  // m
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1064 + (i<<8), (uint32_t)(WDcfg.PeakingTime[brd][i]/tu));	  // ftd
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1068 + (i<<8), tau);      // MM
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x106C + (i<<8), (uint32_t)(WDcfg.TrgThreshold[brd][i]));     // thr
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1070 + (i<<8), 0);	    // tww
				if (WDcfg.DppType == DPP_nPHA_724)
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1074 + (i<<8), (uint32_t)((float)WDcfg.TrgHoldOff/tu + 0.5)); // trgho
				else
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1074 + (i<<8), (uint32_t)((float)WDcfg.TrgHoldOff/(8*tu) + 0.5)); // trgho
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1078 + (i<<8), (uint32_t)(WDcfg.PeakHoldOff[brd][i]/(tu*8)));	// pkho  
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x107C + (i<<8), 1000);    // blho
				//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1084 + (i<<8), 4);    // tvaw

				// -----------------------------------
				// Energy gain and input range
				// -----------------------------------
				if ((WDcfg.DigitizerModel == 780) || (WDcfg.DigitizerModel == 781) || (WDcfg.DigitizerModel == 724)) {  // HACK: 724 doesn't support analog gain!!!
					int drange;
					switch (WDcfg.InputDynamicRange[brd][i]) {
						case 0 : drange = 5; break;  // 0.6Vpp
						case 1 : drange = 6; break;  // 1.4Vpp
						case 2 : drange = 9; break;  // 3.7Vpp
						case 3 : drange = 10; break; // 9.5Vpp
					}
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10B4 + (i<<8), drange & 0xF);  
				} else if (WDcfg.DigitizerModel == 5000) {  
					int drange;
					if (0) {
						drange = WDcfg.InputDynamicRange[brd][i];  // direct drive gain register (for debug)
					} else {
						if      (WDcfg.EnergyCoarseGain[brd][i] == 0.25) drange = 0x43;  // 9000 mVpp - Att = 4; Gain = 1
						else if (WDcfg.EnergyCoarseGain[brd][i] == 0.5)	 drange = 0x45;  // 4500 mVpp - Att = 4; Gain = 2
						else if (WDcfg.EnergyCoarseGain[brd][i] == 1)	 drange = 0x42;  // 2250 mVpp - Att = 1; Gain = 1
						else if (WDcfg.EnergyCoarseGain[brd][i] == 2)	 drange = 0x44;  // 1125 mVpp - Att = 1; Gain = 2
						else if (WDcfg.EnergyCoarseGain[brd][i] == 4)	 drange = 0x48;  //  562 mVpp - Att = 1; Gain = 4
						else if (WDcfg.EnergyCoarseGain[brd][i] == 8)	 drange = 0x50;  //  281 mVpp - Att = 1; Gain = 8
						else if (WDcfg.EnergyCoarseGain[brd][i] == 16)	 drange = 0x60;  //  140 mVpp - Att = 1; Gain = 16
						else if (WDcfg.EnergyCoarseGain[brd][i] == 32)	 drange = 0x90;  //   70 mVpp - Att = 1; Gain = 32
						else if (WDcfg.EnergyCoarseGain[brd][i] == 64)	 drange = 0xA0;  //   35 mVpp - Att = 1; Gain = 64
						else if (WDcfg.EnergyCoarseGain[brd][i] == 128)	 drange = 0x110;  //  17 mVpp - Att = 1; Gain = 128
						else if (WDcfg.EnergyCoarseGain[brd][i] == 256)	 drange = 0x120;  //   8 mVpp - Att = 1; Gain = 256
						else if (WDcfg.EnergyCoarseGain[brd][i] == 500)	 drange = 0x13E;  //   4 mVpp - Att = 1; Gain = 500
						else drange = 0x42;  // bad setting: use gain = 1
						WDcfg.EnergyCoarseGain[brd][i] = 1;
					}
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10B4 + (i<<8), drange & 0xFF);  

					// Veto Settings
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10A8 + (i<<8), (uint32_t)(WDcfg.VetoWindow[brd][i]/10));	 // veto window width (0 = as long as input signal)
					ret |= RegisterSetBits(handle[brd], 0x10A4 + (i<<8), 2, 2, 1);	 // enable veto from GPIO
					// AC coupling and PZ comp
					ret |= RegisterSetBits(handle[brd], 0x10BC + (i<<8), 0, 1, WDcfg.ACcoupling[brd][i]);	 // 0=disabled, 1=1.8us, 2=4.7us, 3=10us
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10B0 + (i<<8), WDcfg.ACPZcomp[brd][i]);	// DAC for AC Pole Zero compensation

					// memory test mode (replace adc data with known patterns)
					if (0)
						ret |= RegisterSetBits(handle[brd], 0x8000, 3, 3, 1);
				}

				if (WDcfg.DppType == DPP_PHA_724) {
					if (FirstPass) msg_printf(MsgLog, "WARNING: this firmware version doesn't support Fine Gain\n");
				}
				if (WDcfg.DigitizerModel == 5000) 
					Tg = (tau*traprise)/WDcfg.EnergyFineGain[brd][i];  // coarse gain is hardware
				else
					Tg = (tau*traprise)/(WDcfg.EnergyCoarseGain[brd][i] * WDcfg.EnergyFineGain[brd][i]);
				// set trapezoid gain and nroma factor
				for (shf=0; shf<32; shf++) {
					if ((1<<shf) > (int)(Tg)) 
					break;
				}
				shf--;

				// the board provides energy with 15 bits (32K channels); for the x724/x780 the CAENDigitizer library divide by 2 and gives the energy on 14 bits 
				// calculate rebinning factor to go into WDcfg.EHnbin channels.
				if (WDcfg.DigitizerModel == 5000) 
					WDcfg.EnergyDiv[brd][i] = (1<<15) / WDcfg.EHnbin;  
				else
					WDcfg.EnergyDiv[brd][i] = (1<<14) / WDcfg.EHnbin;  

				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 0, 5, shf);	// shf
				if (WDcfg.NSPeak[brd][i] > 3) {
					WDcfg.NSPeak[brd][i] = 3;
					msg_printf(MsgLog, "WARNING: option %d for NSPeak is not allowed. Forced to 3 (=64 samples)\n", WDcfg.NSPeak[brd][i]);
				}
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 12, 13, WDcfg.NSPeak[brd][i]);	// nspk
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 16, 16, WDcfg.PulsePolarity[brd][i]);	// inv
				//ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 18, 19, 0);	// trgmode
				if (WDcfg.TrapNSBaseline[brd][i] > 6) {
					WDcfg.TrapNSBaseline[brd][i] = 6;
					msg_printf(MsgLog, "WARNING: option %d for NsBaseline is not allowed. Forced to 6 (=16384 samples)\n", WDcfg.TrapNSBaseline[brd][i]);
				}
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 20, 22, WDcfg.TrapNSBaseline[brd][i]);	// nsbl
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 8, 9, WDcfg.Decimation[brd][i]);	// decimation
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 26, 26, SysVars.UseRollOverFakeEvents);	// enable rollover tracing
				//ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 25, 25, 1);	// enable tstamp_reset tracing
				//ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 15, 15, 1);	// Enable Spike Reject
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 29, 29, 1);	// Enable Baseline clipping
				if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_DISABLED) {
					if (WDcfg.DigitizerModel == 5000)
						ret |= RegisterSetBits(handle[brd], 0x1048 + (i<<8), 0, 0, 0);   // Disable self trigger
					else
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // Disable self trigger
				}


				if (WDcfg.DppType == DPP_nPHA_724) {
					// normalize trapezoid height
					tnorm = (uint32_t)(65535.0 * (1<<shf) / Tg);
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10C4 + (i<<8), tnorm); 

					// compensate zero in the TTF (derivative)
					dcomp = (uint32_t)(65536.0 * (1 - exp(-(double)ttf_delay/tau)));
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D0 + (i<<8), dcomp); 

					// Enable TAC mode
					if (0) {
						if (FirstPass) msg_printf(MsgLog, "[WARNING]: TAC mode enabled (trapezoid replaced by input signal)\n");
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 30, 30, 1);
					}

					// energy skimming (use energy filter settings)
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10C8 + (i<<8), (uint32_t)WDcfg.EnergyLCut[brd][i]); 
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10CC + (i<<8), (uint32_t)WDcfg.EnergyUCut[brd][i]); 
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 28, 28, WDcfg.EnableEnergySkim);
				}
	

			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			// PSD_720/751/730/725 + CI_720
			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			} else if ((WDcfg.DppType == DPP_PSD_720) || (WDcfg.DppType == DPP_PSD_730) || (WDcfg.DppType == DPP_PSD_751) || (WDcfg.DppType == DPP_CI)) {

				//if ((WDcfg.DppType == DPP_CI) || (WDcfg.DppType == DPP_PSD_720))
				if (WDcfg.DppType == DPP_CI)
					ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle[brd], -1, WDcfg.PreTrigger);  // common to all channels
				else							
					ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle[brd],  i, WDcfg.PreTrigger);  // individual

				// *****************************************
				// Energy Gain (ChargeSensitivity)
				// *****************************************
				// The PSD and CI firmware uses the parameter ChargeSensitivity to set the conversion gain from charge to 1 LSB (channel) and
				// to define the full scale range of the energy spectrum. However, for many users it is more familiar to use the "CoarseGain" 
				// for this purpose. If the ChargeSensitivity is not set in the config file (default=-1), then the CoarseGain is used to calculate 
				// it automatically according to the convertion tables below.
				// NOTE: in some boards, the ChargeSensitivity goes in steps of 4 instead of 2. In this cases, the intermediate CoarseGains are obtained
				// by a mathematical division by 2 applied in the software (Energy Divider). 
				// The boards (all of them) provide the energy (=charge) with 16 bits (64k channels); the energy value is divided by 2^N to fit into the
				// wanted number of channels (parameter EHnbin).
				// Allowed values for the CoarseGain are 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32 (not all boards support all values)

				// Analog Sensitivity in fC/ch (charge represented with 16 bit => 64K channels)
				//				   CoarseGain =	1/16	1/8		1/4		1/2		1		2		4		8		16		32
				// ------------------------------------------------------------------------------------------------------------
				// x730 - 2 Vpp					2560	1280	640		320		160		80		40		20		10		5
				// x730 - 0.5 Vpp				640		320		160		80		40		20		10		5		2.5		1.25
				// x725 - 2 Vpp					2560	1280	640		320		160		80		40		20		10		---
				// x725 - 0.5 Vpp 				640		320		160		80		40		20		10		5		2.5		---
				// x751 - 1 Vpp 				---		640		320		160		80		40		20		---		---		---
				// x720 - 2 Vpp	  				---		---		640		320		160		80		40		---		---		---

				// ChargeSensitivity Setting - Energy Divider 
				//				   CoarseGain =	1/16	1/8		1/4		1/2		1		2		4		8		16		32
				// ------------------------------------------------------------------------------------------------------------
				// x730 - 2 Vpp					4 - 2	4 - 1	3 - 2	3 - 1	2 - 2	2 - 1	1 - 2	1 - 1	0 - 2	0 - 1
				// x730 - 0.5 Vpp				4 - 2	4 - 1	3 - 2	3 - 1	2 - 2	2 - 1	1 - 2	1 - 1	0 - 2	0 - 1
				// x725 - 2 Vpp					4 - 1	3 - 2	3 - 1	2 - 2	2 - 1	1 - 2	1 - 1	0 - 2	0 - 1	---
				// x725 - 0.5 Vpp 				4 - 1	3 - 2	3 - 1	2 - 2	2 - 1	1 - 2	1 - 1	0 - 2	0 - 1	---
				// x751 - 1 Vpp 				---		5 - 1	4 - 1	3 - 1	2 - 1	1 - 1	0 - 1	---		---		---
				// x720 - 2 Vpp	  				---		---		2 - 1	1 - 2	1 - 1	0 - 2	0 - 1	---		---		---

				// Energy FSR (MeV) measured with a NaI detector (PMT biased @ 700 V)
				//				   CoarseGain =	1/16	1/8		1/4		1/2		1		2		4		8		16		32
				// ------------------------------------------------------------------------------------------------------------
				// x730 - 2 Vpp					800		400		200		100		50		25		12		6		3		1.5
				// x730 - 0.5 Vpp				200		100		50		25		12		6		3		1.5		0.8		0.4
				// x725 - 2 Vpp					800		400		200		100		50		25		12		6		3		---
				// x725 - 0.5 Vpp 				200		100		50		25		12		6		3		1.5		0.8		---
				// x751 - 1 Vpp 				---		200		100		50		25		12		6		---		---		---
				// x720 - 2 Vpp	  				---		---		200		100		50		25		12		---		---		---

				if (WDcfg.ChargeSensitivity[brd][i] < 0) {
					uint16_t gc, rebin, sens = 0, ediv = 1, NotAllowed = 0;

					// find the CoarseGain option (0=1/16, 1=1/8,  ... 4=1, ...  9=32)
					for(gc = 0; gc < 10; gc++)
						if ((uint16_t)(16 * WDcfg.EnergyCoarseGain[brd][i]) <= (1 << gc))
							break;

					// the board provides energy with 16 bits (64K channels); calculate rebinning factor to go into WDcfg.EHnbin channels.
					rebin = (1<<16) / WDcfg.EHnbin;  // Max value for EHnbin = 32K => rebin >= 2

					// convert the coarse gain option (0 to 9) into the relevant setting for the charge sensitivity
					// and for the energy multiplier that takes into account the rebinning and the gaps (2 by 2)
					// in the sensitivity (where present)
					if (WDcfg.DigitizerModel == 730) {
						switch (gc) {
						case 0  : sens = 4; ediv = rebin;   break;  // 1/16        
						case 1  : sens = 4; ediv = rebin/2; break;  // 1/8         
						case 2  : sens = 3; ediv = rebin;   break;  // 1/4         
						case 3  : sens = 3; ediv = rebin/2; break;  // 1/2         
						case 4  : sens = 2; ediv = rebin;   break;  // 1           
						case 5  : sens = 2; ediv = rebin/2; break;  // 2           
						case 6  : sens = 1; ediv = rebin;   break;  // 4           
						case 7  : sens = 1; ediv = rebin/2; break;  // 8           
						case 8  : sens = 0; ediv = rebin;   break;  // 16          
						case 9  : sens = 0; ediv = rebin/2; break;  // 32          
						default : NotAllowed = 1; break;  
						}
					} else if (WDcfg.DigitizerModel == 725) {
						switch (gc) {
						case 0  : sens = 4; ediv = rebin/2;  break;  // 1/16      
						case 1  : sens = 3; ediv = rebin;    break;  // 1/8	      
						case 2  : sens = 3; ediv = rebin/2;  break;  // 1/4	      
						case 3  : sens = 2; ediv = rebin;    break;  // 1/2	      
						case 4  : sens = 2; ediv = rebin/2;  break;  // 1	      
						case 5  : sens = 1; ediv = rebin;    break;  // 2	      
						case 6  : sens = 1; ediv = rebin/2;  break;  // 4	      
						case 7  : sens = 0; ediv = rebin;    break;  // 8	      
						case 8  : sens = 0; ediv = rebin/2;  break;  // 16	      
						case 9  : if (rebin >= 4) {                  // 32
									sens = 0; 
									ediv = rebin/4; 
								  } else NotAllowed = 1;
								  break;  	      
						default : NotAllowed = 1; break;  
						}
					} else if (WDcfg.DigitizerModel == 751) {
						switch (gc) {
						case 1  : sens = 5; ediv = rebin/2; break;  // 1/8	       
						case 2  : sens = 4; ediv = rebin/2; break;  // 1/4	       
						case 3  : sens = 3; ediv = rebin/2; break;  // 1/2	       
						case 4  : sens = 2; ediv = rebin/2; break;  // 1           
						case 5  : sens = 1; ediv = rebin/2; break;  // 2           
						case 6  : sens = 0; ediv = rebin/2; break;  // 4           
						case 7  : if (rebin >= 4) {                 // 8
									sens = 0; 
									ediv = rebin/4; 
								  } else NotAllowed = 1;
								  break;  	      
						case 8  : if (rebin >= 8) {                 // 16
									sens = 0; 
									ediv = rebin/8; 
								  } else NotAllowed = 1;
								  break;  	      
						case 9  : if (rebin >= 16) {                // 32
									sens = 0; 
									ediv = rebin/16; 
								  } else NotAllowed = 1;
								  break;  	      
						default : NotAllowed = 1; break;  
						}
					} else if (WDcfg.DigitizerModel == 720) {
						switch (gc) {
						case 1  : sens = 2; ediv = rebin;    break;  // 1/8	   
						case 2  : sens = 2; ediv = rebin/2;  break;  // 1/4	  	  
						case 3  : sens = 1; ediv = rebin;    break;  // 1/2	  	  
						case 4  : sens = 1; ediv = rebin/2;  break;  // 1			  
						case 5  : sens = 0; ediv = rebin;    break;  // 2			  
						case 6  : sens = 0; ediv = rebin/2;  break;  // 4		
						case 7  : if (rebin >= 4) {                  // 8
									sens = 0; 
									ediv = rebin/4; 
								  } else NotAllowed = 1;
								  break;  	      
						case 8  : if (rebin >= 8) {                  // 16
									sens = 0; 
									ediv = rebin/8; 
								  } else NotAllowed = 1;
								  break;  	      
						case 9  : if (rebin >= 16) {                 // 32
									sens = 0; 
									ediv = rebin/16; 
								  } else NotAllowed = 1;
								  break;  	      
						default : NotAllowed = 1; break;  
						}
					}

					if (NotAllowed) {
						WDcfg.ChargeSensitivity[brd][i] = 0;
						WDcfg.EnergyDiv[brd][i] = 1;
						msg_printf(MsgLog, "WARNING: Value %f for the CoarseGain is not allowed for this board and binning\n", WDcfg.EnergyCoarseGain[brd][i]);
					} else {
						WDcfg.ChargeSensitivity[brd][i] = sens;
						WDcfg.EnergyDiv[brd][i] = ediv;
					}
				} else {
					if (FirstPass) msg_printf(MsgLog, "WARNING: ChargeSensitivity setting is overwriting CoarseGain setting\n");
				}

				// Trigger Threshold
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1060 + (i<<8), WDcfg.TrgThreshold[brd][i]); 

				// Trigger Holdoff
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1074 + (i<<8), WDcfg.TrgHoldOff/stu); 

				// Gate Settings
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1058 + (i<<8), WDcfg.GateWidth[brd][i]/WDcfg.Tsampl); 
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1054 + (i<<8), WDcfg.ShortGateWidth[brd][i]/WDcfg.Tsampl); 
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x105C + (i<<8), WDcfg.PreGate[brd][i]/WDcfg.Tsampl); 

				// Baseline
				if ((WDcfg.DppType == DPP_PSD_751) && (WDcfg.NsBaseline[brd][i] > 7)) {
					WDcfg.NsBaseline[brd][i] = 7;
					if (FirstPass) msg_printf(MsgLog, "WARNING: option %d for NsBaseline is not allowed. Forced to 7 (=512 samples)\n", WDcfg.NsBaseline[brd][i]);
				} 
				if ((WDcfg.DppType != DPP_PSD_751) && (WDcfg.NsBaseline[brd][i] > 4)) {
					WDcfg.NsBaseline[brd][i] = 4;
					if (FirstPass) {
						if (WDcfg.DppType == DPP_PSD_720) msg_printf(MsgLog, "WARNING: option %d for NsBaseline is not allowed. Forced to 4 (=512 samples)\n", WDcfg.NsBaseline[brd][i]);
						else msg_printf(MsgLog, "WARNING: option %d for NsBaseline is not allowed. Forced to 4 (=1024 samples)\n", WDcfg.NsBaseline[brd][i]);
					}
				}
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 20, 22, WDcfg.NsBaseline[brd][i]);
				if ((WDcfg.DppType == DPP_PSD_751) && (WDcfg.NsBaseline[brd][i] != 0))
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1064 + (i<<8), 2); // baseline threshold
				else
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1064 + (i<<8), WDcfg.FixedBaseline[brd][i]); 

				// early baseline freeze (stop baseline caclulation N ns before the start of the gate
				if (0) {
					uint32_t bsl_stop = 500; // time (in ns) before the gate at which the baseline is frozen
					if (WDcfg.DppType == DPP_PSD_730) {
						ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x10D8 + (i << 8), bsl_stop / WDcfg.Tsampl);
						if (FirstPass) msg_printf(MsgLog, "WARNING: early baseline freezing (%d ns before the gate)\n", bsl_stop);
					}
				}

				// digital CFD and interpolated zero crossing (730 only)
				if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_DISABLED) {
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 24, 24, 1);  // Disable self trigger
				} else if ((WDcfg.DppType == DPP_PSD_730) || ((WDcfg.DppType == DPP_PSD_751) && (WDcfg.FWrev >= 8))) {
					ret |= RegisterSetBits(handle[brd], 0x103C + (i<<8), 0, 7, WDcfg.CFDdelay[brd][i]/WDcfg.Tsampl); 
					ret |= RegisterSetBits(handle[brd], 0x103C + (i<<8), 8, 9, WDcfg.CFDfraction[brd][i]); 
					ret |= RegisterSetBits(handle[brd], 0x103C + (i<<8), 10, 11, WDcfg.CFDinterp[brd][i]); 
					// Set Discriminator Mode 0=LED, 1=CFD (x730 only)
					if (WDcfg.DiscrMode[brd][i] == DISCR_MODE_CFD_PSD)
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 6, 6, 1); 
					else
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 6, 6, 0); 
					// Set Extra Word = CFD zero crossing samples
					if (WDcfg.DppType == DPP_PSD_751) {
						if (SysVars.FineTstampMode == 2)
							ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 11, 13, 5);   // Extra Word = samples before and after ZC
						else
							ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 11, 13, 2);   // Extra Word = extended Tstamp + fine Tstamp
					} else {
						if (SysVars.FineTstampMode == 2)
							ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 8, 10, 5);   // Extra Word = samples before and after ZC
						else
							ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 8, 10, 2);   // Extra Word = extended Tstamp + fine Tstamp
					}

					ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 12, 15, WDcfg.TTFsmoothing[brd][i]); 
				}

				// Charge Pedestal
				if (WDcfg.EnablePedestal[brd][i]) {
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i << 8), 4, 4, 1);  // enable pedestal
					if (FirstPass) msg_printf(MsgLog, "WARNING: Charge pedestal enabled. Fixed offset added to the energy values)\n");
				}

				// Charge Sensitivity
				if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725))
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8),  0,  2, WDcfg.ChargeSensitivity[brd][i]);
				else
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8),  0,  1, WDcfg.ChargeSensitivity[brd][i]);

				// Pulse Polarity
				ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 16, 16, WDcfg.PulsePolarity[brd][i]);

				// Disable Trigger hysteresis
				if (0)	ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 30, 30, 1);   

				// Disable Negative trigger detection
				if (0)	ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 31, 31, 1);   

				// Force Baseline calculation to restart at the end of the long gate, regardless the signal over/under thr
				if (0)	ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 15, 15, 1);   

				// Enable Retriggering while busy (used to replicate selftrg onto TRGOUT)
				if (0)	ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 5, 5, 1);   

				// Trigger on input sum (test of a prototype FW)
				if (0) {
					ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 25, 26, 1);   
					if (FirstPass) msg_printf(MsgLog, "WARNING: enabled trigger sum\n");
				}

				// Set fixed charge and extras (for debug)
				if (0)	{
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 3, 3, 1);   
					ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 8, 10, 7);   // Extra Word = 0x12345678
					if (FirstPass) msg_printf(MsgLog, "WARNING: enabled fixed patterns on charge and extras for debug\n");
				}

				// Set zero suppression based on Charge (discard events with Qlong < Qthr)  
				// The ZS is only available for 730/725 starting from FW-rev 130.09
				if ((WDcfg.ChargeLLD[brd][i] > 0) && (WDcfg.DppType == DPP_PSD_730) && (WDcfg.FWrev > 9)) {
					uint32_t Qthr = WDcfg.ChargeLLD[brd][i] * WDcfg.EnergyDiv[brd][i]; 
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 25, 25, 1);   
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1044 + (i<<8), Qthr); 
					if (WDcfg.EnableInput[brd][i])
						msg_printf(MsgLog, "WARNING: Brd %d, Ch %d: enabled zero suppression based on Charge\n", brd, i);
				}

				// use smoothed input for the charge integration
				if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725))
					ret |= RegisterSetBits(handle[brd], 0x1084 + (i<<8), 11, 11, WDcfg.SmoothedForCharge[brd][i]);

				// Internal Pulse Emulator
				if (WDcfg.EnableIPE[brd][i]) {
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 8, 8, 1);
					if (WDcfg.IPEfrequency[brd][i] <= 1)
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 9, 10, 0);
					else if (WDcfg.IPEfrequency[brd][i] <= 10)
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 9, 10, 1);
					else if (WDcfg.IPEfrequency[brd][i] <= 100)
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 9, 10, 2);
					else 
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 9, 10, 3);
				}

				// Set input range for x730 
				if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) {
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1028 + (i<<8), WDcfg.InputDynamicRange[brd][i] & 1);  // 0=2Vpp, 1=0.5Vpp
				}

				// Downgrade emulation
				if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) {
					int dgrade = 0; // 0=14bit, 1=13bit, 2=12bit, 3=10bit. NOTE: the sample is still represented over 14 bits; the FPGA forces 1, 2, or 4 LSBs to zero
					int dsamp = 0;  // 0=500MSps 1=250MSps NOTE: this is not a real downsampling. The FPGA sets odd samples = even samples (s0, s1, s2, s3... become s0, s0, s2, s2...)
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 12, 13, dgrade);  
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 14, 14, dsamp);   
				}

				// Set PSD cut
				if (WDcfg.PSDcutThr[brd][i] != 0) {
					ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x1078 + (i<<8), (uint32_t)(1024*WDcfg.PSDcutThr[brd][i]));   // Set threshold
					if (WDcfg.PSDcutType[brd][i] == 0)
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 27, 28, 2);  // keep neutrons only
					else
						ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 27, 28, 1);  // keep gammas only
				}

				// Pile-Up settings
				// PUR Mode: 0=disabled, 1=reject, 2=close current gate (truncated) and open a new one (x751 only) 
				ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x107c + (i<<8), WDcfg.PurGap[brd][i]); 
				if (WDcfg.PileUpMode[brd][i] == 1) {
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 26, 26, 1); 
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 7, 7, 1);   // Enable the use of Pile-ups to make triggers (for counting ICR or generating trgout pulses)
				} else if ((WDcfg.PileUpMode[brd][i] == 2) && (WDcfg.DigitizerModel == 751)) {
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 25, 25, 1);
				} else {  // PUR disabled
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 26, 26, 0); 
					ret |= RegisterSetBits(handle[brd], 0x1080 + (i<<8), 7, 7, 0);   
					//ret |= CAEN_DGTZ_WriteRegister(handle[brd], 0x107c + (i<<8), 0xFFFF); 
				}

			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			// STD_FW
			// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			} else if (IS_STD_FW(WDcfg.DppType)) {  
				// Set the Pre-Trigger size (actually it is a post-trigger for the STD FW)
				ret |= CAEN_DGTZ_SetPostTriggerSize(handle[brd], 100*(WDcfg.RecordLength - WDcfg.PreTrigger)/WDcfg.RecordLength);  // expressed in percent of the record length

				ret |= CAEN_DGTZ_SetChannelSelfTrigger(handle[brd], CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT, (1<<i));
				if (WDcfg.PulsePolarity[brd][i] == 0) {
					ret |= CAEN_DGTZ_SetChannelTriggerThreshold(handle[brd], i, WDcfg.ZeroVoltLevel[brd][i] + WDcfg.TrgThreshold[brd][i]);
					ret |= CAEN_DGTZ_SetTriggerPolarity(handle[brd], i, CAEN_DGTZ_TriggerOnRisingEdge);  
				} else {
					ret |= CAEN_DGTZ_SetChannelTriggerThreshold(handle[brd], i, WDcfg.ZeroVoltLevel[brd][i] - WDcfg.TrgThreshold[brd][i]);
					ret |= CAEN_DGTZ_SetTriggerPolarity(handle[brd], i, CAEN_DGTZ_TriggerOnFallingEdge);  
				}
			}
		}
		FirstPass = 0;
	}
	if (ret) goto abortcfg;

	// ########################################################################################################
	sprintf(CfgStep, "Generic Write accesses with mask");
	// ########################################################################################################
	for(i=0; i<WDcfg.GWn; i++) {
		if (((int)WDcfg.GWbrd[i] < 0) || (WDcfg.GWbrd[i] == brd)) {
			ret |= CAEN_DGTZ_ReadRegister(handle[brd], WDcfg.GWaddr[i], &d32);
			d32 = (d32 & ~WDcfg.GWmask[i]) | (WDcfg.GWdata[i] & WDcfg.GWmask[i]);
			ret |= CAEN_DGTZ_WriteRegister(handle[brd], WDcfg.GWaddr[i], d32);
		}
	}
	if (ret) goto abortcfg;

	// ########################################################################################################
	sprintf(CfgStep, "Calibration");
	// ########################################################################################################
	if (((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725) || (WDcfg.DigitizerModel == 751)) && !SkipCalibration) {
		msg_printf(MsgLog, "INFO: Calibrating ADCs... ");
		ret |= CAEN_DGTZ_Calibrate(handle[brd]);
		if ((WDcfg.DigitizerModel == 730) || (WDcfg.DigitizerModel == 725)) {
			if (1) {
				for (i=0; i<WDcfg.NumPhyCh; i++) 
					LockTempCalibration_x730(handle[brd], i);
			}
			for (i=0; i<WDcfg.NumPhyCh; i++) {
				CAEN_DGTZ_ReadRegister(handle[brd], 0x1088 + (i<<8), &d32);
				if (!(d32 & 0x8))
					msg_printf(MsgLog, "\nWARNING: calibration not done on ch. %d\n", i);
			}
		}
		Sleep(10);
		msg_printf(MsgLog, "Done\n");
	}

	fprintf(MsgLog, "INFO: Configuration OK\n");
	//SaveRegImage(brd);
	return 0;

	abortcfg:
	msg_printf(MsgLog, "WARNING: Digitizer configuration failed on board %d:\n", brd);
	msg_printf(MsgLog, "WARNING: Error at: %s. Exit Code = %d\n", CfgStep, ret);
	return ret;

}


// --------------------------------------------------------------------------------------------------------- 
// Description: Set virtual probe in the digitizer (analog and digital traces)
// Inputs:		brd = board number
// Outputs:		-
// Return:		Error code (0=success) 
// --------------------------------------------------------------------------------------------------------- 
int SetVirtualProbes(int brd)
{
	int ret=0;

	if (IS_STD_FW(WDcfg.DppType))  // no trace settings with STD FW
		return 0;

	if (WDcfg.WaveformProcessor) {
		ret |= RegisterSetBits(handle[brd], 0x8000, 12, 13, 0); // set trace0 = input
		ret |= RegisterSetBits(handle[brd], 0x8000, 11, 11, 0);	// unset dual trace
		return 0;
	}


	// Analog Probe 0
	if ((TraceSet[0] >= 0) && (TraceSet[0] < 4) && (TraceNames[0][TraceSet[0]][0] != '-')) {
		if (WDcfg.DigitizerModel != 5000)
			ret |= RegisterSetBits(handle[brd], 0x8000, 12, 13, TraceSet[0] & 0x3);
		else
			ret |= RegisterSetBits(handle[brd], 0x800C, 0, 1, TraceSet[0] & 0x3);
	}


	// Analog Probe 1
	if (WDcfg.DigitizerModel != 5000) {
		if ((TraceSet[1] >= 0) && (TraceSet[0] < 4) && (TraceSet[1] < 4) && (TraceNames[1][TraceSet[1]][0] != '-')) {
			RegisterSetBits(handle[brd], 0x8000, 11, 11, 1);	// set dual trace
			if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724))
				ret |= RegisterSetBits(handle[brd], 0x8000, 14, 15, TraceSet[1] & 0x3);
			else if (WDcfg.DppType == DPP_PHA_730)
				ret |= RegisterSetBits(handle[brd], 0x8000, 14, 15, TraceSet[1] & 0x3);
			/*else if ((WDcfg.DppType == DPP_PSD_751) && (WDcfg.FWrev >= 8))
				ret |= RegisterSetBits(handle[brd], 0x8000, 12, 13, TraceSet[1] & 0x3);*/
			else
				ret |= RegisterSetBits(handle[brd], 0x8000, 13, 13, TraceSet[1] & 0x1);
		} else {
			ret |= RegisterSetBits(handle[brd], 0x8000, 11, 11, 0);	// unset dual trace
		}
	} else {
		if ((TraceSet[1] >= 0) && (TraceSet[0] < 4) && (TraceSet[1] < 4) && (TraceNames[1][TraceSet[1]][0] != '-')) {
			RegisterSetBits(handle[brd], 0x800C, 9, 9, 1);	// set dual trace
			ret |= RegisterSetBits(handle[brd], 0x800C, 2, 3, TraceSet[1] & 0x3);
		} else {
			ret |= RegisterSetBits(handle[brd], 0x800C, 9, 9, 0);	// unset dual trace
		}
	}


	// Digital Probe 0 (Trace 3 on plot)
	if ((TraceSet[2] >= 0) && (TraceNames[2][TraceSet[2]][0] != '-')) {
		if (WDcfg.DppType == DPP_PSD_730) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 23, 25, TraceSet[2] & 0x7);
		} else if (WDcfg.DppType == DPP_PHA_730) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 20, 23, TraceSet[2] & 0xF);
		} else if ((WDcfg.DppType == DPP_PSD_720) || (WDcfg.DppType == DPP_CI)) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 20, 22, TraceSet[2] & 0x7);
		} else if ((WDcfg.DppType == DPP_PHA_724) || (WDcfg.DppType == DPP_nPHA_724)) {
			if (WDcfg.DigitizerModel != 5000) 
				ret |= RegisterSetBits(handle[brd], 0x8000, 20, 23, TraceSet[2] & 0xF);
			else
				ret |= RegisterSetBits(handle[brd], 0x800C, 4, 7, TraceSet[2] & 0xF);
		}
	}

	if (WDcfg.DppType == DPP_PSD_751) 
	    ret |= RegisterSetBits(handle[brd], 0x8000, 26, 26, 0); // digital traces 3 and 4 disabled by default

	// Digital Probe 1 (Trace 4 on plot)
	if ((TraceSet[3] >= 0) && (TraceNames[3][TraceSet[3]][0] != '-')) {
		if (WDcfg.DppType == DPP_PSD_730) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 26, 28, TraceSet[3] & 0x7);
		} else if ((WDcfg.DppType == DPP_PSD_720) || (WDcfg.DppType == DPP_CI)) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 23, 25, TraceSet[3] & 0x7);
		} else if (WDcfg.DppType == DPP_PSD_751) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 20, 22, TraceSet[3] & 0xF);
		    ret |= RegisterSetBits(handle[brd], 0x8000, 26, 26, 1); 
		}
	}

	// Digital Probe 2 (Trace 5 on plot)
	if ((TraceSet[4] >= 0) && (TraceNames[4][TraceSet[4]][0] != '-')) {
		if (WDcfg.DigitizerModel == 720) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 23, 25, TraceSet[4] & 0x7);
		} else if (WDcfg.DppType == DPP_PSD_751) {
			ret |= RegisterSetBits(handle[brd], 0x8000, 25, 23, TraceSet[3] & 0xF);
			ret |= RegisterSetBits(handle[brd], 0x8000, 26, 26, 1); 
		}
	} 

	// Digital Probe 3 (Trace 6 on plot)
	if ((TraceSet[5] >= 0) && (TraceNames[4][TraceSet[4]][0] != '-')) {
		if (WDcfg.DigitizerModel == 720)
			ret |= RegisterSetBits(handle[brd], 0x8000, 26, 28, TraceSet[5] & 0x7);
	}

	return ret;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: Set trace options and names for each type of digitizer and DPP
// Return:		Error code (0=success) 
// --------------------------------------------------------------------------------------------------------- 
int SetTraceNames()
{
	int i,j;
	for(i=0; i<MAX_NTRACES; i++) {
		for(j=0; j<MAX_NTRSETS; j++)
			strcpy(TraceNames[i][j], "-");
	}

	if (WDcfg.WaveformProcessor) {
		TraceSet[0] = 0;
		TraceSet[1] = 0;
		TraceSet[2] = 0;
		TraceSet[3] = 0;
		TraceSet[4] = 0;
		TraceSet[5] = 0;
		strcpy(TraceNames[0][0], "Input");
		strcpy(TraceNames[1][0], "SW-WaveProcessor");
		strcpy(TraceNames[2][0], "Trigger");
		strcpy(TraceNames[3][0], "LongGate");
		strcpy(TraceNames[4][0], "ShortGate");
		strcpy(TraceNames[5][0], "BaselineCalc");

	} else {

		switch(WDcfg.DppType) {
			// --------------------------------------------------------------------
			case DPP_PHA_724:
			case DPP_nPHA_724:
				TraceSet[0] = 3;
				TraceSet[1] = 0;
				TraceSet[2] = 4;
				TraceSet[3] = 0;
				TraceSet[4] = -1;
				TraceSet[5] = -1;
				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[0][1], "CR-RC");
				strcpy(TraceNames[0][2], "CR-RC2");
				strcpy(TraceNames[0][3], "Trapezoid");
				strcpy(TraceNames[1][0], "Input");
				strcpy(TraceNames[1][1], "Threshold");
				strcpy(TraceNames[1][2], "Trap-Baseline");
				strcpy(TraceNames[1][3], "Baseline");

				strcpy(TraceNames[2][0],  "none");
				strcpy(TraceNames[2][1],  "Armed");
				strcpy(TraceNames[2][2],  "TrapRunning");
				strcpy(TraceNames[2][3],  "PileUp");
				strcpy(TraceNames[2][4],  "Peaking");
				strcpy(TraceNames[2][5],  "TrgValWindow");
				strcpy(TraceNames[2][6],  "BslFreeze");
				strcpy(TraceNames[2][7],  "TrgHoldOff");
				strcpy(TraceNames[2][8],  "TrgValidation");
				strcpy(TraceNames[2][9],  "OverrangeVeto");
				strcpy(TraceNames[2][10], "ZCwindow");
				strcpy(TraceNames[2][11], "ExtTrigger");
				strcpy(TraceNames[2][12], "Busy");
				strcpy(TraceNames[2][13], "EnergyReady");

				strcpy(TraceNames[3][0], "Trigger");


				break;

			// --------------------------------------------------------------------
			case DPP_CI:
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = 0;
				TraceSet[4] = 0;
				TraceSet[5] = 0;

				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = 0;
				TraceSet[4] = 0;
				TraceSet[5] = 0;

				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[1][0], "Baseline");

				strcpy(TraceNames[2][0], "Trigger");

				strcpy(TraceNames[3][0], "Gate");

				strcpy(TraceNames[4][0], "ExtTrg");
				strcpy(TraceNames[4][1], "OverThr");
				strcpy(TraceNames[4][2], "Trgout");
				strcpy(TraceNames[4][3], "TrgValWindow");
				strcpy(TraceNames[4][4], "PileUp");
				strcpy(TraceNames[4][5], "Coincidence");

				strcpy(TraceNames[5][1], "OverThr");
				strcpy(TraceNames[5][2], "TrgValidation");
				strcpy(TraceNames[5][3], "TrgHoldOff");
				strcpy(TraceNames[5][4], "PileUp");
				strcpy(TraceNames[5][5], "Coincidence");
				break;

			// --------------------------------------------------------------------
			case DPP_PSD_720:
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = 0;
				TraceSet[4] = 0;
				TraceSet[5] = 0;

				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[1][0], "Baseline");

				strcpy(TraceNames[2][0], "Trigger");

				strcpy(TraceNames[3][0], "LongGate");

				strcpy(TraceNames[4][0], "ExtTrg");
				strcpy(TraceNames[4][1], "OverThr");
				strcpy(TraceNames[4][2], "Trgout");
				strcpy(TraceNames[4][3], "TrgValWindow");
				strcpy(TraceNames[4][4], "PileUp");
				strcpy(TraceNames[4][5], "Coincidence");

				strcpy(TraceNames[5][0], "ShortGate");
				strcpy(TraceNames[5][1], "OverThr");
				strcpy(TraceNames[5][2], "TrgValidation");
				strcpy(TraceNames[5][3], "TrgHoldOff");
				strcpy(TraceNames[5][4], "PileUp");
				strcpy(TraceNames[5][5], "Coincidence");
				break;

			// --------------------------------------------------------------------
			case DPP_PSD_751:
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = -1;
				TraceSet[4] = -1;
				TraceSet[5] = -1;

				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[1][0], "Baseline");
				strcpy(TraceNames[1][1], "CFD/TTF");

				strcpy(TraceNames[2][0], "Trigger");

				strcpy(TraceNames[3][0], "LongGate");
				strcpy(TraceNames[3][1], "OverThr");
				strcpy(TraceNames[3][2], "Trgout");
				strcpy(TraceNames[3][3], "TrgValWindow");
				strcpy(TraceNames[3][4], "PileUp");
				strcpy(TraceNames[3][5], "Coincidence");
				strcpy(TraceNames[3][6], "BslFreeze");

				strcpy(TraceNames[4][0], "ShortGate");
				strcpy(TraceNames[4][1], "OverThr");
				strcpy(TraceNames[4][2], "TrgValidation");
				strcpy(TraceNames[4][3], "TrgHoldOff");
				strcpy(TraceNames[4][4], "PileUp");
				strcpy(TraceNames[4][5], "Coincidence");
				break;

			// --------------------------------------------------------------------
			case DPP_PSD_730:
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = 0;
				TraceSet[4] = -1;
				TraceSet[5] = -1;

				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[0][1], "CFD/TTF");
				strcpy(TraceNames[1][0], "Baseline");
				strcpy(TraceNames[1][1], "CFD/TTF");

				strcpy(TraceNames[2][0], "LongGate");
				strcpy(TraceNames[2][1], "OverThr");
				strcpy(TraceNames[2][2], "Trgout");
				strcpy(TraceNames[2][3], "TrgValWindow");
				strcpy(TraceNames[2][4], "PUR_tag");
				strcpy(TraceNames[2][5], "Coincidence");
				strcpy(TraceNames[2][6], "PeakFound");
				strcpy(TraceNames[2][7], "Trigger");

				strcpy(TraceNames[3][0], "ShortGate");
				strcpy(TraceNames[3][1], "OverThr");
				strcpy(TraceNames[3][2], "TrgValidation");
				strcpy(TraceNames[3][3], "TrgHoldOff");
				strcpy(TraceNames[3][4], "PileUpTrigger");
				strcpy(TraceNames[3][5], "IsNeutron");
				strcpy(TraceNames[3][6], "BSLfreeze");
				strcpy(TraceNames[3][7], "Trigger");
				break;

			// --------------------------------------------------------------------
			case DPP_PHA_730:
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = 0;
				TraceSet[3] = 0;
				TraceSet[4] = -1;
				TraceSet[5] = -1;

				strcpy(TraceNames[0][0], "Input");
				strcpy(TraceNames[0][1], "CR-RC");
				strcpy(TraceNames[0][2], "CR-RC2");
				strcpy(TraceNames[0][3], "Trapezoid");
				strcpy(TraceNames[1][0], "Input");
				strcpy(TraceNames[1][1], "Threshold");
				strcpy(TraceNames[1][2], "Trapez-Baseline");
				strcpy(TraceNames[1][3], "Baseline");

				strcpy(TraceNames[2][0], "Peaking");
				strcpy(TraceNames[2][1], "Armed");
				strcpy(TraceNames[2][2], "PkRun");
				strcpy(TraceNames[2][3], "PileUp");
				strcpy(TraceNames[2][4], "Peaking");
				strcpy(TraceNames[2][5], "TrgVal_Window");
				strcpy(TraceNames[2][6], "Bsline_Freeze");
				strcpy(TraceNames[2][7], "Trg_HoldOff");
				strcpy(TraceNames[2][8], "TrgVal");
				strcpy(TraceNames[2][9], "AcqBusy");
				strcpy(TraceNames[2][10], "ZC_Window");
				strcpy(TraceNames[2][11], "ExtTrg");
				strcpy(TraceNames[2][12], "Busy");
				strcpy(TraceNames[2][13], "Veto");

				strcpy(TraceNames[3][0], "Trigger");

				break;

			default:  // Std Firmware
				TraceSet[0] = 0;
				TraceSet[1] = -1;
				TraceSet[2] = -1;
				TraceSet[3] = -1;
				TraceSet[4] = -1;
				TraceSet[5] = -1;

				strcpy(TraceNames[0][0], "Input");
				break;
			}
		}
	return 0;
}
