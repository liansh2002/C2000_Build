;//########################################################################
;//
;// FILE:  DSP280x_CodeStartBranch.asm	
;//
;// TITLE: Branch for redirecting code execution after boot. 
;//
;// For these examples, code_start is the first code that is executed after
;// exiting the boot ROM code. 
;//
;// The codestart section in the linker cmd file is used to physically place
;// this code at the correct memory location.  This section should be placed 
;// at the location the BOOT ROM will re-direct the code to.  For example, 
;// for boot to FLASH this code will be located at 0x3f7ff6. 
;//
;// In addition, the example DSP280x projects are setup such that the codegen
;// entry point is also set to the code_start label.  This is done by linker 
;// option -e in the project build options.  When the debugger loads the code,
;// it will automatically set the PC to the "entry point" address indicated by
;// the -e linker option.  In this case the debugger is simply assigning the PC, 
;// it is not the same as a full reset of the device. 
;// 
;// The compiler may warn that the entry point for the project is other then
;//  _c_init00.  _c_init00 is the C environment setup and is run before 
;// main() is entered. The code_start code will re-direct the execution 
;// to _c_init00 and thus there is no worry and this warning can be ignored. 
;// 
;//#########################################################################
;// Running on TMS320LF280xA                 
;// External clock is 20MHz, PLL * 10/2 , CPU-Clock 100 MHz	      
;// Date: from 2005/12/28
;// Version:1.00     Change Date:  		
;//#########################################################################


***********************************************************************

    .ref _c_int00
    .global code_start
    .ref _B1RstEntryPointer

***********************************************************************
* Function: codestart section
*
* Description: Branch to code starting point
***********************************************************************

    .sect "codestart"

code_start:

        ;LB _c_int00         ;Branch to start of boot.asm in RTS library
		LB  _B1RstEntryPointer
		
;end codestart section

	.end
	
;//===========================================================================
;// End of file.
;//===========================================================================
