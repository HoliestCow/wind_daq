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

#ifndef _QUEUES_H
#define _QUEUES_H                    // Protect against multiple inclusion

#include "digiTES.h"

#define EV_QUEUE_SIZE				(1024*32)   // max num event in the queue 
#define EV_QUEUE_ALMFULL_LEVEL		(EV_QUEUE_SIZE * 98 / 100)   // almost full level 


//****************************************************************************
// Function prototypes
//****************************************************************************
int CreateQueues(uint32_t *AllocatedMemSize);
int ClearQueues();
int DestroyQueues();
int PushEvent(int b, int ch, GenericDPPEvent_t *evnt);
float GetQueueOccupancy(int b, int ch);
int QueueIsFull(int b, int ch);
int GetBuiltEvents(GenericDPPEvent_t evnt[MAX_NBRD][MAX_NCH], int NumEv[MAX_NBRD][MAX_NCH]);

#endif