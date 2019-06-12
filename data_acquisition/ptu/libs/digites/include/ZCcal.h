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

#ifndef _ZCCAL_H
#define _ZCCAL_H                    // Protect against multiple inclusion


#include <stdint.h>

#define ZCCFILE_VERSION	0

extern "C"
{
	int ZCcal_CreateZCTable(int b, int ch, int *AllocatedSize);
	int ZCcal_DestroyZCTables(void);
	int ZCcal_ResetZCTables(int Nsamples);
	int ZCcal_GetFillingProgress(int b, int ch);
	int ZCcal_AddSample(int b, int ch, uint16_t zcval, uint16_t Energy);
	uint16_t ZCcal_ApplyCorrection(int b, int ch, uint16_t zcval, uint16_t Energy);
	int ZCcal_SaveCorrectionTables(char *ZCcal_FileName);
	int ZCcal_LoadCorrectionTables(char *ZCcal_FileName);
}

#endif
