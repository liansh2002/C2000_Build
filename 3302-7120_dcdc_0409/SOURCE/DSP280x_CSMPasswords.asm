;//###########################################################################
;//
;// FILE:	DSP280x_CSMPasswords.asm
;//
;// TITLE:	DSP280x Code Security Module Passwords.
;// 
;// DESCRIPTION:
;//
;//         This file is used to specify password values to
;//         program into the CSM password locations in Flash
;//         at 0x3F7FF8 - 0x3F7FFF.
;//
;//         In addition, the reserved locations 0x3F7F80 - 0X3f7ff5 are 
;//         all programmed to 0x0000
;//
;//###########################################################################
;// 		
;//###########################################################################

; The "csmpasswords" section contains the actual CSM passwords that will be
; linked and programmed into to the CSM password locations (PWL) in flash.  
; These passwords must be known in order to unlock the CSM module. 
; All 0xFFFF's (erased) is the default value for the password locations (PWL).

; It is recommended that all passwords be left as 0xFFFF during code
; development.  Passwords of 0xFFFF do not activate code security and dummy 
; reads of the CSM PWL registers is all that is required to unlock the CSM.  
; When code development is complete, modify the passwords to activate the
; code security module.

      .sect "csmpasswds"

      .int	0xffff		;//
      .int	0xffff		;//
      .int	0xffff		;//
      .int	0xffff		;//
      .int	0xffff		;//
      .int	0xffff		; //                           ;//
      .int	0xffff		;//                            ;//
      .int	0xffff		;//
	
;----------------------------------------------------------------------

;
; 
; 
; 
;  

; 

        .sect "csm_rsvd"
        .loop (3F7FF5h - 3F7F80h + 1)
              .int 0x0000
        .endloop

;//===========================================================================
;// End of file.
;//===========================================================================

      
