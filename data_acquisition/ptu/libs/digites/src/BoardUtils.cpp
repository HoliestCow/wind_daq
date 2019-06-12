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
#include "BoardUtils.h"

#include <stdio.h>
#ifdef WIN32
    #include <time.h>
    #include <sys/timeb.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <stdint.h>   /* C99 compliant compilers: uint64_t */
    #include <ctype.h>    /* toupper() */
    #include <sys/time.h>
	#include <time.h>
#endif




// --------------------------------------------------------------------------------------------------------- 
// Description: Set bits of a registers 
// Inputs:		handle = board handle 
//				addr = address of the register
//				start_bit = first bit of the parameter being written 
//				end_bit = last bit of the parameter being written 
//				val: value to write
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int RegisterSetBits(int handle, uint16_t addr, int start_bit, int end_bit, int val)
{
	uint32_t mask=0, reg;
	uint16_t ch;
	int ret;
	int i;

	if (((addr & 0xFF00) == 0x8000) && (addr != 0x8000) && (addr != 0x8004) && (addr != 0x8008)) { // broadcast access to channel individual registers (loop over channels)
		for(ch = 0; ch < WDcfg.NumPhyCh; ch++) {
			ret = CAEN_DGTZ_ReadRegister(handle, 0x1000 | (addr & 0xFF) | (ch << 8), &reg);
			for(i=start_bit; i<=end_bit; i++)
				mask |= 1<<i;
			reg = reg & ~mask | ((val<<start_bit) & mask);
			ret |= CAEN_DGTZ_WriteRegister(handle, 0x1000 | (addr & 0xFF) | (ch << 8), reg);   
		}
	} else { // access to channel individual register or mother board register
		ret = CAEN_DGTZ_ReadRegister(handle, addr, &reg);
		for(i=start_bit; i<=end_bit; i++)
			mask |= 1<<i;
		reg = reg & ~mask | ((val<<start_bit) & mask);
		ret |= CAEN_DGTZ_WriteRegister(handle, addr, reg);   
	}
	return ret;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: Write to internal SPI bus
// Inputs:		handle = board handle 
//				ch = channel
//				address = SPI address
//				value: value to write
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int WriteSPIRegister(int handle, uint32_t ch, uint32_t address, uint32_t value) {
    uint32_t SPIBusy = 1;
	int32_t ret = CAEN_DGTZ_Success;
    
    uint32_t SPIBusyAddr        = 0x1088 + (ch<<8);
    uint32_t addressingRegAddr  = 0x80B4;
    uint32_t valueRegAddr       = 0x10B8 + (ch<<8);

    while(SPIBusy) {
        if((ret = CAEN_DGTZ_ReadRegister(handle, SPIBusyAddr, &SPIBusy)) != CAEN_DGTZ_Success)
            return CAEN_DGTZ_CommError;
        SPIBusy = (SPIBusy>>2)&0x1;
	    if (!SPIBusy) {
            if((ret = CAEN_DGTZ_WriteRegister(handle, addressingRegAddr, address)) != CAEN_DGTZ_Success)
                return CAEN_DGTZ_CommError;	
            if((ret = CAEN_DGTZ_WriteRegister(handle, valueRegAddr, value)) != CAEN_DGTZ_Success)
                return CAEN_DGTZ_CommError;
	    }
        Sleep(1);
    }
    return CAEN_DGTZ_Success;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Read from internal SPI bus
// Inputs:		handle = board handle 
//				ch = channel
//				address = SPI address
// Output		value: read value
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int ReadSPIRegister(int handle, uint32_t ch, uint32_t address, uint32_t *value) {
    uint32_t SPIBusy = 1;
    int32_t ret = CAEN_DGTZ_Success;
    uint32_t SPIBusyAddr        = 0x1088 + (ch<<8);
    uint32_t addressingRegAddr  = 0x80B4;
	uint32_t valueRegAddr       = 0x10B8 + (ch<<8);
	uint32_t val                = 0x0000;

    while(SPIBusy) {
        if((ret = CAEN_DGTZ_ReadRegister(handle, SPIBusyAddr, &SPIBusy)) != CAEN_DGTZ_Success)
            return CAEN_DGTZ_CommError;
        SPIBusy = (SPIBusy>>2)&0x1;
	    if (!SPIBusy) {
            if((ret = CAEN_DGTZ_WriteRegister(handle, addressingRegAddr, address)) != CAEN_DGTZ_Success)
                return CAEN_DGTZ_CommError;	
            if((ret = CAEN_DGTZ_ReadRegister(handle, valueRegAddr, value)) != CAEN_DGTZ_Success)
                return CAEN_DGTZ_CommError;
	    }
        Sleep(1);
    }
    return CAEN_DGTZ_Success;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Lock temperature compensation (i.e. disable dynamic calibration) of the ADC chips in 
//              the x730 and x725 models
// Inputs:		handle = board handle 
//				ch = channel
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int LockTempCalibration_x730(int handle, int ch) 
{
	uint32_t lock, ctrl;
	int ret=0;

	// enter engineering functions
	ret |= WriteSPIRegister(handle, ch, 0x7A, 0x59);
	ret |= WriteSPIRegister(handle, ch, 0x7A, 0x1A);
	ret |= WriteSPIRegister(handle, ch, 0x7A, 0x11);
	ret |= WriteSPIRegister(handle, ch, 0x7A, 0xAC);

	// read lock value
	ret |= ReadSPIRegister (handle, ch, 0xA7, &lock);
	// write lock value
	ret |= WriteSPIRegister(handle, ch, 0xA5, lock);

	// enable lock
	ret |= ReadSPIRegister (handle, ch, 0xA4, &ctrl);
	ctrl |= 0x4;  // set bit 2
	ret |= WriteSPIRegister(handle, ch, 0xA4, ctrl);

	ret |= ReadSPIRegister (handle, ch, 0xA4, &ctrl);
	return ret;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: make a new calibration of the ADC chips in the x730 and x725 models
// Inputs:		handle = board handle 
//				ch = channel
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int CalibrateWithExternalSignal_x730(int handle, int ch)
{
	uint32_t reg;
	uint32_t CoarseOffset[2], FineOffset[2], CoarseGain[2], MediumGain[2], FineGain[2], DiffSkew;
	int ret = 0, i;
	FILE *cf;

	ret |= ReadSPIRegister(handle, ch, 0x20, &CoarseOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x21, &FineOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x22, &CoarseGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x23, &MediumGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x24, &FineGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x26, &CoarseOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x27, &FineOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x28, &CoarseGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x29, &MediumGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x2A, &FineGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x70, &DiffSkew);
	printf("ADC0: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[0], FineOffset[0], CoarseGain[0], MediumGain[0], FineGain[0]);
	printf("ADC1: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[1], FineOffset[1], CoarseGain[1], MediumGain[1], FineGain[1]);
	printf("Differential Skew: %02X\n\n", DiffSkew);

	printf("Send a 50 MHz sine wave to channel %d, then press a key\n", ch);
	getch();

	// enable I2E correction algorithm
	ret |= WriteSPIRegister(handle, ch, 0x31, 0x21);

	for (i = 0; i<10; i++) {
		ret |= ReadSPIRegister(handle, ch, 0x30, &reg);
		if (reg == 0x1C) break;
		Sleep(100);
	}
	if (reg != 0x1C) {
		printf("Input signal is not suitable for calibration\n");
		Sleep(1000);
		return -1;
	}
	// disable I2E correction algorithm
	ret |= WriteSPIRegister(handle, ch, 0x31, 0x20);

	ret |= ReadSPIRegister(handle, ch, 0x20, &CoarseOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x21, &FineOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x22, &CoarseGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x23, &MediumGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x24, &FineGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x26, &CoarseOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x27, &FineOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x28, &CoarseGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x29, &MediumGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x2A, &FineGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x70, &DiffSkew);
	printf("ADC0: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[0], FineOffset[0], CoarseGain[0], MediumGain[0], FineGain[0]);
	printf("ADC1: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[1], FineOffset[1], CoarseGain[1], MediumGain[1], FineGain[1]);
	printf("Differential Skew: %02X\n\n", DiffSkew);
	//cf = fopen("extcal.txt", "w");
	//fprintf(cf, "%2d %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", ch, CoarseOffset[0], FineOffset[0], CoarseGain[0], MediumGain[0], FineGain[0], CoarseOffset[1], FineOffset[1], CoarseGain[1], MediumGain[1], FineGain[1], DiffSkew);
	//fclose(cf);
	getch();

	return ret;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: load calibration of the ADC chips from file
// Inputs:		handle = board handle 
//				ch = channel
// Return:		0=OK, negative number = error code
// --------------------------------------------------------------------------------------------------------- 
int LoadCalibrationFromFile_x730(int handle, int ch)
{
	uint32_t reg;
	uint32_t CoarseOffset[2], FineOffset[2], CoarseGain[2], MediumGain[2], FineGain[2], DiffSkew;
	int ret = 0, i;
	FILE *cf;

	ret |= ReadSPIRegister(handle, ch, 0x20, &CoarseOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x21, &FineOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x22, &CoarseGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x23, &MediumGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x24, &FineGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x26, &CoarseOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x27, &FineOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x28, &CoarseGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x29, &MediumGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x2A, &FineGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x70, &DiffSkew);
	printf("ADC0: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[0], FineOffset[0], CoarseGain[0], MediumGain[0], FineGain[0]);
	printf("ADC1: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[1], FineOffset[1], CoarseGain[1], MediumGain[1], FineGain[1]);
	printf("Differential Skew: %02X\n\n", DiffSkew);


	cf = fopen("extcal.txt", "r");
	fscanf(cf, "%2d %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", &ch, &CoarseOffset[0], &FineOffset[0], &CoarseGain[0], &MediumGain[0], &FineGain[0], &CoarseOffset[1], &FineOffset[1], &CoarseGain[1], &MediumGain[1], &FineGain[1], &DiffSkew);
	fclose(cf);



	//ret |= WriteSPIRegister(handle, ch, 0x31, 0x22);

	ret |= WriteSPIRegister(handle, ch, 0x20, CoarseOffset[0]);
	ret |= WriteSPIRegister(handle, ch, 0x21, FineOffset[0]);
	ret |= WriteSPIRegister(handle, ch, 0x22, CoarseGain[0]);
	ret |= WriteSPIRegister(handle, ch, 0x23, MediumGain[0]);
	ret |= WriteSPIRegister(handle, ch, 0x24, FineGain[0]);
	ret |= WriteSPIRegister(handle, ch, 0x26, CoarseOffset[1]);
	ret |= WriteSPIRegister(handle, ch, 0x27, FineOffset[1]);
	ret |= WriteSPIRegister(handle, ch, 0x28, CoarseGain[1]);
	ret |= WriteSPIRegister(handle, ch, 0x29, MediumGain[1]);
	ret |= WriteSPIRegister(handle, ch, 0x2A, FineGain[1]);
	ret |= WriteSPIRegister(handle, ch, 0x70, DiffSkew);

	ret |= WriteSPIRegister(handle, ch, 0xFE, 0x01);
	Sleep(100);
	ret |= WriteSPIRegister(handle, ch, 0xFE, 0x00);


	//ret |= WriteSPIRegister(handle, ch, 0x31, 0x3D);



	ret |= ReadSPIRegister(handle, ch, 0x20, &CoarseOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x21, &FineOffset[0]);
	ret |= ReadSPIRegister(handle, ch, 0x22, &CoarseGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x23, &MediumGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x24, &FineGain[0]);
	ret |= ReadSPIRegister(handle, ch, 0x26, &CoarseOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x27, &FineOffset[1]);
	ret |= ReadSPIRegister(handle, ch, 0x28, &CoarseGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x29, &MediumGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x2A, &FineGain[1]);
	ret |= ReadSPIRegister(handle, ch, 0x70, &DiffSkew);

	printf("ADC0: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[0], FineOffset[0], CoarseGain[0], MediumGain[0], FineGain[0]);
	printf("ADC1: Offs: %02X %02X - Gain: %02X %02X %02X\n", CoarseOffset[1], FineOffset[1], CoarseGain[1], MediumGain[1], FineGain[1]);
	printf("Differential Skew: %02X\n\n", DiffSkew);

	getch();

	return ret;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Save all the regsiters of the borad to a file
// Inputs:		handle = handle of the board
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SaveRegImage(int handle) 
{
	FILE *regs;
	char fname[100];
	int ret;
	uint32_t addr, reg, ch;

	sprintf(fname, "reg_image_%d.txt", handle);
	regs=fopen(fname, "w");
	if (regs==NULL)
		return -1;

	fprintf(regs, "[COMMON REGS]\n");
	for(addr=0x8000; addr <= 0x8200; addr += 4) {
		ret = CAEN_DGTZ_ReadRegister(handle, addr, &reg);
		if (ret==0) {
			fprintf(regs, "%04X : %08X\n", addr, reg);
		} else {
			fprintf(regs, "%04X : --------\n", addr);
			Sleep(1);
		}
	}
	for(addr=0xEF00; addr <= 0xEF34; addr += 4) {
		ret = CAEN_DGTZ_ReadRegister(handle, addr, &reg);
		if (ret==0) {
			fprintf(regs, "%04X : %08X\n", addr, reg);
		} else {
			fprintf(regs, "%04X : --------\n", addr);
			Sleep(1);
		}
	}
	for(addr=0xF000; addr <= 0xF088; addr += 4) {
		ret = CAEN_DGTZ_ReadRegister(handle, addr, &reg);
		if (ret==0) {
			fprintf(regs, "%04X : %08X\n", addr, reg);
		} else {
			fprintf(regs, "%04X : --------\n", addr);
			Sleep(1);
		}
	}

	for(ch=0; ch<(uint32_t)WDcfg.NumPhyCh; ch++) {
		fprintf(regs, "[CHANNEL %d]\n", ch);
		for(addr=0x1000+(ch<<8); addr <= (0x10FF+(ch<<8)); addr += 4) {
			if (addr != 0x1090+(ch<<8)) {
				ret = CAEN_DGTZ_ReadRegister(handle, addr, &reg);
				if (ret==0) {
					fprintf(regs, "%04X : %08X\n", addr, reg);
				} else {
					fprintf(regs, "%04X : --------\n", addr);
					Sleep(1);
				}
			}
		}
	}

	fclose(regs);
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Read Board Information
// Inputs:		b=board index
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ReadBoardInfo(int b, char *ConnectString)
{
	int ret, i;
	uint32_t d32;
	int MajorNumber, MinorNumber;
	CAEN_DGTZ_BoardInfo_t BoardInfo;			// struct with the board information

	/* Once we have the handler to the digitizer, we use it to call the other functions */
	ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
	if (ret) {
		msg_printf(MsgLog, "ERROR: Can't read board info\n");
		return -1;
	}

	WDcfg.NumPhyCh = BoardInfo.Channels;
	WDcfg.EnableMask[b] = 0;
	for(i=0; i<MAX_NCH; i++)
		if ((WDcfg.EnableInput[b][i]<<i) && (i < WDcfg.NumPhyCh)) WDcfg.EnableMask[b] |= (1<<i);


	sprintf(ConnectString, "%s, %d, %d, ", BoardInfo.ModelName, BoardInfo.Channels, BoardInfo.SerialNumber);
	if (BoardInfo.FamilyCode == 5) {
		WDcfg.DigitizerModel = 751;
		WDcfg.Tsampl = 1;
		WDcfg.Nbit = 10;
	} else if (BoardInfo.FamilyCode == 7) {
		WDcfg.DigitizerModel = 780;
		WDcfg.Tsampl = 10;
		WDcfg.Nbit = 14;
	} else if (BoardInfo.FamilyCode == 13) {
		WDcfg.DigitizerModel = 781;
		WDcfg.Tsampl = 10;
		WDcfg.Nbit = 14;
	} else if (BoardInfo.FamilyCode == 0) {
		WDcfg.DigitizerModel = 724;
		WDcfg.Tsampl = 10;
		WDcfg.Nbit = 14;
	} else if (BoardInfo.FamilyCode == 11) {
		WDcfg.DigitizerModel = 730;
		WDcfg.Tsampl = 2;
		WDcfg.Nbit = 14;
	} else if (BoardInfo.FamilyCode == 14) {
		WDcfg.DigitizerModel = 725;
		WDcfg.Tsampl = 4;
		WDcfg.Nbit = 14;
	} else if (BoardInfo.FamilyCode == 3) {
		WDcfg.DigitizerModel = 720;
		WDcfg.Tsampl = 4;
		WDcfg.Nbit = 12;
	} else if (BoardInfo.FamilyCode == 12) {
		WDcfg.DigitizerModel = 720;
		WDcfg.Tsampl = 4;
		WDcfg.Nbit = 12;
	} else if (BoardInfo.FamilyCode == 999) {  // temporary code for Hexagon
		WDcfg.DigitizerModel = 5000;
		WDcfg.Tsampl = 10;
		WDcfg.Nbit = 14;
	} else {
		msg_printf(MsgLog, "ERROR: Unknown digitizer model\n");
		return -1;
	}

	/* Check firmware revision (only DPP firmware can be used with this Demo) */
	sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
	sscanf(&BoardInfo.AMC_FirmwareRel[4], "%d", &MinorNumber);
	WDcfg.FWrev = MinorNumber;
	if (WDcfg.DigitizerModel == 5000) { // Hexagon
		sprintf(WDcfg.FwTypeString, "DPP_PHA_Hexagon");
		WDcfg.DppType = DPP_nPHA_724;
		strcat(ConnectString, "DPP_PHA_Hexagon, ");
	} else if (MajorNumber == 128) {
		if (MinorNumber >= 0x40) {
			sprintf(WDcfg.FwTypeString, "DPP_PHA (new version)");
			WDcfg.DppType = DPP_nPHA_724;
			strcat(ConnectString, "DPP_nPHA_724, ");
		} else {
			sprintf(WDcfg.FwTypeString, "DPP_PHA");
			WDcfg.DppType = DPP_PHA_724;
			strcat(ConnectString, "DPP_PHA_724, ");
		}
	} else if (MajorNumber == 130) {
		sprintf(WDcfg.FwTypeString, "DPP_CI");
		WDcfg.DppType = DPP_CI;
		strcat(ConnectString, "DPP_CI_720, ");
	} else if (MajorNumber == 131) {
		sprintf(WDcfg.FwTypeString, "DPP_PSD");
		WDcfg.DppType = DPP_PSD_720;
		strcat(ConnectString, "DPP_PSD_720, ");
	} else if (MajorNumber == 132) {
		sprintf(WDcfg.FwTypeString, "DPP_PSD");
		WDcfg.DppType = DPP_PSD_751;
		strcat(ConnectString, "DPP_PSD_751, ");
	} else if (MajorNumber == 136) { 
		sprintf(WDcfg.FwTypeString, "DPP_PSD");
		WDcfg.DppType = DPP_PSD_730;  // NOTE: valid also for x725
		strcat(ConnectString, "DPP_PSD_730, ");
	} else if (MajorNumber == 139) { 
		sprintf(WDcfg.FwTypeString, "DPP_PHA");
		WDcfg.DppType = DPP_PHA_730;  // NOTE: valid also for x725
		strcat(ConnectString, "DPP_PHA_730, ");
	} else {
		sprintf(WDcfg.FwTypeString, "Raw Waveforms (Std. FW)");
		WDcfg.DppType = STD_730;
		WDcfg.AcquisitionMode = ACQ_MODE_MIXED;
		WDcfg.WaveformEnabled = 1;
		strcat(ConnectString, "Std FW, ");
	}
	msg_printf(MsgLog, "INFO: Brd %d: CAEN Digitizer Model %s (s.n. %d)\n", b, BoardInfo.ModelName, BoardInfo.SerialNumber);
	msg_printf(MsgLog, "INFO: ROC FPGA: %s\n", BoardInfo.ROC_FirmwareRel);
	msg_printf(MsgLog, "INFO: AMC FPGA: %s (%s)\n", BoardInfo.AMC_FirmwareRel, WDcfg.FwTypeString);
	sprintf(ConnectString, "%s %s, %s, ", ConnectString, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel);

	if (IS_DPP_FW(WDcfg.DppType) && (WDcfg.DigitizerModel != 5000)) {
		CAEN_DGTZ_ReadRegister(handle[b], 0x8158, &d32);
		if (d32 == 0x53D4) {
			msg_printf(MsgLog, "INFO: The DPP is licensed\n");
			strcat(ConnectString, "YES");
		} else {
			if (d32 > 0) {
				msg_printf(MsgLog, "WARNING: DPP not licensed: %d minutes remaining\n", (int)((float)d32/0x53D4 * 30));
			} else {
				msg_printf(MsgLog, "ERROR: DPP not licensed: time expired\n\n");
				return -1;
			}
			strcat(ConnectString, "NO");
		}
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: force clock sync in one board
// Inputs:		b = board index
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ForceClockSync(int b)
{    
    int ret;
    Sleep(500);
    /* Force clock phase alignment */
    ret = CAEN_DGTZ_WriteRegister(handle[b], 0x813C, 1);
    /* Wait an appropriate time before proceeding */
    Sleep(2000);
    return ret;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: wait until the acquisition is started
// Return:		1=Acquisition Started, 0=Acquisition not started -1=error
// --------------------------------------------------------------------------------------------------------- 
int WaitForAcquisitionStarted(int b) 
{
	int ret=0;
	uint32_t d32;

	do {
		ret = CAEN_DGTZ_ReadRegister(handle[b], CAEN_DGTZ_ACQ_STATUS_ADD, &d32);    // Read run status
		if (ret < 0)
			return ret;
		if (kbhit()) {
			getch();
			break;
		}
		Sleep(1);
	} while ((d32 & 0x4) == 0);
	if (d32 & 0x4)
		return 1;
	else
		return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: start the acquisition 
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int StartAcquisition() 
{
	int b;
	int Started = 0;
	uint32_t d32a[MAX_NBRD], d32b[MAX_NBRD];
	time_t timer;
    struct tm* tm_info;

	// StartMode 1: use the TRGIN-TRGOUT daisy chain; the 1st trigger starts the acquisition
	if ((WDcfg.StartMode == START_MODE_TRGIN_1ST_SW) || ((WDcfg.StartMode == START_MODE_TRGIN_1ST_HW) )) {
		for(b=0; b<WDcfg.NumBrd; b++) {
			uint32_t mask;
			if ((b == 0) && (WDcfg.StartMode == START_MODE_TRGIN_1ST_SW)) mask = 0x80000000;
			else mask = 0x40000000;
			CAEN_DGTZ_ReadRegister(handle[b], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, &d32a[b]);
			CAEN_DGTZ_ReadRegister(handle[b], CAEN_DGTZ_FP_TRIGGER_OUT_ENABLE_ADD, &d32b[b]);
			CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, mask);
			CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_FP_TRIGGER_OUT_ENABLE_ADD, mask);
		}

		if (WDcfg.StartMode == START_MODE_TRGIN_1ST_HW) {
			printf("Boards armed. Waiting for TRGIN signal to start run (press a key to force)\n");
			Started = WaitForAcquisitionStarted(WDcfg.NumBrd-1);
		}
		if (!Started)
			CAEN_DGTZ_SendSWtrigger(handle[0]); /* Send a software trigger to the 1st board to start the acquisition */

		/* set the registers back to the original settings */
		/*for(b=0; b<WDcfg.NumBrd; b++) {
			CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD, d32a[b]);
			CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_FP_TRIGGER_OUT_ENABLE_ADD, d32b[b]);
		}*/

	// StartMode 2: use SIN-TRGOUT daisy chain; the 1st board waits for an external logic level to start the acquisition
	} else if ((WDcfg.StartMode == START_MODE_SYNCIN_1ST_SW) || (WDcfg.StartMode == START_MODE_SYNCIN_1ST_HW)) {
		if (WDcfg.StartMode == START_MODE_SYNCIN_1ST_HW) {
			printf("Boards armed. Waiting for SIN/GPI signal to start run (press a key to force)\n");
			Started = WaitForAcquisitionStarted(WDcfg.NumBrd-1);
		}
		if (!Started) {
			uint32_t d32;
			//CAEN_DGTZ_SetAcquisitionMode(handle[0], CAEN_DGTZ_SW_CONTROLLED);
			CAEN_DGTZ_ReadRegister(handle[0], CAEN_DGTZ_ACQ_CONTROL_ADD, &d32);    
			CAEN_DGTZ_WriteRegister(handle[0], CAEN_DGTZ_ACQ_CONTROL_ADD, (d32 & 0xFFFFFFF8) | 0);    // SW controlled
			CAEN_DGTZ_SWStartAcquisition(handle[0]);
		}

	// StartMode 3: start the boards one by one with a software command (the time stamps of the boards won't be synchronized)
	} else {
		for(b=0; b<WDcfg.NumBrd; b++) 
			CAEN_DGTZ_SWStartAcquisition(handle[b]);
	}

	// string with start time and date
    time(&timer);
    tm_info = localtime(&timer);
    strftime(Stats.AcqStartTimeString, 26, "%Y-%m-%d %H:%M:%S", tm_info);

	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: stop the acquisition 
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int StopAcquisition()
{
	int b;
	// Note: in case the SIN-TRGOUT daisy chain is used to start/stop the run, stopping the 1st board will stop the acquisition in all of the boards
	// simultaneously; however, the CAEN_DGTZ_SWStopAcquisition function must be callod for the other boards too because they need to be disarmed
	msg_printf(MsgLog, "INFO: Stopping Acquisition\n");
	for(b=0; b<WDcfg.NumBrd; b++)
		CAEN_DGTZ_SWStopAcquisition(handle[b]); 
	return 0;
}




// --------------------------------------------------------------------------------------------------------- 
// Description: Manual Settings of the DC/DC switching frequency in Hexagon
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int Manage_DCDC()
{
    int quit=0, b=0, ch=0, c=0, ret=0;
	long time, ptime;

	uint32_t m_address = 0xD000;
	uint32_t f_address = 0xD004;
	uint32_t d_address = 0xD008;

	uint32_t data = 0;

	uint32_t fcore   = 0; 
	uint32_t fvan    = 0;  
	uint32_t fvccio  = 0;
	uint32_t fvadc   = 0;
	uint32_t dcore   = 0; 
	uint32_t dvan    = 0;  
	uint32_t dvccio  = 0;
	uint32_t dvadc   = 0;
	uint32_t en_mask = 0;


	ptime = get_time();
	time = ptime;

	while (!quit) {
		c = 0;
		time = get_time();
		if ((time - ptime) > 1000) {
			ptime = time;
			ClearScreen();
			//////////////////////
			//    Data stored   //
			//////////////////////
			// Freq
			CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
			fcore = (data & 0xFF); 
			CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
			fvan = (data & 0xFF00) >> 8; 
			CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
			fvccio = (data & 0xFF0000) >> 16; 
			CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
			fvadc = (data & 0xFF000000) >> 24;
			// Delay
			CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
			dcore = (data & 0xFF); 
			CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
			dvan = (data & 0xFF00) >> 8; 
			CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
			dvccio = (data & 0xFF0000) >> 16; 
			CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
			dvadc = (data & 0xFF000000) >> 24;
			// Mask
			CAEN_DGTZ_ReadRegister(handle[b], m_address,&data);
			en_mask = data;
			printf("\n\nDC-DC Converter Setting Panel\n\n");
			printf("a   : change f-vcore   %5d [C.C.]\n", (int)fcore);
			printf("b   : change f-van     %4d [C.C.]\n" , (int)fvan);
			printf("c   : change f-vccio   %6d [C.C.]\n" , (int)fvccio);
			printf("d   : change f-vadc    %5d [C.C.]\n\n" , (int)fvadc);
			printf("e   : change d-vcore   %5d [C.C.]\n", (int)dcore);
			printf("f   : change d-van     %4d [C.C.]\n" , (int)dvan);
			printf("g   : change d-vccio   %6d [C.C.]\n" , (int)dvccio);
			printf("h   : change d-vadc    %5d [C.C.]\n\n" , (int)dvadc);
			printf("m   : mask ON/OFF  0x%x [Hex]\n", en_mask);			
			printf("q   : Quit DC-DC Converter controller\n\n");
		}
		if (kbhit())
			c = getch();
		switch (c) {
			case 'a':
				printf("Enter new Vcore frequency [C.C.]: ");
				scanf("%i", &fcore);
				CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
				data = (data& 0xFFFFFF00) + fcore; 
				CAEN_DGTZ_WriteRegister(handle[b], f_address, (uint32_t)(data));
				break;
			case 'b':
				printf("Enter new Van frequency [C.C.]: ");
				scanf("%i", &fvan);
				CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
				data = (data& 0xFFFF00FF) + (fvan << 8) ; 
				CAEN_DGTZ_WriteRegister(handle[b], f_address, (uint32_t)(data));
				break;
			case 'c':
				printf("Enter new Vccio frequency [C.C.]: ");
				scanf("%i", &fvccio);
				CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
				data = (data& 0xFF00FFFF) + (fvccio << 16); 
				CAEN_DGTZ_WriteRegister(handle[b], f_address, (uint32_t)(data));
				break;	
			case 'd':
				printf("Enter new Vadc frequency [C.C.]: ");
				scanf("%i", &fvadc);
				CAEN_DGTZ_ReadRegister(handle[b], f_address,&data);
				data = (data& 0x00FFFFFF) + (fvadc << 24); 
				CAEN_DGTZ_WriteRegister(handle[b], f_address, (uint32_t)(data));
				break;	
			case 'e':
				printf("Enter new Delay Vcore: ");
				scanf("%i", &dcore);
				CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
				data = (data& 0xFFFFFF00) + dcore; 
				CAEN_DGTZ_WriteRegister(handle[b], d_address, (uint32_t)(data));
				break;	
			case 'f':
				printf("Enter new Delay Van: ");
				scanf("%i", &dvan);
				CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
				data = (data& 0xFFFF00FF) + (dvan << 8); 
				CAEN_DGTZ_WriteRegister(handle[b], d_address, (uint32_t)(data));
				break;	
			case 'g':
				printf("Enter new Delay Van: ");
				scanf("%i", &dvccio);
				CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
				data = (data& 0xFF00FFFF) + (dvccio << 16); 
				CAEN_DGTZ_WriteRegister(handle[b], d_address, (uint32_t)(data));
				break;	
			case 'h':
				printf("Enter new Delay Vadc: ");
				scanf("%i", &dvadc);
				CAEN_DGTZ_ReadRegister(handle[b], d_address,&data);
				data = (data& 0x00FFFFFF) + (dvadc << 24); 
				CAEN_DGTZ_WriteRegister(handle[b], d_address, (uint32_t)(data));
				break;	
			case 'm':
				printf("Enter Enable mask: ");
				scanf("%x", &en_mask);
				CAEN_DGTZ_WriteRegister(handle[b], m_address, (uint32_t)(en_mask));
				break;	
			case 'q':
				quit=1;
			default:
				break;
		}
		Sleep(100);
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Manual Control for a direct access to the board regsiters and for other funtions
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int ManualController()
{
	int b=0, c, qmc=0, lastop=0, ch;
	char lops[100];
	char *buff;
	uint32_t rdata[MAX_NBRD];
	uint32_t nb;
	uint64_t tnb=0;
	long time, ptime;
	uint32_t ra, rd;

	while (!qmc) {
		ClearScreen();
		printf("\n\nManual R/W access\n\n");
		printf("Board %d:\n", b);
		printf("w : Write register\n");
		printf("r : Read register\n");
		printf("o : Repeat last operation\n");
		printf("i : Write Register Image\n");
		printf("t : Throughput Test\n");
		printf("k : Propagate CLK to trgout on all boards\n");
		printf("b : Change board\n");
		printf("d : Open DC/DC control panel (for Hexagon only)\n");
		printf("c : Calibrate ADCs\n");
		printf("C : Calibrate ADCs with external signal\n");
		printf("R : Read SPI register\n");
		printf("W : Write SPI register\n");
		printf("q : Quit manual register access\n\n");
		if (lastop > 0) {
			int i;
			printf("Last Operation: %s Addr 0x%04X,  Data = 0x%08X (%d dec)\n\n", lops, ra, rd, rd);
			printf("  31      27      23      19      15      11       7       3      \n");
			printf("   |       |       |       |       |       |       |       |      \n");
			printf("  ");
			for(i=31; i>=0; i--) {
				printf("%2d", (rd>>i)&1);
			}
			printf("\n");
		}
		c = getch();

		switch(c) {

			case 'k':
				// propagate CLK to trgout on all boards
				for(b=0; b<WDcfg.NumBrd; b++) {
					CAEN_DGTZ_ReadRegister(handle[b], CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD, &rdata[b]);
					CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD, 0x00050000);
				}
				printf("Trigger Clk is now output on TRGOUT.\n");
				printf("Press [r] to reload PLL config, any other key to quit clock monitor\n");
				while( (c=getch()) == 'r') {
					//CAEN_DGTZ_WriteRegister(handle[0], 0xEF34, 0);
					ForceClockSync(handle[0]);
					printf("PLL reloaded\n");
				}
				for(b=0; b<WDcfg.NumBrd; b++)
					CAEN_DGTZ_WriteRegister(handle[b], CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD, rdata[b]);
				break;

			case 't':
				ClearScreen();
				printf("Readout Bandwidth Test. Press a key to stop\n");
				buff = (char *)malloc(8*1024*1024);
				ptime = get_time();
				while(!kbhit()) {
					for(b=0; b<WDcfg.NumBrd; b++) {
						CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buff, &nb);
						tnb += nb;
					}
					time=get_time();
					if ((time-ptime) > 1000) {
						printf("Readout Rate = %.3f MB/s\n", ((float)tnb/(1024*1024)) / ((time-ptime)/1000) );
						ptime = time;
						tnb = 0;
					}
				}
				free(buff);
				ClearScreen();
				c = getch();
				break;

			case 'q':
				qmc = 1;
				break;

			case 'i':
				for(b=0; b<WDcfg.NumBrd; b++) {
					printf("Saving Register Images to file reg_image_%d.txt...", b);
					if (SaveRegImage(handle[b]) < 0)
						printf(" Failed!\n\n");
					else
						printf(" Done.\n\n");
				}
				break;

			case 'r':
				printf("Enter Register Address (16 bit hex) : ");
				scanf("%x", &ra);
				CAEN_DGTZ_ReadRegister(handle[b], ra, &rd);
				sprintf(lops, "Read from ");
				lastop = 1;
				break;

			case 'w':
				printf("Enter Register Address (16 bit hex) : ");
				scanf("%x", &ra);
				printf("Enter Register Data (32 bit hex) : ");
				scanf("%x", &rd);
				sprintf(lops, "Write to  ");
				lastop = 2;
				CAEN_DGTZ_WriteRegister(handle[b], ra, rd);
				break;

			case 'c':
				if (CAEN_DGTZ_Calibrate(handle[b]) == 0)
					printf("ADC calibrated\n");
				else
					printf("Calibration failed!\n");
				printf("Press a key\n");
				getch();
				break;

			case 'C':
				printf("Enter Channel Number : ");
				scanf("%x", &ch);
				if (CalibrateWithExternalSignal_x730(handle[b], ch) == 0)
					printf("ADC calibrated\n");
				else
					printf("Calibration failed!\n");
				printf("Press a key\n");
				break;

			case 'R':
				printf("Enter SPI Register Address (8 bit hex) : ");
				scanf("%x", &ra);
				printf("Enter Channel Number : ");
				scanf("%x", &ch);
				ReadSPIRegister(handle[b], ch, ra, &rd);
				printf("SPI reg %02X = %02X\n", ra, rd);
				printf("Press a key\n");
				getch();
				break;

			case 'W':
				printf("Enter SPI Register Address (8 bit hex) : ");
				scanf("%x", &ra);
				printf("Enter Channel Number : ");
				scanf("%x", &ch);
				printf("Value to write : ");
				scanf("%x", &rd);
				WriteSPIRegister(handle[b], ch, ra, rd);
				ReadSPIRegister(handle[b], ch, ra, &rd);
				printf("SPI reg %02X = %02X\n", ra, rd);
				printf("Press a key\n");
				getch();
				break;


			case 'o':
				if (lastop == 1)
					CAEN_DGTZ_ReadRegister(handle[b], ra, &rd);
				else
					CAEN_DGTZ_WriteRegister(handle[b], ra, rd);

			case 'b':
				if (c == 'b') {
					printf("Enter Board number : ");
					scanf("%d", &b);
				}
				break;

			case 'd':
				Manage_DCDC();
				break;

			default:
				break;
		}

	}
	printf("Quitting Manual Controller\n");
	return 0;

}


static int HVstatus_String(uint32_t status, uint32_t ctrl, char *StatString)
{
	if      (status & (1<<10)) sprintf(StatString, "Inhibit");
	else if (status & (1<<9))  sprintf(StatString, "OverTemp");
	else if (status & (1<<7))  sprintf(StatString, "MaxCurrent");
	else if (status & (1<<6))  sprintf(StatString, "MaxVoltage");
	else if (status & (1<<5))  sprintf(StatString, "UnderVoltage");
	else if (status & (1<<4))  sprintf(StatString, "OverVoltage");
	else if (status & (1<<3))  sprintf(StatString, "OverCurrent");
	else if (status & (1<<2))  sprintf(StatString, "RampDown");
	else if (status & (1<<1))  sprintf(StatString, "RampUp");
	else if ((status & 1) && (ctrl & 1))  sprintf(StatString, "ON");
	else                       sprintf(StatString, "OFF");
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: High Voltage controller (for DT5780/DT5790 and Hexagon only)
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int HVsettings()
{
	int quit=0, c=0, ret=0, bb, cc;
	static int ch, b, InitHV=1;
	uint32_t vset_addr, iset_addr, rampup_addr, rampdn_addr, vmax_addr, ctrl_addr, stat_addr, imon_addr, vmon_addr, temp_addr;
	uint32_t d32, status, ctrl, temp;
	// voltages are expressed in V, currents in uA, ramps in V/s, 
	double vset, iset, vmax, rampup, rampdn, vmon, imon;
	double vset_m, iset_m, vmax_m, vmon_m, imon_m, ramp_m;
	double vset_q, iset_q, vmax_q, vmon_q, imon_q;
	long time, ptime;
	char StatString[100];

	int IsHexagon = ((WDcfg.DigitizerModel == 780) || (WDcfg.DigitizerModel == 781) || (WDcfg.DigitizerModel == 790)) ? 0 : 1; 

	vset_addr   = IsHexagon ? 0x10D8    : 0x1220; 
	iset_addr   = IsHexagon ? 0x10DC    : 0x1224; 
	rampup_addr = IsHexagon ? 0x10E0    : 0x1228; 
	rampdn_addr = IsHexagon ? 0x10E4    : 0x122C; 
	vmax_addr   = IsHexagon ? 0x10E8    : 0x1230; 
	ctrl_addr   = IsHexagon ? 0x10EC    : 0x1234; 
	stat_addr   = IsHexagon ? 0x10F0    : 0x1238; 
	vmon_addr   = IsHexagon ? 0x10F4    : 0x1240; 
	imon_addr   = IsHexagon ? 0x10F8    : 0x1244; 
	temp_addr   = 0x10FC; // only for Hexagon

	vset_m      = IsHexagon ? 0.113      : 0.1; // V
	vset_q      = IsHexagon ? -45        : 0; 
	vmon_m      = IsHexagon ? 0.11       : 0.1; // V
	vmon_q      = IsHexagon ? -440       : 0; 
	iset_m      = IsHexagon ? 0.0064     : 0.01;  // uA
	iset_q      = IsHexagon ? -3.1       : 0;
	imon_m      = IsHexagon ? 0.0064     : 0.01;  // uA
	imon_q      = IsHexagon ? -23        : 0; 
	vmax_m      = IsHexagon ? vset_m     : 20;  // V
	vmax_q      = IsHexagon ? vset_q     : 0; 
	ramp_m      = IsHexagon ? 50*vset_m  : 1 ;  // V/s

	if (WDcfg.DigitizerModel == 790) {
		iset_m *= 5;
		imon_m *= 5;
	}

	ptime = get_time();
	time = ptime;

	// Set HV parameters (read from the config file)
	if (InitHV) {
		InitHV = 0;
		for(bb=0; bb < WDcfg.NumBrd; bb++) {
			for(cc=0; cc < WDcfg.NumPhyCh; cc++) {
				if (SysVars.HVmax > 0) 	ret |= CAEN_DGTZ_WriteRegister(handle[bb], vmax_addr   + (cc<<8), (uint32_t)(0.5 + (SysVars.HVmax - vmax_q) / vmax_m));
				if (WDcfg.HV_Vset[bb][cc] > 0) ret |= CAEN_DGTZ_WriteRegister(handle[bb], vset_addr   + (cc<<8), (uint32_t)(0.5 + (WDcfg.HV_Vset[bb][cc] - vset_q) / vset_m));
				if (WDcfg.HV_Iset[bb][cc] > 0) ret |= CAEN_DGTZ_WriteRegister(handle[bb], iset_addr   + (cc<<8), (uint32_t)(0.5 + (WDcfg.HV_Iset[bb][cc] - iset_q) / iset_m));
				if (WDcfg.HV_RampUp[bb][cc] > 0) ret |= CAEN_DGTZ_WriteRegister(handle[bb], rampup_addr + (cc<<8), (uint32_t)(0.5 + WDcfg.HV_RampUp[bb][cc] / ramp_m));
				if (WDcfg.HV_RampDown[bb][cc] > 0) ret |= CAEN_DGTZ_WriteRegister(handle[bb], rampdn_addr + (cc<<8), (uint32_t)(0.5 + WDcfg.HV_RampDown[bb][cc] / ramp_m));
				if (ret) {
					msg_printf(MsgLog, "Error in accessing the High Voltage registers. Press a key...\n");
					getch();
					return -1;
				}
			}
		}
	}

	while (!quit) {
		c = 0;
		time = get_time();
		if ((time - ptime) > 1000) {
			ptime = time;
			ClearScreen();
			CAEN_DGTZ_ReadRegister(handle[b], vset_addr + (ch<<8), &d32);
			vset = d32 * vset_m + vset_q;
			if (vset < 0) vset = 0;
			CAEN_DGTZ_ReadRegister(handle[b], iset_addr + (ch<<8), &d32);
			iset = d32 * iset_m + iset_q;
			if (iset < 0) iset = 0;
			CAEN_DGTZ_ReadRegister(handle[b], vmax_addr + (ch<<8), &d32);
			vmax = d32 * vmax_m + vmax_q;
			if (vmax < 0) vmax = 0;
			CAEN_DGTZ_ReadRegister(handle[b], rampup_addr + (ch<<8), &d32);
			rampup = d32 * ramp_m;
			if (rampup < 0) rampup = 0;
			CAEN_DGTZ_ReadRegister(handle[b], rampdn_addr + (ch<<8), &d32);
			rampdn = d32 * ramp_m;
			if (rampdn < 0) rampdn = 0;
			CAEN_DGTZ_ReadRegister(handle[b], vmon_addr + (ch<<8), &d32);
			vmon = d32 * vmon_m + vmon_q;
			if (vmon < 0) vmon = 0;
			CAEN_DGTZ_ReadRegister(handle[b], imon_addr + (ch<<8), &d32);
			imon = d32 * imon_m + imon_q;
			if (imon < 0) imon = 0;
			CAEN_DGTZ_ReadRegister(handle[b], stat_addr + (ch<<8), &status);
			CAEN_DGTZ_ReadRegister(handle[b], ctrl_addr + (ch<<8), &ctrl);
			CAEN_DGTZ_ReadRegister(handle[b], temp_addr + (ch<<8), &temp);

			if (IsHexagon) status &= ~(1 << 4);  // HACK: the current firmware has a bug on the OverVoltage bit

			HVstatus_String(status, ctrl, StatString);
			printf("\n\nHV Setting Panel\n\n");
			printf("Channel %d - Board %d:\n", ch, b);
			printf("v : Vset        %7.2f V  (Vmon = %7.2f V)\n", vset, vmon);
			if (iset < 1)	printf("i : Iset        %7.2f nA (Imon = %7.2f nA)\n", iset*1000, imon*1000);
			else			printf("i : Iset        %7.2f uA (Imon = %7.2f uA)\n", iset, imon);
			printf("m : Vmax        %4d V\n", (int)vmax);
			printf("u : Ramp Up     %4d V/s\n", (int)rampup);
			printf("d : Ramp Down   %4d V/s\n", (int)rampdn);
			printf("1 : ON\n");
			printf("0 : OFF\n");
			printf("c : Change Channel\n");
			printf("b : Change Board\n");
			printf("q : Quit HV controller\n\n");
			printf("Status      %s\n", StatString);
			if (IsHexagon)	printf("Temperature = %d deg\n", temp & 0xFF);
		}
		if (kbhit())
			c = getch();
		switch (c) {
			case 'v':
				printf("Enter new Vset (V): ");
				scanf("%lf", &vset);
				CAEN_DGTZ_WriteRegister(handle[b], vset_addr + (ch<<8), (uint32_t)((vset-vset_q)/vset_m));
				break;
			case 'i':
				printf("Enter new Iset (uA): ");
				scanf("%lf", &iset);
				CAEN_DGTZ_WriteRegister(handle[b], iset_addr + (ch<<8), (uint32_t)((iset-iset_q)/iset_m));
				break;
			case 'm':
				printf("Enter new Vmax (V): ");
				scanf("%lf", &vmax);
				CAEN_DGTZ_WriteRegister(handle[b], vmax_addr + (ch<<8), (uint32_t)((vmax-vmax_q)/vmax_m));
				break;
			case 'u':
				printf("Enter new RampUp (V/s): ");
				scanf("%lf", &rampup);
				CAEN_DGTZ_WriteRegister(handle[b], rampup_addr + (ch<<8), (uint32_t)(rampup/ramp_m));
				break;
			case 'd':
				printf("Enter new RampDown (V/s): ");
				scanf("%lf", &rampdn);
				CAEN_DGTZ_WriteRegister(handle[b], rampdn_addr + (ch<<8), (uint32_t)(rampdn/ramp_m));
				break;
			case '1':
				CAEN_DGTZ_ReadRegister(handle[b], ctrl_addr + (ch<<8), &ctrl);
				ctrl |= 1;
				CAEN_DGTZ_WriteRegister(handle[b], ctrl_addr + (ch<<8), ctrl);
				break;
			case '0':
				CAEN_DGTZ_ReadRegister(handle[b], ctrl_addr + (ch<<8), &ctrl);
				ctrl &= 0xFFFFFFFE;
				CAEN_DGTZ_WriteRegister(handle[b], ctrl_addr + (ch<<8), ctrl);
				break;
			case 'c':
				printf("Enter new channel: ");
				scanf("%d", &ch);
				break;
			case 'b':
				printf("Enter new board: ");
				scanf("%d", &b);
				break;
			case 'q':
				quit=1;
			default:
				break;
		}
		Sleep(100);
	}
	return 0;

}

