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

#ifndef _BOARDUTIL_H
#define _BOARDUTIL_H                    // Protect against multiple inclusion

#include "digiTES.h"

//****************************************************************************
// Function prototypes
//****************************************************************************
int RegisterSetBits(int handle, uint16_t addr, int start_bit, int end_bit, int val);
int WriteSPIRegister(int handle, uint32_t ch, uint32_t address, uint32_t value);
int LockTempCalibration_x730(int handle, int ch);
int CalibrateWithExternalSignal_x730(int handle, int ch);
int LoadCalibrationFromFile_x730(int handle, int ch);
int ReadSPIRegister(int handle, uint32_t ch, uint32_t address, uint32_t *value);
int SaveRegImage(int handle);
int ReadBoardInfo(int b, char *ConnectString);
int ForceClockSync(int handle);
int StartAcquisition();
int StopAcquisition();
int ManualController();
int HVsettings();

#endif