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

#include "Console.h"
#include <stdio.h> 
#include <stdarg.h>

#ifdef linux

#define CLEARSCR "clear"


/*****************************************************************************/
static struct termios g_old_kbd_mode;

static void cooked(void)
{
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

static void raw(void)
{
	static char init=0;
	struct termios new_kbd_mode;

	//if (init) return;
	/* put keyboard (stdin, actually) in raw, unbuffered mode */
	tcgetattr(0, &g_old_kbd_mode);
	memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
	new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_kbd_mode);
	/* when we exit, go back to normal, "cooked" mode */
	atexit(cooked);
	init = 1;
}

// --------------------------------------------------------------------------------------------------------- 
//  Init console window (terminal)
// --------------------------------------------------------------------------------------------------------- 
int InitConsole() 
{
	raw();
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
//  GETCH  
// --------------------------------------------------------------------------------------------------------- 
int getch(void)
{
	unsigned char temp;

    /* stdin = fd 0 */
	if(read(0, &temp, 1) != 1)
		return 0;
	return temp;
}


// --------------------------------------------------------------------------------------------------------- 
//  KBHIT  
// --------------------------------------------------------------------------------------------------------- 
int kbhit()
{
	struct timeval timeout;
	fd_set read_handles;
	int status;

    /* check stdin (fd 0) for activity */
	FD_ZERO(&read_handles);
	FD_SET(0, &read_handles);
	timeout.tv_sec = timeout.tv_usec = 0;
	status = select(0 + 1, &read_handles, NULL, NULL, &timeout);
	if(status < 0) {
		printf("select() failed in kbhit()\n");
		exit(1);
	}
    return (status);
}

// --------------------------------------------------------------------------------------------------------- 
//  SCANF (change termios settings, then execute scanf) 
// --------------------------------------------------------------------------------------------------------- 
int _scanf(char *fmt, ...) 
{
	int ret;
	cooked();
	va_list args;
	va_start(args, fmt);
	ret = vscanf(fmt, args);
	va_end(args);
	raw();
	return ret;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: get time from the computer
// Return:		time in ms
// --------------------------------------------------------------------------------------------------------- 
long get_time()
{
    long time_ms;
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;
    return time_ms;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: get write time of a file 
// Return:		0: OK, -1: error
// --------------------------------------------------------------------------------------------------------- 
int GetFileUpdateTime(char *fname, uint64_t *ftime)
{
	struct stat t_stat;

	*ftime = 0;
    stat(fname, &t_stat);
    //struct tm * timeinfo = localtime(&t_stat.st_mtime); 
    *ftime = (uint64_t)t_stat.st_mtime; // last modification time
    return 0;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: clear the console
// --------------------------------------------------------------------------------------------------------- 
void ClearScreen()
{
	// HACK : how to clear screen in Linux?
}

// --------------------------------------------------------------------------------------------------------- 
// Description: get a string from stdin (space are allowed)
// Input:		str: pointer to the string
//				MaxCount: max num of characters
// Return:		size of the string
// --------------------------------------------------------------------------------------------------------- 
int GetString(char *str, int MaxCounts)
{
	fflush(stdin);
	cooked();
	fgets(str, MaxCounts, stdin);
	raw();
	return(strlen(str));
}



#else  // Windows

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>


// --------------------------------------------------------------------------------------------------------- 
//  Init console window (terminal)
// --------------------------------------------------------------------------------------------------------- 
int InitConsole() 
{
	// Set console window size
	system("mode con: cols=80 lines=60");
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: clear the console
// --------------------------------------------------------------------------------------------------------- 
void ClearScreen()
{
	system("cls");  
}

// --------------------------------------------------------------------------------------------------------- 
// Description: get time from the computer
// Return:		time in ms
// --------------------------------------------------------------------------------------------------------- 
long get_time()
{
	long time_ms;
	struct _timeb timebuffer;
	_ftime( &timebuffer );
	time_ms = (long)timebuffer.time * 1000 + (long)timebuffer.millitm;
	return time_ms;
}



// --------------------------------------------------------------------------------------------------------- 
// Description: get write time of a file 
// Return:		0: OK, -1: error
// --------------------------------------------------------------------------------------------------------- 
int GetFileUpdateTime(char *fname, uint64_t *ftime)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    HANDLE hFile;

	wchar_t wtext[200];
	mbstowcs(wtext, fname, strlen(fname)+1);//Plus null
	LPWSTR ptr = wtext;

	*ftime = 0;
    hFile = CreateFile(ptr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE) 
        return -1;

    // Retrieve the file times for the file.
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
        return -1;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&ftWrite, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
	CloseHandle(hFile);    

	//printf("Y=%d M=%d D=%d - H=%d, min=%d, sec=%d\n", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	*ftime = 366*31*24*3600*(uint64_t)stLocal.wYear   + 
	 	         31*24*3600*(uint64_t)stLocal.wMonth  + 
	 			    24*3600*(uint64_t)stLocal.wDay    + 
	 			       3600*(uint64_t)stLocal.wHour   + 
	 				     60*(uint64_t)stLocal.wMinute + 
						    (uint64_t)stLocal.wSecond;

    return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: get a string from stdin (space are allowed)
// Input:		str: pointer to the string
//				MaxCount: max num of characters
// Return:		size of the string
// --------------------------------------------------------------------------------------------------------- 
int GetString(char *str, int MaxCounts)
{
	fflush(stdin);
	fgets(str, MaxCounts, stdin);
	return((int)strlen(str));
}

#endif



// --------------------------------------------------------------------------------------------------------- 
// Description: printf to screen and to log file
// Return:		0: OK
// --------------------------------------------------------------------------------------------------------- 
int msg_printf(FILE *MsgLog, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(MsgLog, fmt, args);
	fflush(MsgLog);
	va_end(args);
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	return 0;
}

