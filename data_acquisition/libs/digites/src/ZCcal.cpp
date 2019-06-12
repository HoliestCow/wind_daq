//#include "stdafx.h"
#include <stdexcept>
#include <stdio.h>

#include "ZCcal.h"

extern "C"
{
	int ZCcal_CreateZCTable(int b, int ch, int *AllocatedSize) {
		return -1;
	}

	int ZCcal_DestroyZCTables(void) {
		return 0;
	}

	int ZCcal_ResetZCTables(int Nsamples) {
		return 0;
	}

	int ZCcal_GetFillingProgress(int b, int ch) {
		 return 0;
	}

	int ZCcal_AddSample(int b, int ch, uint16_t zcval, uint16_t Energy) {
		return 0;
	}

	uint16_t ZCcal_ApplyCorrection(int b, int ch, uint16_t zcval, uint16_t Energy) {
		return zcval;
	}

	int ZCcal_SaveCorrectionTables(char *ZCcal_FileName) {
		return 0;
	}

	int ZCcal_LoadCorrectionTables(char *ZCcal_FileName) {
		return 0;
	}

}
