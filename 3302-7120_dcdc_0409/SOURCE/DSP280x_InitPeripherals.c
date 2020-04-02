//#########################################################
//
// FILE:	DSP28_InitPeripherals.c
//
// TITLE:	DSP28 Device Initialization To Default State.
//
//#########################################################
// Running on TMS320LF280xA   DCDC part   h741au1(1.00)               
// External clock is 20MHz, PLL * 10/2 , CPU-Clock 100 MHz	      
// Date: from May 23, 2005 to Oct 30, 2006  , (C) www & mhp & mj & lyg
// Version:1.00     Change Date: May 24, 2005 , 
//#########################################################
#define RAM_FUNC_LOAD   0x3EC000    // Source location in Flash
#define RAM_FUNC_LENGTH 0x000080    // Number of 32-bit values to copy
#define RAM_FUNC_RUN    0x008000    // Destination location in RAM

#include "DSP280x_Device.h"

extern unsigned char	ucMdlAddr;		   	/*  */
extern interrupt void dcdc_isr(void);
//extern interrupt void dcdc_load_isr1(void);

//-------------------------------------------------------------------------
// InitPeripherals:
//-------------------------------------------------------------------------
// The following function initializes the peripherals to a default state.
// It calls each of the peripherals default initialization functions.
// This function should be executed at boot time or on a soft reset.
//
void InitPeripherals(void)
{

	// Initialize CPU Timers To default State:
	InitCpuTimersPointer();

	// Initialize Event Manager Peripheral To default State:
	InitEPwmPointer();
	
	// Initialize ADC Peripheral To default State:
	InitAdcPointer();
	
	// Initialize eCAN Peripheral To default State:
	InitECanPointer();

	// Initialize SPI Peripherals To default State:
	//	InitSpi();

	// Initialize SCI Peripherals To default State:
	InitSciPointer();
	
	// Initializes the I2C To default State:
	InitI2CPointer();
	
}

//=========================================================
// No more.
//=========================================================
