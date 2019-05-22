/******************************************************************************

  CAEN SpA - Viareggio
  www.caen.it

  Description:
  -----------------------------------------------------------------------------
  This is a function that calculates the FFT for a vector of 'ns' samples.
  The number ns must be a power of 2. In case it isn't, the closest power of 2
  smaller than ns will be considered and exceeding samples ignored.
  The calculation of the FFT is based on imaginary numbers (x = real part,
  y = imaginary part). However, the input vector is represented as real numbers
  (unsigned short) and the function returns a vector of real numbers (double)
  that are the amplitude (i.e. sqrt(x^2 + y^2) ) of the FFT points. 
  The amplitude is also normalized respect to the maximum amplitude (for 
  example 4096 for 12 bit samples) and expressed in dB. A contant baseline (for
  example 0.0000001 which is -140dB) is also added to the value in order to 
  low clip the FFT points. Since the FFT has two symmetrical lobes, only half
  points are returned.

  Input Parameters: 
  --------------------------------------------------------------------------
  wave: pointer to the input vector (waveform samples)
  fft: pointer to the output vector (fft amplitude in dB, half lobe)
  ns: number of samples of the input vector wave
  WindowType: Type of windowing for the FFT (see fft.h for more details)

  Return:
  --------------------------------------------------------------------------
  Number of pointf of the output vector fft

******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "digiTES.h"
#include "fft.h"

#ifndef M_PI
#define M_PI 3.141592653
#endif

#define FFT_BASELINE  0.0000001  // Baseline for low clipping (-140dB)
#define NORM_FACTOR   (1 << WDcfg.Nbit) // Normalize the amplitide 

// Global Variables 
int Ncycfft=0;
int Nsfft;
int Nmeanfft=1;
int FFTrdy;  
double *avgfft=NULL, *fftm=NULL, *fft1=NULL;	// buffers for the FFT waveform

// --------------------------------------------------------------------------------------------------------- 
// Description: Calculate the FFT from a waveform
// Inputs:		wave = pointer to the input vector (waveform samples)
//				ns = number of samples of the input vector wave
//				WindowType = Type of windowing for the FFT 
// Output:      fft: pointer to the output vector (fft amplitude in dB, half lobe)
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int FFT (uint16_t *wave, int ns, int WindowType, double *fft)
{
    int m,n,ip,le,le1,nm1,k,l,j,i,nv2;
    double u1,u2,u3,arg,c,s,t1,t2,t3,t4;
    double *x, *y;

    // ns should be a power of 2. If it is not, find the maximum m
    // such that n = 2^m < ns and get n samples instead of ns.
    i = 1;
    while ((1 << i) < ns)
        i++;
    if (ns == (1 << i)) {
        m = i;
        n = ns;
    } else {
        m = i - 1;
        n = (1 << m);
    }

    // allocate the memory buffers for the real and imaginary parts of the fft
    x = (double *)malloc(n * sizeof(double));
    y = (double *)malloc(n * sizeof(double));

    // apply the windowing to the input vector
    for(i=0; i<n; i++) {
        y[i] = 0.0; // imaginary part of the input vector (always 0)
        switch (WindowType) {
            case HANNING_FFT_WINDOW  :  
                x[i] = wave[i] * (0.5 - 0.5 * cos(2*M_PI * i/n));
                break;
            case HAMMING_FFT_WINDOW  :  
                x[i] = wave[i] * (0.54 - 0.46 * cos(2*M_PI * i/n)); 
                break;
            case BLACKMAN_FFT_WINDOW  :  
                x[i] = wave[i] * (2.4 * (0.42323 - 0.49755*cos(2*M_PI*i/n) + 0.07922*cos(4*M_PI*i/n)));
                break;
            case RECT_FFT_WINDOW  :  
                x[i] = wave[i];
                break;
            default :  
                x[i] = wave[i] * (2.4*(0.42323-0.49755*cos(2*M_PI*(i)/n)+0.07922*cos(4*M_PI*(i)/n)));
                break;
        }
    }

    // now the vectors x and y represents the input waveform expressed as imaginary numbers
    // after the appplication of the windowing.

    // calculate the FFT
    for(l=0; l<m; l++) {
        le = 1 << (m-l);
        le1 = le/2;
        u1 = 1.0;
        u2 = 0.0;
        arg = M_PI/le1;
        c = cos(arg);
        s = -sin(arg);

        for (j=0; j<le1; j++) {
            for (i=j; i<n; i=i+le) {
                ip = i+le1;
                t1 = x[i]+x[ip];
                t2 = y[i]+y[ip];
                t3 = x[i]-x[ip];
                t4 = y[i]-y[ip];
                x[ip] = t3*u1-t4*u2;
                y[ip] = t4*u1+t3*u2;
                x[i] = t1;
                y[i] = t2;
            }
        u3 = u1*c-u2*s;
        u2 = u2*c+u1*s;
        u1 = u3;
        }
    }

    nv2 = n/2;
    nm1 = n-1;
    j = 0;
    i = 0;
    while (i < nm1) {
        if(i > j)
            goto rif;
        t1 = x[j];
        t2 = y[j];
        x[j] = x[i];
        y[j] = y[i];
        x[i] = t1;
        y[i] = t2;
        rif:
        k = nv2;
        rife:
        if (k>j)
            goto rifer;
        j = j-k;
        k = k/2;
        goto rife;
        rifer:
        j = j+k;
        i++;
    }

    // get the amplitude of the FFT (modulo)
    y[0]=y[0]/n;
    x[0]=x[0]/n;
    fft[0]=sqrt(x[0]*x[0] + y[0]*y[0]);
    for(i=1;i<n/2;i++) {
        y[i]=2*y[i]/n;
        x[i]=2*x[i]/n;
        fft[i]=sqrt(x[i]*x[i] + y[i]*y[i]);
    }

    // Add the baseline, normalize and transform in dB
    for(i=0; i<n/2; i++) 
        fft[i] = 20 * log10(fft[i]/NORM_FACTOR + FFT_BASELINE);

    // free the buffers and return the number of points (only half FFT)
    free(x);
    free(y);
    return (n/2);
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Initialize variables and allocate memory buffers for the FFT
// Inputs:		-
// Outputs:		AllocatedSize = total num of bytes allocated
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int InitFFT(uint32_t *AllocatedSize)
{
	int fftsize = WDcfg.RecordLength * sizeof(double);
	*AllocatedSize = 0;
	avgfft  = (double *)malloc(fftsize);
	fftm = (double *)malloc(fftsize);
	fft1 = (double *)malloc(fftsize);
	if ((avgfft==NULL) || (fftm==NULL) || (fft1==NULL)) return -1;
	*AllocatedSize = 3 * fftsize;
	Ncycfft = 0;
	FFTrdy = 0;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Free fft buffers
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int CloseFFT()
{
	if (avgfft  != NULL) free(avgfft);
	if (fft1 != NULL) free(fft1);
	if (fftm != NULL) free(fftm);
	avgfft = NULL;
	fft1 = NULL;
	fftm = NULL;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Set the number of waveform to average for the FFT
// Inputs:		nfft: num of waveforms
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int SetFFTaverage(int nfft)
{
	Nmeanfft = nfft;
	Ncycfft = 0;
	return 0;
}

// --------------------------------------------------------------------------------------------------------- 
// Description: Add one FFT waveform to the accumulator (to average N fft waveforms)
// Inputs:		Wave: time domain waveform
// Return:		0=OK, -1=error
// --------------------------------------------------------------------------------------------------------- 
int AccumulateFFT(Waveform_t Wave)
{
	int i;
	if (Nmeanfft == 1) {
		Nsfft = FFT(Wave.AnalogTrace[0], Wave.Ns, HANNING_FFT_WINDOW, avgfft);
		FFTrdy = 1;
	} else {
		Ncycfft++;
		if (Ncycfft == 1) {
			Nsfft = FFT(Wave.AnalogTrace[0], Wave.Ns, HANNING_FFT_WINDOW, fftm);
		} else {
			Nsfft = FFT(Wave.AnalogTrace[0], Wave.Ns, HANNING_FFT_WINDOW, fft1);
			for (i=0; i<Nsfft; i++)	fftm[i] += fft1[i];
		}
		if (Ncycfft == Nmeanfft) {
			for (i=0; i<Nsfft; i++)	avgfft[i] = fftm[i]/Nmeanfft;
			Ncycfft = 0;
			FFTrdy = 1;
		}
	}
	return 0;
}


// --------------------------------------------------------------------------------------------------------- 
// Description: Get averaged FFT (if ready)
// Inputs:		-
// Outputs:		nfft: num of waveforms averaged
//              ns: num of points
//				fft: pointer to the fft buffer
// Return:		0=FFT not ready, 1=FFT ready
// --------------------------------------------------------------------------------------------------------- 
int GetFFT(int *nfft, int *ns, double **fft)
{
	if (!FFTrdy) return 0;
	*nfft = Nmeanfft;
	*ns = Nsfft;
	*fft = avgfft;
	return 1;
}
