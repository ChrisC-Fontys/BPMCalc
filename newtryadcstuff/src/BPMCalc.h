/*
 * BPMCalc.h
 *
 *  Created on: 16 mei 2023
 *      Author: Chris
 */

#ifndef SRC_BPMCALC_H_
#define SRC_BPMCALC_H_



#endif /* SRC_BPMCALC_H_ */

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
/***************************** TYPEFDEFS ***********************************/
typedef enum States
{
	WAIT,
	TRIG,
	WAIT2,
	DELTA,
	AVG
}States;

/***************************** CONSTANTS ***********************************/
#define MINUTE 60
/************************** Function Prototypes ****************************/

void SetPeakThreshold(int p_threshold);
void AutosetPeakThreshold(float p_percentage, int p_secondsActive);
int PeakDetection(int p_data, int p_avgAmount);
unsigned long MeasurementAveraging();

float GetFrequency(unsigned long p_timeDelta);
int GetBPM(unsigned long p_timeDelta);
