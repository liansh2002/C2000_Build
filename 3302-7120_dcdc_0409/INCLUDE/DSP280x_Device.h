// TI File $Revision: /main/2 $
// Checkin $Date: January 25, 2006   14:03:29 $
//###########################################################################
//
// FILE:   DSP280x_Device.h
//
// TITLE:  DSP280x Device Definitions.
//
//###########################################################################
// $TI Release: DSP280x V1.30 $
// $Release Date: February 10, 2006 $
//###########################################################################

#ifndef DSP280x_DEVICE_H
#define DSP280x_DEVICE_H


#ifdef __cplusplus
extern "C" {
#endif

struct uintData
{
	unsigned int uiLD;
	unsigned int uiHD;
};
typedef struct uintData uintStructData;

typedef	union 
{
	unsigned int id1 ;
	struct   packed_data1
	{
		 unsigned highchar1 : 8;
		 unsigned lowchar1  : 8;
	}bitdata1;                                                                                                    
}ubitint1 ;

typedef	union 
{
	unsigned int id ;
	struct   packed_data
	{
		 unsigned highchar : 8;
		 unsigned lowchar  : 8;
	}bitdata;                                                                                                    
}ubitint ;
 
typedef	union 
{
	float fd ;
	ubitint intdata[2];   
	long	lData;	//changed by lvyg
//	intStructData iData; //changed by lvyg
}ubitfloat ;


typedef union
{
	unsigned int msgaddid[3];
	struct
	{
		unsigned ADDIDL	:16;		/* reserved */
		unsigned ADDIDH	:16;		/* reserved */
		unsigned ADDADD	:8;		/* com attribute */
		unsigned ADDOK	:8;		/* 1:continuous frame */
	}msgaddbit;
}frameaddid;

typedef union
{
	unsigned long uld;
	ubitint intdata[2];  
}ubitulong ;

// add for bootloader:  mhp 20061102
typedef	union 
{
	unsigned long ulData;
	uintStructData uiData;
}ulongBitunion;

typedef struct 
{
	unsigned int	*uiLoadAddress;
	unsigned int	uiLength;
} APPLICATIONHEADER; 

#define   TARGET   1
//---------------------------------------------------------------------------
// User To Select Target Device:

#define   DSP28_2808   0
#define   DSP28_2801   0
#define   DSP28_2802   0
#define   DSP28_2806   TARGET

#define   HV   TARGET
//#define   LV   TARGET

#ifdef HV
#define   MDL_TYPE		20
#define   VOLT_SHIFT		12			// 
#define   CURR_SHIFT		13			// 
#define   POWER_ALL		304*32//
#endif


//---------------------------------------------------------------------------
// Common CPU De  finitions:
//

extern cregister volatile unsigned int IFR;
extern cregister volatile unsigned int IER;

#define  EINT   asm(" clrc INTM")
#define  DINT   asm(" setc INTM")
#define  ERTM   asm(" clrc DBGM")
#define  DRTM   asm(" setc DBGM")
#define  EALLOW asm(" EALLOW")
#define  EDIS   asm(" EDIS")
#define  ESTOP0 asm(" ESTOP0")

#define M_INT1  0x0001
#define M_INT2  0x0002
#define M_INT3  0x0004
#define M_INT4  0x0008
#define M_INT5  0x0010
#define M_INT6  0x0020
#define M_INT7  0x0040
#define M_INT8  0x0080
#define M_INT9  0x0100
#define M_INT10 0x0200
#define M_INT11 0x0400
#define M_INT12 0x0800
#define M_INT13 0x1000
#define M_INT14 0x2000
#define M_DLOG  0x4000
#define M_RTOS  0x8000


//---------------------------------------------------------------------------
// For Portability, User Is Recommended To Use Following Data Type Size
// Definitions For 16-bit and 32-Bit Signed/Unsigned Integers:
//

#ifndef DSP28_DATA_TYPES
#define DSP28_DATA_TYPES
typedef int             int16;
typedef long            int32;
typedef unsigned int    Uint16;
typedef unsigned long   Uint32;
typedef float           float32;
typedef long double     float64;
#endif

// shift from DSP280x_Examples.h
/*-----------------------------------------------------------------------------
      Specify the clock rate of the CPU (SYSCLKOUT) in nS.

      Take into account the input clock frequency and the PLL multiplier
      selected in step 1.
 
      Use one of the values provided, or define your own.
      The trailing L is required tells the compiler to treat 
      the number as a 64-bit value.  

      Only one statement should be uncommented.

      Example:  CLKIN is a 20MHz crystal. 
 
                In step 1 the user specified PLLCR = 0xA for a 
                100Mhz CPU clock (SYSCLKOUT = 100MHz).  

                In this case, the CPU_RATE will be 10.000L
                Uncomment the line:  #define CPU_RATE   10.000L
-----------------------------------------------------------------------------*/

#define CPU_RATE   10.000L   // for a 100MHz CPU clock speed (SYSCLKOUT)
//#define CPU_RATE   13.330L   // for a 75MHz CPU clock speed (SYSCLKOUT)
//#define CPU_RATE   20.000L   // for a 50MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   33.333L   // for a 30MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   41.667L   // for a 24MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   50.000L   // for a 20MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   66.667L   // for a 15MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE  100.000L   // for a 10MHz CPU clock speed  (SYSCLKOUT)

//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Include Example Header Files:
//

#include "DSP280x_GlobalPrototypes.h"         // Prototypes for global functions within the .c files.

#include "DSP280x_ePwm_defines.h"             // Macros used for PWM examples. 
#include "DSP280x_I2C_defines.h"              // Macros used for I2C examples.

#define PARTNO_2808 0x3C
#define PARTNO_2806 0x34
#define PARTNO_2802 0x24
#define PARTNO_2801 0x2C
#define MSG_SIZE_8		0x08
#define BOOTLOADER_PFC 	0x01
#define BOOTLOADER_DCDC 0x10 
#define FIRSTTRIES	255	// We try 255 times on the first header
#define	TRIES	5
#define	TIME_1000mS				50000
#define	TIME_200mS				10000
#define	BLOCKACKNOWLEDGE			0  	// Block Acknowledge
#define	MISSINGDATAPACKET			1  	// Missing Data Packet in Block
#define	LASTBLOCKACKNOWLEDGE		2  	// Last Block Acknowledge
#define	INVALIDAPPLICATION			3  	//	Invalid Application Image
#define	DEVICEMISPROGRAMMED			4  	//	Device Misprogrammed 
#define SIZE_OF_BOOTLOADERHEADER	4
#define PWL0         (volatile Uint16*)0x003F7FF8   /* Password 0 */
#define PWL1         (volatile Uint16*)0x003F7FF9   /* Password 1 */
#define PWL2         (volatile Uint16*)0x003F7FFA   /* Password 2 */
#define PWL3         (volatile Uint16*)0x003F7FFB   /* Password 3 */
#define PWL4         (volatile Uint16*)0x003F7FFC   /* Password 4 */
#define PWL5         (volatile Uint16*)0x003F7FFD   /* Password 5 */
#define PWL6         (volatile Uint16*)0x003F7FFE   /* Password 6 */
#define PWL7         (volatile Uint16*)0x003F7FFF   /* Password 7 */ 
#define	FLASHROWSIZE				128	// 128 words = 256 bytes, the maximum size
#define	RAMBLOCKSIZE				128 
#define ENP_LO_SN_ADDR		120		//Ä£¿éÐòÁÐºÅ
#define ENP_HI_SN_ADDR		124  
// Include files not used with DSP/BIOS
#ifndef DSP28_BIOS
#include "DSP280x_DefaultISR.h"
#endif


// DO NOT MODIFY THIS LINE.  
#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_RATE) - 9.0L) / 5.0L)



//  add for bootloader : mhp 20061102
extern Uint16 BootRamfuncsLoadStart;
extern Uint16 BootRamfuncsLoadEnd;
extern Uint16 BootRamfuncsRunStart;

extern volatile Uint16 uiFromApplication;
extern volatile	ulongBitunion ulCanBootDest; 
extern volatile	ulongBitunion ulCanBootMDL;
extern volatile	ulongBitunion ulCanBootMDH;
extern volatile	ulongBitunion ulCanBootBlockAddr;	
extern volatile	Uint16   uiCanBootBlockLength; 
extern volatile	Uint16   uiCanSlaveAddress;

extern volatile	Uint16  uiCanErrorCount; 
extern volatile	Uint16  uiSendID;   
extern volatile	Uint16 uiSendOk; 
extern volatile	Uint16 uiRecvOk; 
extern volatile	Uint16 uiMdlAddr; 


extern void ApplicationValidPointer(void);
extern void ServiceDogPointer(void);
extern void B1AppEntryPointer(void);
extern void B1RstEntryPointer(void);
extern void InitFlashPointer(void);
extern void MemCopyPointer(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);
extern void InitSysCtrlPointer(void);
extern void InitGpioPointer(void);
extern void InitECanPointer(void);
extern void InitI2CPointer(void);
extern void InitSciPointer(void);
extern void InitCpuTimersPointer(void);
extern void DisableDogPointer(void);
extern void EnableDogPointer(void);
extern void	KickDogPointer(void);

extern void	InitAdcPointer(void);
extern void	InitEPwmPointer(void);
extern void InitPieCtrlPointer(void);
extern void InitPieVectTablePointer(void);
extern void InitPeripheralsPointer(void);

extern void InitRAMPointer(void);
extern void InitRAM(void);
extern interrupt void USER_ISR_Pointer(void);
extern void Application_Entry_Point(void);

extern void CRC16_CheckWord(void);
extern void Boot_Version(void);
extern void Software_Version(void);
extern void Hardware_Version(void);
extern void Baseline_Version(void);


//---------------------------------------------------------------------------
// Include All Peripheral Header Files:
//

#include "DSP280x_Adc.h"                // ADC Registers
#include "DSP280x_DevEmu.h"             // Device Emulation Registers
#include "DSP280x_CpuTimers.h"          // 32-bit CPU Timers
#include "DSP280x_ECan.h"               // Enhanced eCAN Registers
#include "DSP280x_ECap.h"               // Enhanced Capture
#include "DSP280x_EPwm.h"               // Enhanced PWM 
#include "DSP280x_EQep.h"               // Enhanced QEP
#include "DSP280x_Gpio.h"               // General Purpose I/O Registers
#include "DSP280x_I2c.h"                // I2C Registers
#include "DSP280x_PieCtrl.h"            // PIE Control Registers
#include "DSP280x_PieVect.h"            // PIE Vector Table
#include "DSP280x_Spi.h"                // SPI Registers
#include "DSP280x_Sci.h"                // SCI Registers
#include "DSP280x_SysCtrl.h"            // System Control/Power Modes
#include "DSP280x_XIntrupt.h"           // External Interrupts
#include "DCDC100A_isr.h"

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of DSP280x_DEVICE_H definition


//===========================================================================
// End of file.
//===========================================================================
