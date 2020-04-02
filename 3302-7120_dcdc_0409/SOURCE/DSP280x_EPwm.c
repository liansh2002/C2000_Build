// TI File $Revision: /main/3 $
// Checkin $Date: December 2, 2004   14:54:39 $
//###########################################################################
//
// FILE:   DSP280x_EPwm.c
//
// TITLE:  DSP280x ePWM Initialization & Support Functions.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File

// Configure which ePWM timer interrupts are enabled at the PIE level:
// 1 = enabled,  0 = disabled
#define EPWM1_INT_ENABLE  1
//---------------------------------------------------------------------------
// InitEPwm: 
//---------------------------------------------------------------------------
// This function initializes the ePWM(s) to a known state.
//
void InitEPwm(void)
{
	// Initialize ePWM1/2
	// for HV
	InitEPwm1Gpio();
	InitEPwm2Gpio();
    InitEPwm3Gpio();
    InitEPwm4Gpio();
	
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
	EDIS;

	// force all PWM to low
	EPwm1Regs.AQSFRC.bit.RLDCSF = 0;
	EPwm2Regs.AQSFRC.bit.RLDCSF = 0;
	EPwm3Regs.AQSFRC.bit.RLDCSF = 0;
    EPwm4Regs.AQSFRC.bit.RLDCSF = 0;

    EPwm1Regs.AQCSFRC.all = 0x05;		//PWM1A,PWM1B force low
	EPwm2Regs.AQCSFRC.all = 0x05;		//PWM2A,PWM2B force lowÿ
	EPwm3Regs.AQCSFRC.all = 0x05;		//PWM3A,PWM3B force lowÿ
    EPwm4Regs.AQCSFRC.all = 0x05;		//PWM4A,PWM4B force low 
		// Config Epwm1 for HV
		EPwm1Regs.TBPRD = 166;
		EPwm1Regs.TBPHS.half.TBPHS = 0;
		EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// Count up/dn
		EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
		EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;			//master module
		EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;		// Pass through 
		//TBCLK=SYSCLKOUT/(HSPCLKDIV * CLKDIV)=SYSCLKOUT
		EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;		    //Timebase clock pre-scale
		EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;	    //High speed time pre-scale
		EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;	    // Select INT on Zero event
		EPwm1Regs.ETSEL.bit.INTEN = 0;		            // Enable INT
		EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;				// Generate INT on 1st event
		//EPwm1Regs.ETSEL.bit.SOCAEN = 1;					//Enables SOCA pulse
		//EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;		//Enable CNTR = zero event
		//EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;				// Generate pulse on 1st event 

		// for PWM  wave
		EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
		EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
		EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
		EPwm1Regs.AQCTLA.bit.CBU = AQ_SET;			// Set PWM1A on event : ctr = zero
		EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;			// Clear PWM1A on ctr = cmpa(up)		
		EPwm1Regs.AQCTLB.bit.CBU = AQ_SET;			
		EPwm1Regs.AQCTLB.bit.CBD = AQ_CLEAR;			
	
		EPwm1Regs.DBCTL.bit.OUT_MODE = 0x00;
		EPwm1Regs.DBCTL.bit.POLSEL = 0x00;
        EPwm1Regs.DBFED=0;
        EPwm1Regs.TBCTR = 0;	
		EPwm1Regs.CMPA.half.CMPA = 165;
        EPwm1Regs.CMPB = 161;
		
	
//		EALLOW;
//		EPwm1Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO;
//		EPwm1Regs.HRCNFG.bit.EDGMODE = HR_FEP;
//		EPwm1Regs.HRCNFG.bit.CTLMODE = HR_CMP;
		//(frac(PWMDuty*PWMperiod)*MEP_SF) << 8) + 0180h
		//MEP_SF = (10ns/180ps) = 55
		//eg. (frac(0.2016*500)*55<<8)+180h 
		//CMPA = 100, CMPAHR=0.8*55<<8+180h =44<<8+180h = 2D80h
//		EPwm1Regs.CMPA.half.CMPAHR = 0x0180;
//		EDIS;
	
		// Config Epwm2 for HV
		EPwm2Regs.TBPRD = 166;
		EPwm2Regs.TBPHS.half.TBPHS =0;
		EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// Count up/dn
		EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
		EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
		EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;		// Sync flow through 
		EPwm2Regs.TBCTL.bit.PHSDIR = TB_UP;				//Phase Direction
		//TBCLK=SYSCLKOUT/(HSPCLKDIV * CLKDIV)=SYSCLKOUT
		EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;				//Timebase clock pre-scale
		EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;			//High speed time pre-scale
        //EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;			// Select INT on PRD event
	//	EPwm2Regs.ETSEL.bit.INTEN = 0;		// Enable INT
	//	EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;				// Generate INT on 1st event
		// for PWM  wave
		//EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_IMMEDIATE;
		EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
		EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
		EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
		EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;			// Set PWM2A on event , ctr = prd
		EPwm2Regs.AQCTLA.bit.CBD = AQ_SET;		// Clear PWM2A on ctr = cmpa(dn)
		EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;		
		EPwm2Regs.AQCTLB.bit.CBD = AQ_SET;		
			
		EPwm2Regs.DBCTL.bit.OUT_MODE = 0x00;
		EPwm2Regs.DBCTL.bit.POLSEL = 0x00;           //
		EPwm2Regs.DBFED = 0;       //only for 2812    BJG
		EPwm2Regs.CMPA.half.CMPA = 12;
        EPwm2Regs.CMPB = 88;


        //2018-10-26 ¿ªÖÐ¶Ï£¬ÓÃÓÚ¼äÐªÄ£Ê½¿ØÖÆ
        EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;            //CTR==PRDÊ±£¬¼ÇÒ»´ÎÊÂ¼þ
        EPwm2Regs.ETSEL.bit.INTEN = 0;                      //ÔÝÊ±½ûÖ¹ÖÐ¶Ï
        EPwm2Regs.ETPS.bit.INTPRD = ET_DISABLE;             //³õÊ¼ÇåÁã£¬ÓÃÓÚÍ£Ö¹Ê±¼ä¼ÆÊý
        EPwm2Regs.ETCLR.all = 0x000F;                       //Çå³ýËùÓÐÊÂ¼þ
        //2018-10-26 end
	
	//	EALLOW;
	//	EPwm2Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO;
	//	EPwm2Regs.HRCNFG.bit.EDGMODE = HR_FEP;
	//	EPwm2Regs.HRCNFG.bit.CTLMODE = HR_CMP;
	//	EPwm2Regs.CMPA.half.CMPAHR = 0x180;
	//	EDIS;

		// Config Epwm3 for HV
		EPwm3Regs.TBPRD = 166;
		EPwm3Regs.TBPHS.half.TBPHS = 0;
		EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// Count up/dn
		EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
		EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
		EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;		// Sync flow through 
		EPwm3Regs.TBCTL.bit.PHSDIR = TB_UP;				//Phase Direction
		//TBCLK=SYSCLKOUT/(HSPCLKDIV * CLKDIV)=SYSCLKOUT
		EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;				//Timebase clock pre-scale
		EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;			//High speed time pre-scale
        EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
		EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
        EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
        EPwm3Regs.AQCTLA.bit.PRD = AQ_SET;			// Set PWM3A on event : ctr = zero
		EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;			// Clear PWM3A on ctr = cmpa(up)		
		EPwm3Regs.AQCTLB.bit.PRD = AQ_SET;			
		EPwm3Regs.AQCTLB.bit.CBD = AQ_CLEAR;		
		
		EPwm3Regs.DBCTL.bit.OUT_MODE = 0x00;
		EPwm3Regs.DBCTL.bit.POLSEL = 0x00;
        EPwm3Regs.DBFED=0;	
		EPwm3Regs.CMPA.half.CMPA = 60;
        EPwm3Regs.CMPB =10;
        EPwm3Regs.TBCTR = 0;
	//	EALLOW;
	//	EPwm3Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO;
	//	EPwm3Regs.HRCNFG.bit.EDGMODE = HR_FEP;
    //	EPwm3Regs.HRCNFG.bit.CTLMODE = HR_CMP;
		//(frac(PWMDuty*PWMperiod)*MEP_SF) << 8) + 0180h
		//MEP_SF = (10ns/180ps) = 55
		//eg. (frac(0.2016*500)*55<<8)+180h 
   		//CMPA = 100, CMPAHR=0.8*55<<8+180h =44<<8+180h = 2D80h
	//	EPwm3Regs.CMPA.half.CMPAHR = 0x0180;
	//	EDIS;

		EPwm4Regs.TBPRD = 166;
		EPwm4Regs.TBPHS.half.TBPHS =0;
		EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// Count up/dn
		EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;
		EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
		EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;		// Sync flow through 
		EPwm4Regs.TBCTL.bit.PHSDIR = TB_UP;				//Phase Direction
		//TBCLK=SYSCLKOUT/(HSPCLKDIV * CLKDIV)=SYSCLKOUT
		EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;				//Timebase clock pre-scale
		EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;			//High speed time pre-scale

		// for PWM  wave
		EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
		EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
		EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
	
		EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;			// Set PWM4A on event , ctr = prd
		EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;		// Clear PWM4A on ctr = cmpa(dn)
		EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;			// Set PWM4A on event , ctr = prd
		EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;		// Clear PWM4A on ctr = cmpa(dn)
		
		EPwm4Regs.DBCTL.bit.OUT_MODE = 0x00;
		EPwm4Regs.DBCTL.bit.POLSEL = 0x00;           //4A----4B  CHANGE IN PCB   MXH 2010/12/15
		EPwm4Regs.DBFED = 0;       //only for 2812    BJG
		EPwm4Regs.CMPA.half.CMPA = 6;
	    EPwm4Regs.CMPB =156;
	//	EALLOW;
	//	EPwm4Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO;
	//	EPwm4Regs.HRCNFG.bit.EDGMODE = HR_FEP;
	//	EPwm4Regs.HRCNFG.bit.CTLMODE = HR_CMP;
	//	EPwm4Regs.CMPA.half.CMPAHR = 0x180;
	//	EDIS;
		
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
	EDIS; 
}

//---------------------------------------------------------------------------
// Example: InitEPwmGpio: 
//---------------------------------------------------------------------------
// This function initializes GPIO pins to function as ePWM pins
//
// Each GPIO pin can be configured as a GPIO pin or up to 3 different
// peripheral functional pins. By default all pins come up as GPIO
// inputs after reset.  
// 


void InitEPwm1Gpio(void)
{
   EALLOW;
   
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Enable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Enable pull-up on GPIO1 (EPWM1B)   
   
/* Configure ePWM-1 pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be ePWM1 functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B
   
    EDIS;
}

void InitEPwm2Gpio(void)
{
   EALLOW;
	
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Enable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;    // Enable pull-up on GPIO3 (EPWM3B)

/* Configure ePWM-2 pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be ePWM2 functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B
   
    EDIS;
}

void InitEPwm3Gpio(void)
{
   EALLOW;
	
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;    // Enable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;    // Enable pull-up on GPIO5 (EPWM3B)

/* Configure ePWM-2 pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be ePWM2 functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // Configure GPIO4 as EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   // Configure GPIO5 as EPWM3B
   
    EDIS;
}

void InitEPwm4Gpio(void)
{
   EALLOW;
	
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;    // Enable pull-up on GPIO6 (EPWM4A)
    GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;    // Enable pull-up on GPIO7 (EPWM4B)

/* Configure ePWM-2 pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be ePWM2 functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;   // Configure GPIO6 as EPWM4A
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;   // Configure GPIO7 as EPWM4B
   
    EDIS;
}

//---------------------------------------------------------------------------
// Example: InitEPwmSyncGpio: 
//---------------------------------------------------------------------------
// This function initializes GPIO pins to function as ePWM Synch pins
//
/*
void InitEPwmSyncGpio(void)
{

   EALLOW;

// Configure EPWMSYNCI  
   
// Enable internal pull-up for the selected pins 
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

   GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;    // Enable pull-up on GPIO6 (EPWMSYNCI)
// GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;   // Enable pull-up on GPIO32 (EPWMSYNCI)    

// Set qualification for selected pins to asynch only 
// This will select synch to SYSCLKOUT for the selected pins.
// Comment out other unwanted lines.

   GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;   // Synch to SYSCLKOUT GPIO6 (EPWMSYNCI)
// GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 0;  // Synch to SYSCLKOUT GPIO32 (EPWMSYNCI)    

// Configure EPwmSync pins using GPIO regs
// This specifies which of the possible GPIO pins will be EPwmSync functional pins.
// Comment out other unwanted lines.   

   GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 2;    // Enable pull-up on GPIO6 (EPWMSYNCI)  
// GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 2;   // Enable pull-up on GPIO32 (EPWMSYNCI)    



// Configure EPWMSYNC0  

// Enable internal pull-up for the selected pins 
// Pull-ups can be enabled or disabled by the user. 
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

// GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;    // Enable pull-up on GPIO6 (EPWMSYNC0)
   GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;   // Enable pull-up on GPIO33 (EPWMSYNC0)    

// GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 3;    // Enable pull-up on GPIO6 (EPWMSYNC0)
   GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 2;   // Enable pull-up on GPIO33 (EPWMSYNC0)  //mxh/2010/12/15  

}
*/

//===========================================================================
// End of file.
//===========================================================================
