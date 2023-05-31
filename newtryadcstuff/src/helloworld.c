/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
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


/************************** Constant Definitions ****************************/

#define XADC_DEVICE_ID 			XPAR_XADCPS_0_DEVICE_ID
//#define XTIME_GLOBAL_REGISTER
const int avgAmount = 1;
/************************** Function-specific ****************************/
/**************************** Enum Definitions ******************************/

/*typedef enum States
{
	WAIT,
	TRIG,
	WAIT2,
	DELTA,
	AVG
}States;*/

/************************** Variable Definitions ****************************/
/*XTime peakTime1, peakTime2;
u64 timeDelta = 0;
//short triggeredState = 0;
States triggeredState = WAIT;
int peakThreshold = 1000;
int counter = 0;

int sigmaTimeDelta = 0;
int avgTimeDelta = 0;

//memory leak prevention
int* a_timeDelta = NULL; */


/***************** Macros (Inline Functions) Definitions ********************/

#define printf xil_printf /* Small foot-print printf function */

/************************** Function Prototypes *****************************/
/****************************************************************************/
/**************************        MAIN         *****************************/
/****************************************************************************/


//todo: change hardware arch to send interrupts to pynq, in order to implement an interrupt handler and locs.
//todo: add LED that turns on when a peak is detected.
//todo:

int main()
{
    init_platform();
    printf("Starting program...\n\r");

    int Status;

    Status = XAdcConfig(XADC_DEVICE_ID);
    if (Status != XST_SUCCESS) {
    	printf("configuring XADC unsuccessful\n\r");
    	return XST_FAILURE;
    }

    SetPeakThreshold(3);
    int averagedTimeDifference = 0;

	while(1)
	{
		//note: by sleeping the program for a specifiedd amount of time, we can vary the sample rate. possible disadvantage is incosistent sampling times.
		//therefore, we might want to use interrupts in order to make sure the sample rate stays consistent. (interrupts are used lol)
		float data =  XAdcGetValues();
		printf("%0d.%03d Volts.\n\r", (int)(data), XAdcFractionToInt(data));

		averagedTimeDifference = PeakDetection(data, avgAmount);

		// apply filters
		// add piek detection
	} //end while

    printf("Program finished \n\r");
    cleanup_platform();
    return XST_SUCCESS;
} //end main


//todo: create interrupt service routie that saves the data in variable every time adc triggers an interrupt.
