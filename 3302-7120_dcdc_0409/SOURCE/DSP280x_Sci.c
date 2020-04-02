// TI File $Revision: /main/2 $
// Checkin $Date: December 2, 2004   11:59:53 $
//###########################################################################
//
// FILE:	DSP280x_Sci.c
//
// TITLE:	DSP280x SCI Initialization & Support Functions.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File

//---------------------------------------------------------------------------
// InitSci: 
//---------------------------------------------------------------------------
// This function initializes the SCI(s) to a known state.
//
void InitSci(void)
{
	// Initialize SCI-A:

	InitSciaGpio();
	
	SciaRegs.SCICCR.all = 0x0027;   // 
	SciaRegs.SCICTL1.all = 0x0003;  // 
	SciaRegs.SCICTL2.all = 0x0000;  // 
	
//	SciaRegs.SCIHBAUD = 0x0001;  	//
//	SciaRegs.SCILBAUD = 0x0044;		//
	SciaRegs.SCIHBAUD = 0x0000;  		// . 
	SciaRegs.SCILBAUD = 0x00A2;		//
	
	SciaRegs.SCIRXST.all = 0x00;   //
	SciaRegs.SCIRXEMU    = 0x00;   //
	SciaRegs.SCIRXBUF.all= 0x00;
	//rsvd1;   
	SciaRegs.SCITXBUF    = 0x00;
	//SciaRegs.SCIFFTX.all =; 
	//SciaRegs.SCIFFRX.all =; 
	//SciaRegs.SCIFFCT.all =; 
	//rsvd2;   
	//rsvd3;   
	SciaRegs.SCIPRI.all  =0x10;  // 

	SciaRegs.SCICTL1.all = 0x0023;  // 

	SciaRegs.SCIFFTX.all=0xE040; //
	SciaRegs.SCIFFRX.all=0x204f; //
	SciaRegs.SCIFFCT.all=0x0;    //

}	

void InitSciaGpio()
{
   EALLOW;

/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled disabled by the user.  
// This will enable the pullups for the specified pins.

	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;	   // Enable pull-up for GPIO29 (SCITXDA)

/* Set qualification for selected pins to asynch only */
// Inputs are synchronized to SYSCLKOUT by default.  
// This will select asynch (no qualification) for the selected pins.

	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)

/* Configure SCI-A pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be SCI functional pins.

	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
	
    EDIS;
}

	
//===========================================================================
// End of file.
//===========================================================================
