//########################################################
//
// FILE:   dcdc100a.h
//
// TITLE:  dcdc100a variables definitions.
//
//########################################################
// Running on TMS320LF280xA   DCDC part   H641AU211                
// External clock is 20MHz, PLL * 10/2 , CPU-Clock 100 MHz	      
// Date: from March 16, 2006 to Nov 30, 2006  , (C) www & mhp & lsy
// Version:1.00     Change Date: April 4, 2006 , (C) www & mhp & lsy					  	
//########################################################


/*------------结构定义-------------------------*/
/*struct uintData
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
	//intStructData iData; //changed by lvyg
}ubitfloat ;


typedef union
{
	unsigned int msgaddid[3];
	struct
	{
		unsigned ADDIDL	:16;		// reserved 
		unsigned ADDIDH	:16;		// reserved 
		unsigned ADDADD	:8;		// com attribute 
		unsigned ADDOK	:8;		// 1:continuous frame 
	}msgaddbit;
}frameaddid;

typedef union
{
	unsigned long uld;
	ubitint intdata[2];  
}ubitulong ;*/

/*------------常量定义-------------------------*/

#define RECOK		0xf0		/* communication normal */ 		
#define CRCCOM		0xf1 		/* CRC check fail */
#define XORCOM		0xf1 		/* XOR check fail */
#define NOVCOM		0xf2		/* invalid command  */
#define NULLCOM		0xf3		/* other fault  */

//#define ADREF		3       
//
#define iq10_VFACTOR	40560
#define iq12_VFACTOR	162242
//
//
#define iq10_IFACTOR	671089 //1011896	
//
#define iq12_VOLTFACTOR		21178
//
//
#define iq10_CURRFACTOR		80//
#define iq10_TEMPFACTOR  	1920       
#define iq10_TEMPFACTOR_Inside  1920 

#define iq10_TEMPUP	112640//107520  //

#define MAX_NUM		62
#define POWER_MAX	15360000


/*------------变量定义-------------------------*/
struct ECAN_REGS ECana32Temp;

unsigned int	uiAdcData0[62];			//  
signed long	lAdcSum0;				//  
unsigned int	uiAdcData1[62];			//  
signed long	lAdcSum1;				//  
unsigned int	uiAdcData2[62];			// adc data buffer 
signed long	lAdcSum2;				//  
unsigned int	uiAdcData3[62];			// adc data buffer 
signed long	lAdcSum3;				// 
unsigned int	uiAdcPoint;

ubitfloat	iq10AcCurrFt;
ubitfloat	iq10MdlPowerFt;
ubitfloat	iq10MdlCurrFt;
ubitfloat	iq10MdlVoltFt;			// 
ubitfloat	iq10MdlVolt;				// 
ubitfloat	iq10MdlVoltPre;				// 
ubitfloat	iq10MdlCurr;				// 
ubitfloat	iq10SoftLimit;
ubitfloat	iq10WalkinLimit;
ubitfloat	iq10MdlAvgCurr;			//  
ubitfloat	iq10CurrDisp;				//
ubitfloat	iq10MdlLimit;				// 
ubitfloat	iq10TempCtrl;			// 
ubitfloat	iq10TempMOld;
ubitfloat	iq10TempDc;				// 
ubitfloat	iq10TempCOld;
ubitfloat	iq10ReonTime;			// 
ubitfloat	iq10LoadSstartTime;		// 
ubitfloat	iq10MdlVoltUp;			// 
ubitfloat	iq10MdlTempUp;			// 
ubitfloat	iq10SetVolt;				//
ubitfloat	iq10SetLimit;				// 
ubitfloat	iq10SetPower;			// 
ubitfloat	iq10SetVoltTrue;			// 
ubitfloat	iq10SetLimitTrue;			// 
ubitfloat	iq10SetPowerTrue;		// 
ubitfloat	iq10SetPowerWant;		// 
ubitfloat	iq10SetSysTmp;			// 
ubitfloat	iq10RunTime;				// 
ubitfloat	iq10MaintainTimes;		//  
ubitfloat	iq10OpenTime;			// 
ubitfloat	iq10Tmp;
signed long	lPowerMax;
signed long	lCurrRate;


ubitfloat	fRdTemp;
ubitfloat	iq10VoltAdd;				// 
ubitfloat	iq12Tmp;
ubitfloat	tagStatusSd;
ubitfloat	iq10AdjTmp;
ubitfloat	iq10AcVab;
ubitfloat	iq10AcVbc;
ubitfloat	iq10AcVca;
ubitfloat	iq10Vaca;
ubitfloat	iq10Vacb;
ubitfloat	iq10Vacc;

ubitfloat	iq10CurrMax;

ubitfloat		iq12VoltSampSysa;			//
ubitfloat		iq12VoltSampSysb;			//
ubitfloat		iq12VoltConSysa;			//
ubitfloat		iq12VoltConSysb;			//
ubitfloat		iq10CurrSampSysa;			//
ubitfloat		iq10CurrSampSysb;			//
ubitfloat		iq10CurrConSysa;			//
ubitfloat		iq10CurrConSysb;			//
ubitfloat		iq10PowerConSysa;			//
ubitfloat		iq10PowerConSysb;			//
ubitfloat		iq10AcaSampSysa;
ubitfloat		iq10AcaSampSysb;
ubitfloat		iq10AcbSampSysa;
ubitfloat		iq10AcbSampSysb;
ubitfloat		iq10AccSampSysa;
ubitfloat		iq10AccSampSysb;
ubitfloat		iq12VoltConSysa_inv;

unsigned int	uiMdlCalibra;

unsigned int	uiMdlCurr;				// 
unsigned int	uiMdlCurrOld;				// 

unsigned int	uiLedTimer;
unsigned int	uiFreStepTimer;

unsigned int	uiBaseTimer; 			// 
unsigned int	uiBox0FailTimer;
unsigned int	uiSoftOvpTimer;
unsigned int	uiSequeOnTimer;		//  
unsigned int	uiWalkinTimer;
unsigned int	uiCanFailTime;		// 
unsigned int	uiSciFailTime;		// 
unsigned int	ui60STimer;			// 
unsigned int	uiOTjudgeTimer;		// 
unsigned int	uiMaskWarnCnt;		// 

unsigned int	uiCount1;
unsigned int	uiCount2;
unsigned int	uiCount3;
unsigned int	uiCount4;
//unsigned int	uiCntMax1;
//unsigned int	uiCntMax2;
//unsigned int	uiCntMax3;
//unsigned int	uiCntMax4;
/**************************************************************/ 
unsigned int	uiNodeId0H;		
unsigned int	uiNodeId0L;	
unsigned int	uiNodeId1H;
unsigned int	uiNodeId1L;
unsigned int	uiCharactData0H;		
unsigned int	uiCharactData0L;		
unsigned int	uiVersionNoHw;	
unsigned int	uiVersionNoSw;	
unsigned int	uiMaintainData0H;	
unsigned int	uiMaintainData0L;	
unsigned int	uiMaintainData1H;	
unsigned int	uiMaintainData1L;	
unsigned int	uiBarCode0H;	
unsigned int	uiBarCode0L;	
unsigned int	uiBarCode1H;	
unsigned int	uiBarCode1L;	
unsigned int	uiBarCode2H;	
unsigned int	uiBarCode2L;	
unsigned int	uiBarCode3H;	
unsigned int	uiBarCode3L;	
unsigned int    RAMP_MAX;

unsigned int	uiPfcOnOff;
unsigned int	uiActionReady;			//
unsigned int	uiCloseTime;				// 
unsigned int	uiOpenTime;				//
unsigned int	uiMdlStatus;				//  
unsigned int	uiIoStatu[2];				// 
unsigned int	uiMdlCtrl;
unsigned int	uiPfcCtrl;
unsigned int	uiRdMemNum;
unsigned int	uiRdNumSet;
unsigned int  *  uiMemScopeAdd0;
unsigned int  *  uiMemScopeAdd1;
unsigned int	uiI2CErrorType;
unsigned int    VASAMPLE_INV;
unsigned int     il732ref;
unsigned int     il607ref;
unsigned int     il680ref;
unsigned int     il550ref;
unsigned int     il520ref;
unsigned int     il430ref;
unsigned int     il420ref;
unsigned int     il360ref;
unsigned int     il350ref;
unsigned int     il310ref;
unsigned int     il303ref;
//volatile unsigned int uiMdlAddr;

/**************************************************************/
unsigned char	ucOffFlag;
unsigned char	ucLightFlag;			//
unsigned char	ucPermitAdjust;
unsigned char	ucOpenOrder;
unsigned char	ucOverVoltStatus;	// 
unsigned char	ucOverVoltCount;	// 
unsigned char	ucLimitStatus;		// 
unsigned char	ucOverVdcBak;
unsigned char	ucOverTempBak;		
unsigned char	ucCurrUnBal;
unsigned char	ucCurrSerUnBal;
unsigned char	ucCircuitCalFlag;		
unsigned char	ucIoSamConFlag;	    
unsigned char	ucAdcSampFlag;		
unsigned char	ucCan5DatFlag;		
unsigned char	ucCan5SynFlag;		
unsigned char	ucMemScopeFlag;
unsigned char	ucLoadShareFlag;	
unsigned char	ucIoSamOVFlag;
unsigned char	ucSciDatFlag;
unsigned char	ucWalkinFlag;

unsigned char	ucOffPowerBak;
unsigned char	ucTempDeviceBak;
unsigned char	ucTempAlarm;
unsigned char	ucWriteNumber;
//unsigned char	ucMdlAddr;				// 
unsigned char	ucAddressSmall;			//
unsigned char	ucAddressCalFlag;
unsigned char	ucReadEepromOk;		// 
unsigned char	ucBroadCastCom;
unsigned char	ucCANSend;
unsigned char	ucMsgType;
unsigned char	ucErrType;
unsigned char	ucAddIdentify;		
unsigned char	ucAddIdentifyNum;
unsigned char	ucMemoryScope;
unsigned int	uiCanDataId;	
unsigned int	uiCurrCtrlFlag;
unsigned int	uiSetEepromAdd;
unsigned int	uiEepromFlag;
unsigned int	uiVpfcSet;
unsigned int    uiVpfcSet_true;
unsigned int	uiRdMsgBuffer[4];
unsigned char	ucVoltAdjDelta;		// 
unsigned char	ucIDError;			// 
unsigned char	ucMdlOnoff[64];		// 
frameaddid	fMdlAddID[64];
ubitfloat		iq10MdlCurrent[64];			//  
signed int		iMdlCurrDelta[64];		// 
signed int		iCurrDelta;				

/**************************************************************/

// Define variable 
unsigned char ucReceiveOk;			// 
unsigned char ucRxBuffer[60]; //   
unsigned char ucTxBuffer[60]; //   
unsigned char ucSCISendFlag;
unsigned char ucSendCid;
unsigned int uiPfcSendId;
unsigned int uiRxPoint;					//  
unsigned int uiTxPoint;					//  
unsigned int uiTxLength;				//       
unsigned int uiVoltAca;
unsigned int uiVoltAcb;
unsigned int uiVoltAcc;
unsigned int uiPfcStatus;
//unsigned int uiPfcCtrl;
unsigned int uiCheckChar;
unsigned int uiSendChar;
unsigned int uiSciComFail;
unsigned int uiSciComTime;
unsigned int uiSciErrType;
unsigned int uiVoltPfcAll;
unsigned int uiVoltPfcUp;
unsigned int uiVoltPfcDn;
unsigned int uiTempPfc;
unsigned int uiVacaSampSysa;
signed int iVacaSampSysb;
unsigned int uiVacbSampSysa;
signed int iVacbSampSysb;
unsigned int uiVaccSampSysa;
signed int  iVaccSampSysb;
//unsigned int uiVpfcAllSampSys;
//unsigned int uiVpfcUpSampSys;
//unsigned int uiVpfcDnSampSys;
//unsigned int uiVpfcAllCtrlSys;
//unsigned int uiVpfcUpCtrlSys;
//unsigned int uiVpfcDnCtrlSys;
unsigned int uiMemAddr0Value;
unsigned int uiMemAddr1Value;
unsigned int uiMemWrAddr;
unsigned int uiMemWrValue;
unsigned int uiMemRdAddr0;
unsigned int uiMemRdAddr1;
unsigned int uiEpromWrAddr;
unsigned int uiEpromWrValue;
unsigned int uiMdlType;
unsigned int uiAcaVoltOld;
unsigned int uiAcbVoltOld;
unsigned int uiAccVoltOld;


unsigned int uiForbidConnect;
unsigned int uiPositionMode;
unsigned int uiWrSigErr;
unsigned int uiAcLimitFlag;
unsigned int uiCodeType;

// add for self diagnosis
unsigned int uiMasterMdl;
unsigned int uiFuseBroken;
unsigned int uiSelfChkStart;
unsigned int uiSelfChkFlag;
unsigned int uiSelfChkTimer;
unsigned long ulSelfChkTimer;
unsigned int uiSelfChkSend;
unsigned int uiSelfChkEn;
unsigned int uiSelfChkStat;
unsigned int uiSelfchkAddr;
unsigned int uiSelfChking;
unsigned int uiSelfChkAttr;		// 00:	
unsigned int uiSelfChkDelay;
unsigned int uiNextDelay;
unsigned int uiSelfTimeOut;
signed int iVoltSelfAdj;
unsigned long	ulTime1;
unsigned long	ulTime2;
ubitfloat iq10VoltOld;
ubitfloat iq10SetVoltOld;
unsigned int	uiMdlVolt[64];				// 
//unsigned int	uiSetVolt[64];				// 

unsigned int uiEfficfactor[14];

unsigned int uiDCVoltref;
unsigned int uiAdjTimer;
unsigned int uiAdjFlag;
signed int iAdjDelta;
signed int  AvgCurr;
signed int lCurr8A;
signed int lCurr10A;
extern volatile unsigned int uiMdlAddr;

ubitfloat iq10VrefGainCurr;                 //2018-10-19 输出电压根据输出电流补偿值。


/**************************************************************/
interrupt void dcdc_isr(void);
//interrupt void dcdc_load_isr1(void);

//void dcdcloop_init(void);		/* DCDC loop variable initialization */		

void	vDataInit(void);				/*  */
void	vLimitInit(void);				/*  */
void	vMtimerTreat(void);				/*  */
void	vSstartCtrl(void);				/*  */
void	vIoPortCtrl(void);				/*  */
void	vAdcTreatment(void);			/*  */
//void	vHARDADRESS(void);
void	vAvgCurrCal(void);					/* */
void	vCircuitCal(void);					/*  */    
void	vAddressCal(void);					/*  */ 
void	vGetCanData(void);		    		/*  */
void	vRrpProtocol(void);		   			/*  */
void	vRmpProtocol(ubitfloat tempfloat);		/* */
void	uSecond(unsigned int uS);
void vWriteE2PROMSIG(void);
void vWriteFloatDataSig(unsigned int uAddress, ubitfloat fTemp);
ubitfloat	fReadFloatDataThree(unsigned int uAddress);/* */
unsigned char ucWriteFloatDataThree(unsigned int uAddress, ubitfloat fTemp);
unsigned int    uiI2caWriteData(unsigned int uAddress, ubitfloat fTemp);
unsigned int    uiI2caReadData(unsigned int uAddress);
unsigned char  ucAscToHex(unsigned char ascl, unsigned char asch);
unsigned int    uiHexToAsc(unsigned char hex);
void vTxdAsc(unsigned int len);
void	vSendSciData(void);		    		/* */
void	vGetSciData(void);		    		/*  */

//
float fStatusTrans_New(void);//
void vSelfDiagnotics(void);
void main(void);


