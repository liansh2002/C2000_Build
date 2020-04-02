;#########################################################
;
; FILE:	DSP280x_pwd.asm
;
; TITLE:	DSP280x password initialization.
;
;//#########################################################
;// Running on TMS320LF2801   DCDC part  h641AU2               
;// External clock is 20MHz, PLL * 10/2 , CPU-Clock 100 MHz	      
;// Date: from May 23, 2005 to Jan 10, 2007  , (C)  mhp
;// Version:1.00     Change Date: Jan 04, 2007 , 
;//#########################################################

;#include "DSP280x_Device.h"     // DSP280x Headerfile Include File

	.title	"DSP280x_pwd.asm"
	.sect	"csmpasswds"
pwd:
	.int	0x55AA		;PWL0 
	.int	0xAA55		;PWL1
	.int	0xAA55		;PWL2
	.int	0x55AA		;PWL3
	.int	0x55AA		;PWL4
	.int	0xAA55		;PWL5
	.int	0xAA55		;PWL6
	.int	0x55AA		;PWL7
    
