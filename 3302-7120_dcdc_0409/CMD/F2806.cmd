/*
// TI File $Revision: /main/4 $
// Checkin $Date: September 7, 2005   17:40:15 $
//###########################################################################
//
// FILE:	F2806.cmd
//
// TITLE:	Linker Command File For F2806 Device
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//##########################################################################
*/

/* ======================================================
// For Code Composer Studio V2.2 and later
// ---------------------------------------
// In addition to this memory linker command file, 
// add the header linker command file directly to the project. 
// The header linker command file is required to link the
// peripheral structures to the proper locations within 
// the memory map.
//
// The header linker files are found in <base>\DSP280x_Headers\cmd
//   
// For BIOS applications add:      DSP280x_Headers_BIOS.cmd
// For nonBIOS applications add:   DSP280x_Headers_nonBIOS.cmd    
========================================================= */

/* ======================================================
// For Code Composer Studio prior to V2.2
// --------------------------------------
// 1) Use one of the following -l statements to include the 
// header linker command file in the project. The header linker
// file is required to link the peripheral structures to the proper 
// locations within the memory map                                    */

/* Uncomment this line to include file only for non-BIOS applications */
/* -l DSP280x_Headers_nonBIOS.cmd */

/* Uncomment this line to include file only for BIOS applications */
/* -l DSP280x_Headers_BIOS.cmd */

/* 2) In your project add the path to <base>\DSP280x_headers\cmd to the
   library search path under project->build options, linker tab, 
   library search path (-i).
/*========================================================= */

/* Define the memory block start/length for the F2806  
   PAGE 0 will be used to organize program sections
   PAGE 1 will be used to organize data sections

   Notes: 
         Memory blocks on F2806 are uniform (ie same
         physical memory) in both PAGE 0 and PAGE 1.  
         That is the same memory region should not be
         defined for both PAGE 0 and PAGE 1.
         Doing so will result in corruption of program 
         and/or data. 
         
         L0 and L1 memory blocks are mirrored - that is
         they can be accessed in high memory or low memory.
         For simplicity only one instance is used in this
         linker file. 

         Flash sectors can be combined if required. 
*/

MEMORY
{
PAGE 0:    /* Program Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE1 for data allocation */

   RAML0BOOT_PRG   : origin = 0x0093E0, length = 0x000020 /* on-chip RAM block L0 */
   RAML0_PRG   : origin = 0x008000, length = 0x00013E0     /* on-chip RAM block L0 */
   OTP         : origin = 0x3D7800, length = 0x000400     /* on-chip OTP */  
   APPVEC 	   : origin = 0x3F0000, length = 0x000020
   FLASH       : origin = 0x3F0020, length = 0x005FDF     /* on-chip FLASH */
   APPCRC	   : origin = 0x3F5FFF, length = 0x000001
   BOOTVEC 	   : origin = 0x3F6000, length = 0x000040
   LIB_FLASH   : origin = 0x3F6040, length = 0x000270
   FLASH_BOOT  : origin = 0x3F62B0, length = 0x001CD0     /* on-chip FLASH */   
   CSM_RSVD    : origin = 0x3F7F80, length = 0x000076     /* Part of FLASHA.  Program with all 0x0000 when CSM is in use. */
   BEGIN       : origin = 0x3F7FF6, length = 0x000002     /* Part of FLASHA.  Used for "boot to Flash" bootloader mode. */
   CSM_PWL     : origin = 0x3F7FF8, length = 0x000008     /* Part of FLASHA.  CSM password locations in FLASHA */
   
   ROM         : origin = 0x3FF000, length = 0x000FC0     /* Boot ROM */
   RESET       : origin = 0x3FFFC0, length = 0x000002     /* part of boot ROM  */
   VECTORS     : origin = 0x3FFFC2, length = 0x00003E     /* part of boot ROM  */

PAGE 1 :   /* Data Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE0 for program allocation */
           /* Registers remain on PAGE1                                                  */

   RAMM0       : origin = 0x000000, length = 0x000400     /* on-chip RAM block M0 */
   RAMM1       : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
   RAML0_DATA  : origin = 0x009400, length = 0x000C00     /* on-chip RAM block L0 */

}

/* Allocate sections to memory blocks.
   Note:
         codestart user defined section in DSP28_CodeStartBranch.asm used to redirect code 
                   execution when booting to flash
         ramfuncs  user defined section to store functions that will be copied from Flash into RAM
*/ 
 

 SECTIONS
{
 
   /* Allocate program areas: */
   .cinit              : > FLASH      PAGE = 0   
/*   .pinit              : > FLASH_BOOT1,     PAGE = 0  */
   .text               : > FLASH      PAGE = 0   
   .init			   : LOAD = FLASH_BOOT      PAGE = 0      
	{
		DSP280x_Adc.obj (.text)
		DSP280x_EPwm.obj (.text)
		DSP280x_InitPeripherals.obj (.text)
		DSP280x_PieCtrl.obj (.text)
		DSP280x_PieVect.obj (.text)
		/*DSP280x_ECap.obj (.text)*/
		DSP280x_CpuTimers.obj (.text)
		DSP280x_DefaultIsr.obj (.text)
		DSP280x_ECan.obj (.text)
		DSP280x_Gpio.obj (.text)
		DSP280x_I2C.obj (.text)
		DSP280x_Sci.obj (.text)
		DSP280x_MemCopy.obj (.text)
		DSP280x_SysCtrl.obj (.text)
   }
   
   codestart           : > BEGIN       PAGE = 0
   
   bootramfuncs        : LOAD = FLASH_BOOT, 
                         RUN = RAML0BOOT_PRG, 
                         LOAD_START(_BootRamfuncsLoadStart),
                         LOAD_END(_BootRamfuncsLoadEnd),
                         RUN_START(_BootRamfuncsRunStart),
                         PAGE = 0


  
   		ramfuncs         : LOAD = FLASH, 
                         RUN = RAML0_PRG 
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         PAGE = 0
            
   	Flash28_API:
    	{
        -lFlash2806_API_V302.lib(.econst) 
        -lFlash2806_API_V302.lib(.text)
        -lFlash2806_API_V302.lib (.cinit)
    	}              LOAD = FLASH_BOOT, 
                       RUN = RAML0_PRG  
                       LOAD_START(_Flash28_API_LoadStart),
                       LOAD_END(_Flash28_API_LoadEnd),
                       RUN_START(_Flash28_API_RunStart),
                       PAGE = 0   
  	
   	.cal  : > LIB_FLASH      PAGE = 0      
	{
		-lrts2800_ml.lib <fd_mpy.obj u_div.obj i_tofs.obj 
		l_div.obj fd_tol.obj fs_tofd.obj fs_tou.obj 
		boot.obj exit.obj _lock.obj> (.text)
				
		-lrts2800_ml.lib <exit.obj _lock.obj> (.cinit)
		
		-lIQmath.lib <IQ10div.obj IQ10toF.obj > (IQmath)
	}     	
                    
   .appvec             : > APPVEC   PAGE = 0
   appcrc              : > APPCRC   PAGE = 0
   .bootvec               : > BOOTVEC  PAGE = 0
   FlashBoot           : > FLASH_BOOT      PAGE = 0
   
   csmpasswds          : > CSM_PWL     PAGE = 0
   csm_rsvd            : > CSM_RSVD    PAGE = 0
   
   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1       PAGE = 1
   .ebss               : > RAML0_DATA       PAGE = 1
   /*.esysmem            : > RAMM0       PAGE = 1 */
   /* Initalized sections go in Flash */
   /* For SDFlash to program these, they must be allocated to page 0 */
   .econst             : > FLASH      PAGE = 0
   .switch             : > FLASH      PAGE = 0      
   .const              : > FLASH      PAGE = 0
   
   /* Allocate IQ math areas: */
   IQmath              : > FLASH       PAGE = 0                  /* Math Code */
   IQmathTables        : > ROM         PAGE = 0, TYPE = NOLOAD   /* Math Tables In ROM */

   /* .reset is a standard section used by the compiler.  It contains the */ 
   /* the address of the start of _c_int00 for C Code.   /*
   /* When using the boot ROM this section and the CPU vector */
   /* table is not needed.  Thus the default type is set here to  */
   /* DSECT  */ 
   .reset              : > RESET,      PAGE = 0, TYPE = DSECT
   vectors             : > VECTORS     PAGE = 0, TYPE = DSECT
	

/*******************************************************************/
/*	These addresses reside in the RAM section and are hard coded */
/* in the linker command files because the liker cannot be trusted */
/* to put them where we want them.  They have to stay the same for */
/* the first and second level boot loaders and the application.    */
/*******************************************************************/
	{_uiCanErrorCount = 0x07F0;}
	{_uiRecvOk = 0x07F1;}
	{_ulCanBootDest = 0x07F2;}
	{_ulCanBootMDL = 0x07F4;}
	{_ulCanBootMDH = 0x07F6;}
	{_ulCanBootBlockAddr = 0x07F8;}
	{_uiCanBootBlockLength = 0x07FA;}
	{_uiCanSlaveAddress = 0x07FB;}	
	{_uiSendID = 0x07FC;}
	{_uiSendOk = 0x07FD;}
	{_uiMdlAddr = 0x07FE;}	
	{_uiFromApplication = 0x07FF;}
	{_Application_Entry_Point   = 0x3F0000;}
}

/*
//===========================================================================
// No more.
//===========================================================================
*/
