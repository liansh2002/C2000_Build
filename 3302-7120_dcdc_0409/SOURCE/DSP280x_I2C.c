// TI File $Revision: /main/2 $
// Checkin $Date: December 2, 2004   11:55:13 $
//###########################################################################
//
// FILE:	DSP280x_I2C.c
//
// TITLE:	DSP280x SCI Initialization & Support Functions.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File


//---------------------------------------------------------------------------
// InitI2C: 
//---------------------------------------------------------------------------
// This function initializes the I2C to a known state.
//
void InitI2C(void)
{
	InitI2CGpio();

	// Initialize I2C
	I2caRegs.I2CMDR.bit.IRS = 0;
	I2caRegs.I2CSAR = 0x0050;// 
	I2caRegs.I2CPSC.all = 9;		// 
	I2caRegs.I2CCLKL = 65;	// 
	I2caRegs.I2CCLKH = 65;	// 
	I2caRegs.I2CIER.all = 0x00;		// 
	//                                   
	// 
	//	                         
	I2caRegs.I2CMDR.bit.IRS = 1;	// 
   									// 
	I2caRegs.I2CFFTX.all = 0x6000;	// 
	I2caRegs.I2CFFRX.all = 0x2040;	// 

	I2caRegs.I2CCNT=0x0000;
	I2caRegs.I2CSTR.all=0xFFFF;
}	

//---------------------------------------------------------------------------
// Example: InitI2CGpio: 
//---------------------------------------------------------------------------
// This function initializes GPIO pins to function as I2C pins
//
// Each GPIO pin can be configured as a GPIO pin or up to 3 different
// peripheral functional pins. By default all pins come up as GPIO
// inputs after reset.  
// 
// Caution: 
// Only one GPIO pin should be enabled for SDAA operation.
// Only one GPIO pin shoudl be enabled for SCLA operation. 
// Comment out other unwanted lines.

void InitI2CGpio()
{

   EALLOW;
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled disabled by the user.  
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO32 (SDAA)
	GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;	   // Enable pull-up for GPIO33 (SCLA)

/* Set qualification for selected pins to asynch only */
// This will select asynch (no qualification) for the selected pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO32 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO33 (SCLA)

/* Configure SCI pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be I2C functional pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO32 for SDAA operation
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO33 for SCLA operation
	
    EDIS;
	GpioDataRegs.GPASET.bit.GPIO26 = 1;	//WP=1,disable write
}


	
//===========================================================================
// End of file.
//===========================================================================
