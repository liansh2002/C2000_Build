//#########################################################
//
// FILE:	DSP280x_ECan.c
//
// TITLE:	DSP280x Enhanced CAN Initialization & Support Functions.
//
//#########################################################
// Running on TMS320LF280xA   DCDC part   h741au1(1.00)               
// External clock is 20MHz, PLL * 10/2 , CPU-Clock 100 MHz	      
// Date: from May 23, 2005 to Oct 30, 2006  , (C) www & mhp & mj & lyg
// Version:1.00     Change Date: May 24, 2005 , 
//#########################################################

#include "DSP280x_Device.h"     // DSP28 Headerfile Include File

//---------------------------------------------------------------------------
// InitECan: 
//---------------------------------------------------------------------------
// This function initializes the eCAN module to a known state.
//
void InitECan(void)
{
	InitECanaGpio();

 	InitECana();
}

void InitECana(void)		// Initialize eCAN-A module
{
/* Create a shadow register structure for the CAN control registers. This is
 needed, since, only 32-bit access is allowed to these registers. 16-bit access
 to these registers could potentially corrupt the register contents. This is
 especially true while writing to a bit (or group of bits) among bits 16 - 31 */

	struct ECAN_REGS ECanaShadow;

	//uiMdlAddr = 0;

	EALLOW;		// EALLOW enables access to protected bits

	ECanaRegs.CANME.all = 0x00000000;
	
	/* Configure eCAN RX and TX pins for eCAN transmissions using eCAN regs*/  
    
	ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
	ECanaShadow.CANTIOC.bit.TXFUNC = 1;
	ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

	ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
	ECanaShadow.CANRIOC.bit.RXFUNC = 1;
	ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
									// HECC mode also enables time-stamping feature
	
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.SUSP = 1;	
	ECanaShadow.CANMC.bit.SCB = 1;	
	ECanaShadow.CANMC.bit.ABO = 1;	
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	
	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero
 
	/*ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
	ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;*/    
	
/*	for(i=0x6100; i<0x6200; i++)
	{
		addr = (unsigned long *)i;
		*addr=0;
	}*/
	ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000008;
	ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000008;
    
	/*ECanaMboxes.MBOX0.MSGID.bit.IDE = 1;
	ECanaMboxes.MBOX0.MSGID.bit.AME = 1;
	ECanaMboxes.MBOX0.MSGID.bit.AAM = 0;
	ECanaMboxes.MBOX0.MSGID.bit.PROTNO = 0x070;
	ECanaMboxes.MBOX0.MSGID.bit.PTP = 0;
	ECanaMboxes.MBOX0.MSGID.bit.DSTADDRH = 0;
	ECanaMboxes.MBOX0.MSGID.bit.DSTADDRL = 0;
	ECanaMboxes.MBOX0.MSGID.bit.SRCADDR = 0x00;
	ECanaMboxes.MBOX0.MSGID.bit.CNT = 0;
	ECanaMboxes.MBOX0.MSGID.bit.RES1 = 1;
	ECanaMboxes.MBOX0.MSGID.bit.RES2 = 1;*/
	ECanaMboxes.MBOX0.MSGID.all = 0xc7080003;

	/*ECanaMboxes.MBOX1.MSGID.bit.IDE = 1;
	ECanaMboxes.MBOX1.MSGID.bit.AME = 1;
	ECanaMboxes.MBOX1.MSGID.bit.AAM = 0;
	ECanaMboxes.MBOX1.MSGID.bit.PROTNO = 0x060;
	ECanaMboxes.MBOX1.MSGID.bit.PTP = 1;
	ECanaMboxes.MBOX1.MSGID.bit.DSTADDRH = 0;
	ECanaMboxes.MBOX1.MSGID.bit.DSTADDRL = 0;
	ECanaMboxes.MBOX1.MSGID.bit.SRCADDR = 0x0F0; //
	ECanaMboxes.MBOX1.MSGID.bit.CNT = 0;
	ECanaMboxes.MBOX1.MSGID.bit.RES1 = 1;
	ECanaMboxes.MBOX1.MSGID.bit.RES2 = 1;*/
	ECanaMboxes.MBOX1.MSGID.all = 0xc6080783;

	/*ECanaMboxes.MBOX4.MSGID.bit.IDE = 1;
	ECanaMboxes.MBOX4.MSGID.bit.AME = 1;
	ECanaMboxes.MBOX4.MSGID.bit.AAM = 0;
	ECanaMboxes.MBOX4.MSGID.bit.PROTNO = 0x060;
	ECanaMboxes.MBOX4.MSGID.bit.PTP = 1;
	ECanaMboxes.MBOX4.MSGID.bit.DSTADDRH = 0x7;
	ECanaMboxes.MBOX4.MSGID.bit.DSTADDRL = 0x10;
	ECanaMboxes.MBOX4.MSGID.bit.SRCADDR = uiMdlAddr; 
	ECanaMboxes.MBOX4.MSGID.bit.CNT = 0;
	ECanaMboxes.MBOX4.MSGID.bit.RES1 = 1;
	ECanaMboxes.MBOX4.MSGID.bit.RES2 = 1;*/
	ECanaMboxes.MBOX4.MSGID.all = 0xc60F8003;
	ECanaMboxes.MBOX4.MSGID.bit.SRCADDR = uiMdlAddr; 

	/*ECanaMboxes.MBOX5.MSGID.bit.IDE = 1;
	ECanaMboxes.MBOX5.MSGID.bit.AME = 1;
	ECanaMboxes.MBOX5.MSGID.bit.AAM = 0;
	ECanaMboxes.MBOX5.MSGID.bit.PROTNO = 0x070;
	ECanaMboxes.MBOX5.MSGID.bit.PTP = 0;
	ECanaMboxes.MBOX5.MSGID.bit.DSTADDRH = 0x7;
	ECanaMboxes.MBOX5.MSGID.bit.DSTADDRL = 0x1F; //
	ECanaMboxes.MBOX5.MSGID.bit.SRCADDR = uiMdlAddr; 
	ECanaMboxes.MBOX5.MSGID.bit.CNT = 0;
	ECanaMboxes.MBOX5.MSGID.bit.RES1 = 1;
	ECanaMboxes.MBOX5.MSGID.bit.RES2 = 1;*/
	ECanaMboxes.MBOX5.MSGID.all = 0xc707F803;
	ECanaMboxes.MBOX5.MSGID.bit.SRCADDR = uiMdlAddr; 

    
// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
//	as a matter of precaution. 

	ECanaRegs.CANTA.all	= 0xFFFFFFFF;	/* Clear all TAn bits */      
	
	ECanaRegs.CANRMP.all = 0xFFFFFFFF;	/* Clear all RMPn bits */      
	
	ECanaRegs.CANGIF0.all = 0xFFFFFFFF;	/* Clear all interrupt flag bits */ 
	ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

	
/* Configure bit timing parameters for eCANA*/
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.CCR = 1 ;            // Set CCR = 1
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	ECanaShadow.CANES.all = ECanaRegs.CANES.all;

	while(ECanaShadow.CANES.bit.CCE != 1 ) 			// Wait for CCE bit to be set..
	{ECanaShadow.CANES.all = ECanaRegs.CANES.all;}   
    
	ECanaShadow.CANBTC.all = 0;
 	ECanaShadow.CANBTC.bit.BRPREG = 49;//79;//
 	ECanaShadow.CANBTC.bit.TSEG2REG = 3;//1;//
 	ECanaShadow.CANBTC.bit.TSEG1REG = 10;//6;// 
 	ECanaShadow.CANBTC.bit.SAM = 0;//1;//three times
 	ECanaShadow.CANBTC.bit.SJWREG = 2;//0;
 	ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;
    
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.CCR = 0 ;            // Set CCR = 0
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    
	while(ECanaShadow.CANES.bit.CCE != 0 ) 			// Wait for CCE bit to be  cleared..
	{ECanaShadow.CANES.all = ECanaRegs.CANES.all;}   

	/* Enable Mailboxe 0.1.4.5  */
 	ECanaRegs.CANME.all = 0x00000033;		// Required before writing the MSGIDs
	ECanaRegs.CANMD.all = 0x00000003; 
	//ECanaRegs.CANMD.all = 0xFFFFFFCF; 

	EDIS;

	/* even curr, receive data from other module  */
	//C300000B
	ECanaLAMRegs.LAM0.all = 0x800FFFFC;
	ECanaLAMRegs.LAM1.all = 0x800FFFFC;
	ECanaLAMRegs.LAM4.all = 0x800FFFFC;
	ECanaLAMRegs.LAM5.all = 0x800FFFFC;

}	




//---------------------------------------------------------------------------
// Example: InitECanGpio: 
//---------------------------------------------------------------------------
// This function initializes GPIO pins to function as eCAN pins
//
// Each GPIO pin can be configured as a GPIO pin or up to 3 different
// peripheral functional pins. By default all pins come up as GPIO
// inputs after reset.  
// 
// Caution: 
// Only one GPIO pin should be enabled for CANTXA/B operation.
// Only one GPIO pin shoudl be enabled for CANRXA/B operation. 
// Comment out other unwanted lines.


void InitECanGpio(void)
{
   InitECanaGpio();
}

void InitECanaGpio(void)
{
   EALLOW;

/* Enable internal pull-up for the selected CAN pins */
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

	GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	    // Enable pull-up for GPIO30 (CANRXA)
	GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	    // Enable pull-up for GPIO31 (CANTXA)

/* Set qualification for selected CAN pins to asynch only */
// Inputs are synchronized to SYSCLKOUT by default.  
// This will select asynch (no qualification) for the selected pins.

    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)   

/* Configure eCAN-A pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be eCAN functional pins.

	GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;	// Configure GPIO30 for CANTXA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;	// Configure GPIO31 for CANRXA operation

    EDIS;
}

	
/***************************************************/
/* Bit configuration parameters for 100 MHz SYSCLKOUT*/ 
/***************************************************/
/*
The table below shows how BRP field must be changed to achieve different bit
rates with a BT of 10, for a 80% SP:
---------------------------------------------------
BT = 10, TSEG1 = 6, TSEG2 = 1, Sampling Point = 80% 
---------------------------------------------------
1   Mbps : BRP+1 = 10 	: CAN clock = 10 MHz
500 kbps : BRP+1 = 20 	: CAN clock = 5 MHz 
250 kbps : BRP+1 = 40 	: CAN clock = 2.5 MHz 
125 kbps : BRP+1 = 80 	: CAN clock = 1.25 MHz 
100 kbps : BRP+1 = 100 	: CAN clock = 1 MHz
50  kbps : BRP+1 = 200 	: CAN clock = 0.5 MHz

The table below shows how to achieve different sampling points with a BT of 25:
-------------------------------------------------------------
Achieving desired SP by changing TSEG1 & TSEG2 with BT = 25  
-------------------------------------------------------------

TSEG1 = 18, TSEG2 = 4, SP = 80% 
TSEG1 = 17, TSEG2 = 5, SP = 76% 
TSEG1 = 16, TSEG2 = 6, SP = 72% 
TSEG1 = 15, TSEG2 = 7, SP = 68% 
TSEG1 = 14, TSEG2 = 8, SP = 64% 

The table below shows how BRP field must be changed to achieve different bit
rates with a BT of 25, for the sampling points shown above: 

1   Mbps : BRP+1 = 4 
500 kbps : BRP+1 = 8 
250 kbps : BRP+1 = 16 
125 kbps : BRP+1 = 32 
100 kbps : BRP+1 = 40
50  kbps : BRP+1 = 80

*/

//===========================================================================
// End of file.
//===========================================================================

