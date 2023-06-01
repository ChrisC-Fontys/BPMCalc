#include <stdio.h>
#include "stdio.h"
#include "platform.h"
#include "xil_printf.h"
#include "xadcps.h"
#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"
#include "ADCcode.h"

#define printf xil_printf /* Small foot-print printf function */
static XAdcPs XAdcInst;      /* XADC driver instance */

/************************** Variable Definitions ****************************/
const int adc_channel = 13;

XAdcPs *XAdcInstPtr = &XAdcInst;
XAdcPs_Config *ConfigPtr;

u32 VccPintRawData;
float VccPintData;

/******************************* Functions **********************************/

//configure the XAdc
int XAdcConfig(u16 XAdcDeviceId){
	int Status;


		printf("	Entering the XAdc Polled Example. ");

		/*
		 * Initialize the XAdc driver.
		 */
		ConfigPtr = XAdcPs_LookupConfig(XAdcDeviceId);
		if (ConfigPtr == NULL) {
			return XST_FAILURE;
			printf("XAdcPs_LookupConfig() returns XST_FAILURE");
		}
		XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

		Status = XAdcPs_SelfTest(XAdcInstPtr);
		if (Status != XST_SUCCESS) {
			printf("rXAdcPs_SelfTest() returns XST_FAILUREr");
			return XST_FAILURE;
		}

		XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_CONTINPASS);
		return XST_SUCCESS;
}
//get values from XAdc
float XAdcGetValues()
{
		XAdcPs_Reset(XAdcInstPtr);
		printf("\n\r   ----- New sample -----   \n\r");
		VccPintRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_AUX_MIN + adc_channel);
		//printf_bin(VccPintRawData);
		VccPintData = XAdcPs_RawToVoltage(VccPintRawData)*1.12;
		//return (VccPintData);
		return (VccPintRawData);
}
/*
// set the data from the XAdc into a binary representation
void printf_bin(int number)
{
	if (number > 1) {
		printf_bin(number/2);
	}

	printf("%d", number % 2);
}*/

// convert the value into a decimal representation
int XAdcFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}
	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
