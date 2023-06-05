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
#define printf xil_printf /* Small foot-print printf function */


/************************** Variable Definitions ****************************/
static XTime peakTime1, peakTime2 = 0;
static unsigned long timeDelta = 0;
static States triggeredState = WAIT;
static int peakThreshold = 1000;
static int counter = 0;

static u64 sigmaTimeDelta = 0;
static u64 avgTimeDelta = 0;

//memory leak prevention
static unsigned long* a_timeDelta = NULL;
/****************************************************************************/
/*************************        SOURCE         ****************************/
/****************************************************************************/


//todo: deccompose AVG state into functions that are easier to conceptualise.
//setter functions

/*
 * @param
 * data: data samples in integer format (since I am gooing to change the code to not do the volts but raw data instead)
 *
 * @return
 * void
 */
void AutosetPeakThreshold(int p_data, float p_percentage)
{
	int f_maxPeak = 0;
	XTime f_begin, f_end = 0;
	unsigned long long elapsedTime = 0;

	XTime_GetTime(&f_begin);
	while (elapsedTime < COUNTS_PER_SECOND*1.5)
	{

		if (p_data > f_maxPeak) {
			f_maxPeak = p_data;
		}
		XTime_GetTime(&f_end);
		elapsedTime = f_end - f_begin;

	}
	peakThreshold = f_maxPeak * p_percentage;
	printf("new Threshold: %d\n\r",peakThreshold);
}

void SetPeakThreshold(int p_threshold)
{
	peakThreshold = p_threshold;
	printf("Threshold: %d\n\r",peakThreshold);
}

//calculation funcitons

/*
 * @param
 * p_timeDelta: integer value for time difference in ticks, not in seconds.
 *
 * @return
 * (float)frequency calculated based on ticks.
 */
float GetFrequency(unsigned long p_timeDelta)
{
	float f_timeSeconds = p_timeDelta/(float)COUNTS_PER_SECOND;
	float f_frequency = 1.0/f_timeSeconds; //make sure to account for time base of zynq proc, this is not in seconds but most likely ns)
	printf("frequency: %f\n\r", f_frequency);
	return f_frequency;
}
/*
 * @param
 * p_timeDelta: integer value for time difference in ticks, not in seconds.
 *
 * @return
 * (double)BPM.
 */
double GetBPM(unsigned long p_timeDelta)
{
	double f_BPM = GetFrequency(p_timeDelta) * MINUTE;//truncation is happening but not that big of a deal

	return f_BPM;
}

/*
 * @desc
 * executes peak detection based on input data, and averages the time between peaks by a certain amount that can be set.
 * Threshold for peak detection must be set by other functions, defaults to 1000
 *
 * @param
 * p_data: raw samples of the ADC
 * p_avgAmount: averaging degree
 *
 * @return
 * (int)time delta averaged over p_avgAmount.
 *
 * @note
 * truncation does not matter since we are only interested in the amount of time and this doesn't have to be very exact.
 * state machine has been written using a custom type states consisting of WAIT, TRIG, WAIT2, DELTA and AVG.
 *
 */
int PeakDetection(int p_data, int p_avgAmount)
{
	int data = 0; //magic shit
	data = p_data;
	//int avgAmount = p_avgAmount;
	//if I feel like doing it, make sure variables used here can only be seen by this functiom

	//memory leak check prevention start

	//CreateArray(p_avgAmount
	if (a_timeDelta == NULL) {
		//allocate memory nessecary for avgAmount of ints (basically a dynamic array)
		/* int* */ a_timeDelta = (unsigned long*)malloc(p_avgAmount * sizeof(unsigned long));
		//memory allocated?
		if (a_timeDelta == NULL) {
			printf("Memory not allocated.\n");
		}
		else {
			printf("Memory successfully allocated\n");
				//hasRun = 1;
		}//end if
	}//end if

    //state machine for calculating BPM
			switch (triggeredState)
			{
			case WAIT:
				if (data >= peakThreshold) {
					XTime_GetTime(&peakTime1);
					triggeredState = TRIG;

				} //end if
				break;

			case TRIG:
				if (data < peakThreshold) {
					triggeredState = WAIT2;

				} //end if
				break;

			case WAIT2:
				if (data >= peakThreshold) {
					XTime_GetTime(&peakTime2);
					triggeredState = DELTA;

				} //end if
				break;

			case DELTA:

				if (data < peakThreshold) {
					timeDelta = peakTime2-peakTime1;

					triggeredState = AVG;
				}

				break;

			case AVG:

				//save timeDelta into position of array
				//int counter = 0;

				MeasurementAveraging(a_timeDelta, p_avgAmount);
				break;
				//if the amount of times the function has looped is less than avgAMount, loop and increase counter
				/*if (counter < p_avgAmount) {
					triggeredState = WAIT;
					a_timeDelta[counter] = timeDelta;
					counter++;
					break;
				}
				//else, calculate average, set counter to 0 and reset state
				else {

					//function: AverageMeasurement();
					for (int i = 0; i < p_avgAmount; i++) {
						sigmaTimeDelta += a_timeDelta[i];
					} //end for
					triggeredState = WAIT;
					counter = 0;
					*/

					/*free(a_timeDelta);
					printf("memory deallocated");
					a_timeDelta = NULL; //memory leak prevention check over
					avgTimeDelta = sigmaTimeDelta/p_avgAmount;
					sigmaTimeDelta = 0;**/
				//}//end ifelse

			default:
				triggeredState = WAIT;
				//break;

			} //end case

			//debugging prints
			/*printf("State: %d\n\r", triggeredState);
			printf("loop counter: %d\n\r", counter);
			printf("time delta: %llu\n\r", timeDelta);*/
			return avgTimeDelta;
} //end func


void MeasurementAveraging(unsigned long *p_avgArray, int p_avgAmount)
{
	//u64 f_avgTimeDelta = 0;

	if (counter < p_avgAmount) {
		p_avgArray[counter] = timeDelta;
		triggeredState = WAIT;
		counter++;
		return;

	}
		//else, calculate average, set counter to 0 and reset state
	else {

		//function: AverageMeasurement();
		for (int i = 0; i < p_avgAmount; i++) {
			sigmaTimeDelta += p_avgArray[i];
		} //end for


		avgTimeDelta = sigmaTimeDelta/p_avgAmount;
		sigmaTimeDelta = 0;
		triggeredState = WAIT;
		counter = 0;

		free(p_avgArray);
		printf("memory deallocated");
		p_avgArray = NULL; //memory leak prevention check over
		//avgTimeDelta = sigmaTimeDelta/p_avgAmount;
		//sigmaTimeDelta = 0;
	}//end ifelse

}
