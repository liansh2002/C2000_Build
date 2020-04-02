;	The second level boot loader and application depend on these
;	constants being exactly where they are.  Their addresses are
;  hard coded in the linkder command files because the DSECT
;	liker directive does not prevent overlaying code on the
;  specified section but instead puts code wherever it wants.

;	THE NUMBER AND LOCATION OF THESE CONSTANTS CANNOT BE CHANGED
;  WITHOUT ALSO CHANGING THE SECOND LEVEL BOOT AND APPLICATION!

	.global _ApplicationValidPointer,	_Application_Valid
	.global	_ServiceDogPointer,		_ServiceDog
	.global	_B1AppEntryPointer	,	_B1_App_Entry
	.global	_B1RstEntryPointer	,	_B1_Rst_Entry
	.global _InitFlashPointer    ,   _InitFlash
	.global _MemCopyPointer  , _MemCopy
	
	.global _InitSysCtrlPointer  ,  _InitSysCtrl
	.global _InitGpioPointer  ,  _InitGpio
	.global _InitECanPointer   ,  _InitECan
	.global _InitI2CPointer  ,   _InitI2C
	.global _InitSciPointer  ,   _InitSci
	.global _DisableDogPointer  ,  _DisableDog
	.global _EnableDogPointer  ,  _EnableDog
	.global _InitCpuTimersPointer  ,  _InitCpuTimers
	;.global _InitECapAPWMPointer , _InitECapAPWM
	.global _InitAdcPointer  , _InitAdc
	.global _InitEPwmPointer , _InitEPwm
	.global _InitPieCtrlPointer , _InitPieCtrl
	.global _InitPieVectTablePointer , _InitPieVectTable
	.global _InitPeripheralsPointer , _InitPeripherals
	
	.global _InitRAMPointer   ,_InitRAM
	.global _USER_ISR_Pointer  , _USER_ISR
	
	.global _Boot_Version
	
	.sect	".bootvec"

_ApplicationValidPointer	LB			_Application_Valid
_ServiceDogPointer			LB			_ServiceDog
_B1AppEntryPointer			LB			_B1_App_Entry
_B1RstEntryPointer			LB		    _B1_Rst_Entry
_InitFlashPointer			LB 			_InitFlash
_MemCopyPointer      LB 	_MemCopy
_InitSysCtrlPointer  LB 	_InitSysCtrl
_InitGpioPointer     LB 	_InitGpio
_InitECanPointer     LB 	_InitECan
_InitI2CPointer      LB 	_InitI2C
_InitSciPointer      LB 	_InitSci
_DisableDogPointer   LB 	_DisableDog
_EnableDogPointer   LB 	_EnableDog
_InitCpuTimersPointer  LB  	_InitCpuTimers
;_InitECapAPWMPointer LB 	_InitECapAPWM
_InitAdcPointer     LB  	_InitAdc
_InitEPwmPointer 	LB 		_InitEPwm
_InitPieCtrlPointer LB 		_InitPieCtrl
_InitPieVectTablePointer  LB	_InitPieVectTable
_InitPeripheralsPointer	  LB	_InitPeripherals

_InitRAMPointer    LB   _InitRAM
_USER_ISR_Pointer  LB _USER_ISR
_dummy_jump_1	   LB			_dummy_jump_1
_Boot_Version   .int   100

;********************************************************************
	
	.ref _c_int00
	.global  _BOOT_MAIN
	.global  _uiFromApplication,_uiMdlAddr
__stack:	.usect	".stack",0
	
	.sect	"FlashBoot"
_B1_Rst_Entry: 
	;MOV     @SP,#0x0400;
	MOV 	AL,#__stack
	MOV     @SP,AL
	SPM     0
	SETC    OBJMODE
	CLRC    AMODE
	SETC    M0M1MAP
	CLRC    PAGE0    ;28x mode
	MOVW    DP,#0x0000
	CLRC    OVM
	ASP
	
	SETC	  INTM
	MOV  	AL,#0
	MOVW		DP,#_uiMdlAddr ; _uiMdlAddr = 0
	MOV 	@_uiMdlAddr,AL
	LB 		ContinueB1
_B1_App_Entry:
	SETC INTM	
	MOV 	AL,#0FFH
	
ContinueB1:
	MOVW		DP,#_uiFromApplication
	MOV 	@_uiFromApplication,AL
		
	LCR	_BOOT_MAIN
	SETC	  INTM				; disable interrupts
justdie:
	LB    	  justdie
	

;***********************************************************************	
;	.global  _Application_Entry_Point
	.global  _Software_Version,_Hardware_Version,_Baseline_Version	
	.global  _main,_exit
	.sect	".appvec"	
_Application_Entry_Point:   
 	MOV 	AL,#__stack
	MOV     @SP,AL
    LCR 	_main  ;LB     _c_int00
_exit_entry			LCR 	_exit
_Software_Version  .int  100
_Hardware_Version  .int  100
_Baseline_Version  .int  205

;***********************************************************************
	.global _CRC16_CheckWord
	.sect	"appcrc"			
_CRC16_CheckWord:
	.int	0x1070
;***********************************************************************
																				
	.end
;===========================================================================
; No more.
;===========================================================================

