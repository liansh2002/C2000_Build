/*
// TI File $Revision: /main/3 $
// Checkin $Date: April 6, 2005   11:26:37 $
//###########################################################################
//
// FILE:    DSP280x_Headers_nonBIOS.cmd
//
// TITLE:   DSP280x Peripheral registers linker command file 
//
// DESCRIPTION: 
// 
//          This file is for use in Non-BIOS applications.
//
//          Linker command file to place the peripheral structures 
//          used within the DSP280x headerfiles into the correct memory
//          mapped locations.
//
//          This version of the file includes the PieVectorTable structure.
//          For BIOS applications, please use the DSP280x_Headers_BIOS.cmd file
//          which does not include the PieVectorTable structure.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################
*/

MEMORY
{
 PAGE 0:    /* Program Memory */

 PAGE 1:    /* Data Memory */
 
   DEV_EMU     : origin = 0x000880, length = 0x000180     /*  */
   FLASH_REGS  : origin = 0x000A80, length = 0x000060     /*  */
   CSM         : origin = 0x000AE0, length = 0x000010     /*  */
   
   ADC_MIRROR  : origin = 0x000B00, length = 0x000010     /* */
   
   CPU_TIMER0  : origin = 0x000C00, length = 0x000008     /*  */
   CPU_TIMER1  : origin = 0x000C08, length = 0x000008     /*  )*/
   CPU_TIMER2  : origin = 0x000C10, length = 0x000008     /*  )*/

   PIE_CTRL    : origin = 0x000CE0, length = 0x000020     /*  */
   PIE_VECT    : origin = 0x000D00, length = 0x000100     /*  */

   ECANA       : origin = 0x006000, length = 0x000040     /*  */ 
   ECANA_LAM   : origin = 0x006040, length = 0x000040     /*  */
   ECANA_MOTS  : origin = 0x006080, length = 0x000040     /* */
   ECANA_MOTO  : origin = 0x0060C0, length = 0x000040     /* */
   ECANA_MBOX  : origin = 0x006100, length = 0x000100     /*  */

   ECANB       : origin = 0x006200, length = 0x000040     /*  */ 
   ECANB_LAM   : origin = 0x006240, length = 0x000040     /* */
   ECANB_MOTS  : origin = 0x006280, length = 0x000040     /* */
   ECANB_MOTO  : origin = 0x0062C0, length = 0x000040     /*  */
   ECANB_MBOX  : origin = 0x006300, length = 0x000100     /*  */

   EPWM1       : origin = 0x006800, length = 0x000022     /*  */
   EPWM2       : origin = 0x006840, length = 0x000022     /*  */
   EPWM3       : origin = 0x006880, length = 0x000022     /*  */
   EPWM4       : origin = 0x0068C0, length = 0x000022     /* */
   EPWM5       : origin = 0x006900, length = 0x000022     /*  */
   EPWM6       : origin = 0x006940, length = 0x000022     /*  */

   ECAP1       : origin = 0x006A00, length = 0x000020     /* */
   ECAP2       : origin = 0x006A20, length = 0x000020     /* */
   ECAP3       : origin = 0x006A40, length = 0x000020     /*  */
   ECAP4       : origin = 0x006A60, length = 0x000020     /* */         
 
   EQEP1       : origin = 0x006B00, length = 0x000040     /*  */
   EQEP2       : origin = 0x006B40, length = 0x000040     /* */   

   GPIOCTRL    : origin = 0x006F80, length = 0x000040     /* */
   GPIODAT     : origin = 0x006FC0, length = 0x000020     /* */
   GPIOINT     : origin = 0x006FE0, length = 0x000020     /* */
                 
   SYSTEM      : origin = 0x007010, length = 0x000020     /* */
   SPIA        : origin = 0x007040, length = 0x000010     /* */
   SCIA        : origin = 0x007050, length = 0x000010     /* */
   XINTRUPT    : origin = 0x007070, length = 0x000010     /* */

   ADC         : origin = 0x007100, length = 0x000020     /**/
   SPIB        : origin = 0x007740, length = 0x000010     /*  */

   SCIB        : origin = 0x007750, length = 0x000010     /**/
   SPIC        : origin = 0x007760, length = 0x000010     /*  */

   SPID        : origin = 0x007780, length = 0x000010     /*  */
   
   I2CA        : origin = 0x007900, length = 0x000040     /* */

   DCISR	    : origin = 0x009000, length = 0x000080		/*  */
  /* BootVariable : origin = 0x0007C0, length = 0x000040		*/

   CSM_PWL     : origin = 0x3F7FF8, length = 0x000008     /*  */
}

 
SECTIONS
{
   PieVectTableFile : > PIE_VECT,   PAGE = 1

/*** Peripheral Frame 0 Register Structures ***/
   DevEmuRegsFile    : > DEV_EMU,     PAGE = 1
   FlashRegsFile     : > FLASH_REGS,  PAGE = 1
   CsmRegsFile       : > CSM,         PAGE = 1
   AdcMirrorFile     : > ADC_MIRROR,  PAGE = 1   
   CpuTimer0RegsFile : > CPU_TIMER0,  PAGE = 1
   CpuTimer1RegsFile : > CPU_TIMER1,  PAGE = 1
   CpuTimer2RegsFile : > CPU_TIMER2,  PAGE = 1  
   PieCtrlRegsFile   : > PIE_CTRL,    PAGE = 1      

/*** Peripheral Frame 1 Register Structures ***/
   SysCtrlRegsFile   : > SYSTEM,      PAGE = 1
   SpiaRegsFile      : > SPIA,        PAGE = 1
   SciaRegsFile      : > SCIA,        PAGE = 1
   XIntruptRegsFile  : > XINTRUPT,    PAGE = 1
   AdcRegsFile       : > ADC,         PAGE = 1
   SpibRegsFile      : > SPIB,        PAGE = 1
   ScibRegsFile      : > SCIB,        PAGE = 1
   SpicRegsFile      : > SPIC,        PAGE = 1
   SpidRegsFile      : > SPID,        PAGE = 1
   I2caRegsFile      : > I2CA,        PAGE = 1

/*** Peripheral Frame 2 Register Structures ***/
   ECanaRegsFile     : > ECANA,       PAGE = 1
   ECanaLAMRegsFile  : > ECANA_LAM    PAGE = 1   
   ECanaMboxesFile   : > ECANA_MBOX   PAGE = 1
   ECanaMOTSRegsFile : > ECANA_MOTS   PAGE = 1
   ECanaMOTORegsFile : > ECANA_MOTO   PAGE = 1
   
   ECanbRegsFile     : > ECANB,       PAGE = 1
   ECanbLAMRegsFile  : > ECANB_LAM    PAGE = 1   
   ECanbMboxesFile   : > ECANB_MBOX   PAGE = 1
   ECanbMOTSRegsFile : > ECANB_MOTS   PAGE = 1
   ECanbMOTORegsFile : > ECANB_MOTO   PAGE = 1
   
   EPwm1RegsFile     : > EPWM1        PAGE = 1   
   EPwm2RegsFile     : > EPWM2        PAGE = 1   
   EPwm3RegsFile     : > EPWM3        PAGE = 1   
   EPwm4RegsFile     : > EPWM4        PAGE = 1   
   EPwm5RegsFile     : > EPWM5        PAGE = 1   
   EPwm6RegsFile     : > EPWM6        PAGE = 1
   
   ECap1RegsFile     : > ECAP1        PAGE = 1   
   ECap2RegsFile     : > ECAP2        PAGE = 1   
   ECap3RegsFile     : > ECAP3        PAGE = 1   
   ECap4RegsFile     : > ECAP4        PAGE = 1

   EQep1RegsFile     : > EQEP1        PAGE = 1   
   EQep2RegsFile     : > EQEP2        PAGE = 1               

   GpioCtrlRegsFile  : > GPIOCTRL     PAGE = 1
   GpioDataRegsFile  : > GPIODAT      PAGE = 1
   GpioIntRegsFile   : > GPIOINT      PAGE = 1
 
   DcdcisrFile	 	: > DCISR		  PAGE = 1    
   /*BootVariableFile	: > BootVariable	  PAGE = 1
   DcdcmainFile     : > DCMAIN		  PAGE = 1     */
/*** Code Security Module Register Structures ***/
   CsmPwlFile        : > CSM_PWL,     PAGE = 1
}


/*
//===========================================================================
// End of file.
//===========================================================================
*/
