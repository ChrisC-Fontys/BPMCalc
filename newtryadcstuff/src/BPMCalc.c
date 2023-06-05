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
XTime peakTime1, peakTime2 = 0;
unsigned long timeDelta = 0;
States triggeredState = WAIT;
int peakThreshold = 1000;
int counter = 0;

u64 sigmaTimeDelta = 0;
u64 avgTimeDelta = 0;

//memory leak prevention
unsigned long* a_timeDelta = NULL;
/****************************************************************************/
/*************************        SOURCE         ****************************/
/****************************************************************************/


//todo: deccompose AVG state into functions that are easier to conceptualise.
//setter functions


void SetPeakThreshold(int p_threshold)
{
	//while loop that compares prev measurement to current emasurement and see if it is going down
	//when it goes down, you multiply by .8 and set this as threashold
	peakThreshold = p_threshold;
	printf("Threshold: %d\n\r",peakThreshold);
}

//calculation funcitons

float GetFrequency(unsigned long p_timeDelta)
{
	float f_timeSeconds = p_timeDelta/(float)COUNTS_PER_SECOND;
	float f_frequency = 1.0/f_timeSeconds; //make sure to account for time base of zynq proc, this is not in seconds but most likely ns)
	printf("frequency: %f\n\r", f_frequency);
	return f_frequency;
}

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
	int data = 0; //magic shit
	data = p_data;
	int avgAmount = p_avgAmount;

	//memory leak check prevention start
	if (a_timeDelta == NULL) {
		//allocate memory nessecary for avgAmount of ints (basically a dynamic array)
		/* int* */ a_timeDelta = (unsigned long*)malloc(avgAmount * sizeof(unsigned long));
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

				//if the amount of times the function has looped is less than avgAMount, loop and increase counter
				if (counter < avgAmount) {
					triggeredState = WAIT;
					a_timeDelta[counter] = timeDelta;
					counter++;
					break;
				}
				//else, calculate average, set counter to 0 and reset state
				else {

					//function: AverageMeasurement();
					for (int i = 0; i < avgAmount; i++) {
						sigmaTimeDelta += a_timeDelta[i];
					} //end for
					triggeredState = WAIT;
					counter = 0;

					free(a_timeDelta);
					printf("memory deallocated");
					a_timeDelta = NULL; //memory leak prevention check over
					avgTimeDelta = sigmaTimeDelta/avgAmount;
					sigmaTimeDelta = 0;
				}//end ifelse

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


unsigned long MeasurementAveraging(unsigned long *p_avgArray, int p_avgAmount)
{
	u64 f_avgTimeDelta = 0;

	if (counter < p_avgAmount) {
		p_avgArray[counter] = timeDelta;
		triggeredState = WAIT;
		counter++;

	}
		//else, calculate average, set counter to 0 and reset state
	else {

		//function: AverageMeasurement();
		for (int i = 0; i < p_avgAmount; i++) {
			sigmaTimeDelta += p_avgArray[i];
		} //end for


		f_avgTimeDelta = sigmaTimeDelta/p_avgAmount;
		sigmaTimeDelta = 0;
		triggeredState = WAIT;
		counter = 0;
	}//end ifelse


	return f_avgTimeDelta;
}
