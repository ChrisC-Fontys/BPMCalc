/*
 * BPMCalc.c
 *
 *  Created on: 16 mei 2023
 *      Author: Chris
 */

#include <stdlib.h>
#include "stdio.h"
#include "platform.h"
#include "xil_printf.h"
#include "xadcps.h"
#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"
#include "ADCcode.h"
#include "xtime_l.h"
#include "BPMCalc.h"
//#define printf xil_printf /* Small foot-print printf function */


/************************** Variable Definitions ****************************/
static XTime peakTime1, peakTime2 = 0;
static unsigned long timeDelta = 0;
static States triggeredState = WAIT;
static int peakThreshold = 1000;
static int counter = 0;
static int averagedFinished = FALSE;

u64 sigmaTimeDelta = 0;
u64 avgTimeDelta = 0;

//memory leak prevention
unsigned long* a_timeDelta = NULL;
/****************************************************************************/
/*************************        SOURCE         ****************************/
/****************************************************************************/


/*
 * determines threshold value based on peaks of an incoming signal.
 *
 * @param
 * p_percentage: set percentage of peak, in float.
 * p_secondsActive: set how long the function checksfor peaks in whole seconds.
 *
 * @return
 * void
 */
void AutosetPeakThreshold(int p_percentage, int p_secondsActive)
{
	int f_maxPeak = 0;
	XTime f_begin, f_end = 0;
	unsigned long long f_elapsedTime = 0;
	unsigned long long f_runtime = p_secondsActive/COUNTS_PER_SECOND;
	int f_data = 0;

	XTime_GetTime(&f_begin);
	while (f_elapsedTime <= f_runtime)
	{
		f_data = XAdcGetValues();
		if (f_data > f_maxPeak) {
			f_maxPeak = f_data;
		}
		XTime_GetTime(&f_end);
		f_elapsedTime = f_end - f_begin;

	}
	SetPeakThreshold(f_maxPeak*((float)100.0/p_percentage));
}

/*
 * sets peak threshold of peak detection system
 *
 * @param
 * p_threshold: threshold for the peak.
 *
 * @return
 * void
 */
void SetPeakThreshold(int p_threshold)
{
	//while loop that compares prev measurement to current emasurement and see if it is going down
	//when it goes down, you multiply by .8 and set this as threashold
	peakThreshold = p_threshold;
	printf("Threshold: %d\n\r",peakThreshold);
}

/*
 * calculates frequency based on time difference.
 * @param
 * p_timeDelta: integer value for time difference in ticks, not in seconds.
 *
 * @return
 * (float)frequency calculated based on ticks.
 */
float GetFrequency(unsigned long p_timeDelta)
{

	float f_timeSeconds = p_timeDelta/(float)COUNTS_PER_SECOND;
	float f_frequency = 1.0/f_timeSeconds; //make sure to account for time base of zynq proc
	if (f_timeSeconds == 0) f_frequency = 0; //divide by 0 checker
	printf("frequency: %f\n", f_frequency);

	return f_frequency;
}
/*
 * calculates estimated BPM based on time difference.
 *
 * @param
 * p_timeDelta: integer value for time difference in ticks, not in seconds.
 *
 * @return
 * (double)BPM.
 */
int GetBPM(unsigned long p_timeDelta)
{
	int f_BPM = GetFrequency(p_timeDelta) * MINUTE;//truncation is happening but not that big of a deal

	return f_BPM;
}

/*
 * @desc
 * executes peak detection based on input data, and averages the time between peaks by a certain amount that can be set.
 * Threshold for peak detection must be set by other functions, defaults to 1000
 *
 * @param
 * p_data: samples of the ADC
 * p_avgAmount: averaging degree
 *
 * @return
 * time delta averaged over p_avgAmount of type int.
 *
 * @note
 * truncation does not matter since we are only interested in the amount of time and this doesn't have to be very exact.
 * state machine has been written using a custom type states consisting of WAIT, TRIG, WAIT2, DELTA and AVG.
 *
 */
int PeakDetection(int p_data, int p_avgAmount)
{
	int f_data = 0;
	f_data = p_data;

	//allocate memory
	if (a_timeDelta == NULL) {
		a_timeDelta = (unsigned long*)calloc(p_avgAmount, sizeof(unsigned long));
		if (a_timeDelta == NULL) {
			printf("Memory not allocated.\n");
		}
		else {
			printf("Memory successfully allocated\n");
		}
	}

	//state machine
	switch (triggeredState)
	{
	case WAIT:
		if (f_data >= peakThreshold) {
			XTime_GetTime(&peakTime1);
			triggeredState = TRIG;
		}
		break;

	case TRIG:
		if (f_data < peakThreshold) {
			triggeredState = WAIT2;
		}
		break;

	case WAIT2:
		if (f_data >= peakThreshold) {
			XTime_GetTime(&peakTime2);
			triggeredState = DELTA;
		}
		break;
	case DELTA:

		if (f_data < peakThreshold) {
			timeDelta = peakTime2-peakTime1;
			triggeredState = AVG;
		}

		break;

	case AVG:
		MeasurementAveraging(a_timeDelta, p_avgAmount);
		if (averagedFinished == TRUE) {
			free(a_timeDelta);
			printf("memory deallocated");
			a_timeDelta = NULL;
			averagedFinished = FALSE;
		}
		break;

	default:
		triggeredState = WAIT;
	}

	return avgTimeDelta;
}

/*
 * average some measurement array with flexibility for the degreee of averaging.\
 *
 * @param
 * p_avgArray: pointer to dynamic array
 * p_avgAmount: averaging degree
 *
 * @return
 * void
 */
void MeasurementAveraging(unsigned long *p_avgArray, int p_avgAmount)
{
	if (counter < p_avgAmount) {
		p_avgArray[counter] = timeDelta;
		triggeredState = WAIT;
		counter++;
		return;
	}

	else {
		for (int i = 0; i < p_avgAmount; i++) {
			sigmaTimeDelta += p_avgArray[i];
		}
		avgTimeDelta = sigmaTimeDelta/p_avgAmount;
		sigmaTimeDelta = 0;
		triggeredState = WAIT;
		counter = 0;

		averagedFinished = TRUE;


	}


	//return f_avgTimeDelta;
}
