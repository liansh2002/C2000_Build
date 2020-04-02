// TI File $Revision: /main/2 $
// Checkin $Date: December 2, 2004   11:50:58 $
//###########################################################################
//
// FILE:	DSP280x_Gpio.c
//
// TITLE:	DSP280x General Purpose I/O Initialization & Support Functions.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File


//---------------------------------------------------------------------------
// InitGpio: 
//---------------------------------------------------------------------------
// This function initializes the Gpio to a known (default) state.
//
// For more details on configuring GPIO's as peripheral functions,
// refer to the individual peripheral examples and/or GPIO setup example. 
void InitGpio(void)
{
   EALLOW;
   
   // Each GPIO pin can be: 
   // a) a GPIO input/output
   // b) peripheral function 1
   // c) peripheral function 2
   // d) peripheral function 3
   // By default, all are GPIO Inputs 
   GpioCtrlRegs.GPAMUX1.all = 0x0000;     // GPIO functionality GPIO0-GPIO15
   GpioCtrlRegs.GPAMUX2.all = 0x0000;     // GPIO functionality GPIO16-GPIO31
   GpioCtrlRegs.GPBMUX1.all = 0x0000;     // GPIO functionality GPIO32-GPIO34

	//GpioCtrlRegs.GPADIR.all = 0x7FE0FF0;   // GPIO0-GPIO3,GPIO12-GPIO16,GPIO27 are inputs, others:outputs
    GpioCtrlRegs.GPADIR.all = 0x7FF0FF0;   // GPIO0-GPIO3,GPIO12-GPIO15,GPIO27 are inputs, others:outputs
	GpioCtrlRegs.GPBDIR.all = 0x0007;      // GPIO32-GPIO34 are outputs   
   
   // Each input can have different qualification
   // a) input synchronized to SYSCLKOUT
   // b) input qualified by a sampling window
   // c) input sent asynchronously (valid for peripheral inputs only)
	GpioCtrlRegs.GPAQSEL1.all = 0x55000000;    // GPIO12-GPIO15 Qualification using 3 samples. 
	GpioCtrlRegs.GPAQSEL2.all = 0x00000000;    // GPIO16-GPIO31 Synch to SYSCLKOUT
	GpioCtrlRegs.GPBQSEL1.all = 0x0000;    // GPIO32-GPIO34 Synch to SYSCLKOUT 

   // Pull-ups can be enabled or disabled. 
	GpioCtrlRegs.GPAPUD.all = 0x0BFBE3EF;      // 1:Pullup's disabled 0:enable
	GpioCtrlRegs.GPBPUD.all = 0x0000;      // Pullup's enabled GPIO32-GPIO34

	/*GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 0x02;	//QUALPRD = SYSCLKOUT/4
	GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0x02;	//QUALPRD = SYSCLKOUT/4
	GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0x02;	//QUALPRD = SYSCLKOUT/4
	GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0x02;	//QUALPRD = SYSCLKOUT/4*/
	GpioCtrlRegs.GPACTRL.all = 0x02020202;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 0x02;	//QUALPRD = SYSCLKOUT/4

	GpioDataRegs.GPADAT.all = 0xF4040000;

   EDIS;
    GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;  
}	
	
//===========================================================================
// End of file.
//===========================================================================
