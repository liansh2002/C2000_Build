// TI File $Revision: /main/1 $
// Checkin $Date: December 1, 2004   11:11:30 $
//###########################################################################
//
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File

#define ADC_usDELAY  5000L
#define ADC_CKPS   0x0   // 
#define ADC_SHCLK  0x2   //

#define DCVOLT2		0x0
#define DCCURR2		0x1
#define DCVOLT1		0x2
#define DCCURR1		0x3
#define DCTEMP		0x4
#define TEMPCTR		0x5

#define ADDR3		0x0D
#define ADDR4		0x9
#define ADDR2		0x08
#define ADDR1		0x0F
#define ADDR0		0x0E

//---------------------------------------------------------------------------
// InitAdc: 
//---------------------------------------------------------------------------
// This function initializes ADC to a known state.
//
// PLEASE NOTE, ADC INIT IS DIFFERENT ON 281x vs 280x DEVICES!!!
//
void InitAdc(void)
{
	extern void DSP28x_usDelay(Uint32 Count);

	// 
	//
	// 
	// 

	// 
	// 
	// 
	AdcRegs.ADCREFSEL.bit.REF_SEL = 0;			//1: 
												//0:
	
	AdcRegs.ADCTRL3.all = 0x00E0;				// 
	DELAY_US(ADC_usDELAY);						// 

	AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;	//0        
	AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;		
	/*  Acquisition window size.
		        
    	*/

	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;			// 1:  
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;          //  
	//		//

	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = ADDR0;
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = DCVOLT1;
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = DCVOLT2;
	AdcRegs.ADCCHSELSEQ1.bit.CONV03 = ADDR1;

	AdcRegs.ADCCHSELSEQ2.bit.CONV04 = ADDR3;
	AdcRegs.ADCCHSELSEQ2.bit.CONV05 = DCCURR2;
	AdcRegs.ADCCHSELSEQ2.bit.CONV06 = DCCURR2;
	AdcRegs.ADCCHSELSEQ2.bit.CONV07 = ADDR4;

	AdcRegs.ADCCHSELSEQ3.bit.CONV08 = DCCURR2;
	AdcRegs.ADCCHSELSEQ3.bit.CONV09 = DCCURR2;
	AdcRegs.ADCCHSELSEQ3.bit.CONV10 = DCCURR2;
	AdcRegs.ADCCHSELSEQ3.bit.CONV11 = DCVOLT1;

	AdcRegs.ADCCHSELSEQ4.bit.CONV12 = DCVOLT2;
	AdcRegs.ADCCHSELSEQ4.bit.CONV13 = ADDR2;
	AdcRegs.ADCCHSELSEQ4.bit.CONV14 = DCTEMP;
	AdcRegs.ADCCHSELSEQ4.bit.CONV15 = TEMPCTR;
	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0xF;		//  
	
}	

//===========================================================================
// End of file.
//===========================================================================
