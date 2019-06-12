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

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdio.h>

#ifdef linux
    #include <sys/time.h> /* struct timeval, select() */
    #include <termios.h>  /* tcgetattr(), tcsetattr() */
    #include <stdlib.h>   /* atexit(), exit() */
    #include <unistd.h>   /* read() */
    #include <string.h>   /* memcpy() */
    #include <ctype.h>    /* toupper() */
    #include <stdint.h>
    #include <sys/stat.h>

#define scanf _scanf  // before calling the scanf function it is necessart to change termios settings

int getch(void);
int kbhit();
int _scanf(char *fmt, ...);


#else  // Windows

    #include <time.h>
    #include <sys/timeb.h>
    #include <conio.h>
    #include <process.h>
	#include <stdint.h>

	#define getch _getch
	#define kbhit _kbhit
	
#endif

//****************************************************************************
// Function prototypes
//****************************************************************************
int InitConsole();
void ClearScreen();
long get_time();
int msg_printf(FILE *MsgLog, char *fmt, ...);
int GetFileUpdateTime(char *fname, uint64_t *ftime);
int GetString(char *str, int MaxCounts);

#endif