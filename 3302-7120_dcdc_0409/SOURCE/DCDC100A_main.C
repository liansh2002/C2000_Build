//#########################################################
//
// FILE:    dcdc100a_main.c
//
// TITLE:   DSP280x dcdc100a main Functions.
//
//#########################################################
//#########################################################

#include "IQmathLib.h"
#include "DSP280x_Device.h"     // 
#include "dcdc100a_main.h"
#include "config.h"
#include "var.h"

#ifndef GLOBAL_Q
#define GLOBAL_Q 10 /* */
#endif

//20150616
#ifdef dNOT_CAN
    #define INIT_POWER_OFF 0
    #define CAN_ERR_POWER_OFF 0
#else
    #define INIT_POWER_OFF 1
    #define CAN_ERR_POWER_OFF 1
#endif    

#define cDcdcFirmwareVer        0x0108  //
unsigned int uwPfcFirmwareVer = 0x0101; //
#define cCurrDeltaMax           400//
unsigned int uwAveCurrCalIndex = 0; //
unsigned int adresstemp1 = 0;
unsigned int adresstemp2 = 0;
unsigned int hardadress = 0;

#define cOpCurr6Amp             6144
#define cOpCurr5Amp             5120
#define cOpCurr4Amp             4096
#define cOpCurr3Amp             3072
#define cOpCurr2Amp             2048
#define cOpCurr1Amp             1024
#define cOpCurr0Amp5            512

unsigned int leddelay = 1;
unsigned int VOLTcal_delay = 1;
unsigned int uiMdlNumber = 0;      //
unsigned char ubAddrCalcTimer = 0; //
ubitfloat ubitfCodeChkRd0;
ubitfloat ubitfCodeChkRd1;
ubitfloat ubitfCodeChkRd2;

void sCodeChk(void);

#define cInqNewRec 0xB0
int uwTurnOnFirstRpl = 0;
int uwOnOffDCDC = 0;
int iq10PfcFanRatio_Temp;
int iq10PfcFanRatio_Curr;
ubitfloat ubitTestFlag;

void main(void)
{
    InitSysCtrlPointer();

    InitGpioPointer();

    DINT;

    InitRAMPointer();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrlPointer();

    InitPieVectTablePointer();

    EALLOW;
    PieVectTable.XINT13 = &dcdc_isr;
    EDIS;
    CpuTimer1Regs.TCR.bit.TIE = 1;

    MemCopyPointer(&BootRamfuncsLoadStart, &BootRamfuncsLoadEnd, &BootRamfuncsRunStart);
    MemCopyPointer(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

    InitFlashPointer();

    uiMdlAddr = 0;

    InitPeripheralsPointer();

    EnableDogPointer();

    vDataInit();

    vLimitInit();

    IER |= M_INT13;
    EINT;
    ERTM;

    sCodeChk();

#if INIT_POWER_OFF == 1
    uiMdlStatus |= 0x0008;
    uiActionReady = 0;
    uiMdlStatus = uiMdlStatus | 0x0010;
    uwOnOffDCDC = 0x01;
#endif

    //init_can_extend();              //2018-11-01 根据鲁能协议补充CAN初始化
    
    for(;;)
    {
        vSstartCtrl();

        vMtimerTreat();

        vIoPortCtrl();

        vGetCanData();

        vAddressCal();

        vSendSciData();

        vAdcTreatment();

        vCircuitCal();

        vGetCanData();

        vAddressCal();

        vRrpProtocol();

        vAvgCurrCal();

        vGetSciData();
    }
}

/**********************************************************/
/*                                 */
/**********************************************************/
void vMtimerTreat()
{
    signed long lreftmp;
    unsigned long  Dcdc_uTmpLong;
    signed long lTmp;

    Dcdc_uTmpLong = Dcdcisr.ul_Dcdc_Power_Lim / Dcdcisr.ul_Dcdc_Volt_Ave;
    if (Dcdc_uTmpLong >= 31800) {
        Dcdcisr.ui_Dcdc_Power_Lim = 31800;
    }
    else if (Dcdc_uTmpLong < 100) {
        Dcdcisr.ui_Dcdc_Power_Lim = 100;
    }
    else {
        Dcdcisr.ui_Dcdc_Power_Lim = (unsigned int)Dcdc_uTmpLong;
    }

    if (CpuTimer0Regs.TCR.bit.TIF)          //
    {
        CpuTimer0Regs.TCR.bit.TIF = 1;

        ServiceDogPointer();
        uiLedTimer++;
        //
        uiCanFailTime++;
        uiBaseTimer++;
        uiWalkinTimer++;
        uiBox0FailTimer++;
        ui60STimer++;
        uiOTjudgeTimer++;
        uiAdjTimer++ ;
        leddelay++;
        //

        if(leddelay >= 1500)
        {
            leddelay = 1500;
        }

        if((uiActionReady == 1) && (Dcdcisr.ui_Dcdc_Start_Cnt <= 200))
        {
            Dcdcisr.ui_Dcdc_Start_Cnt++;
        }

        if(uiActionReady >= 3)
        {
            VOLTcal_delay++;
        }
        if(VOLTcal_delay >= 1000)
        {
            VOLTcal_delay = 1000;
        }

        if(Dcdcisr.ui_Dcdc_Short_Flag == 1)
        {

            Dcdcisr.ui_short_ontime++;
            if(Dcdcisr.ui_short_ontime <= 6000) //
            {
                Dcdcisr.ui_short_permit = 1;
            }
            else
            {
                Dcdcisr.ui_short_permit = 0; //
                Dcdcisr.test = 200;
            }
        }

        else
        {
            Dcdcisr.ui_short_permit = 1;
            Dcdcisr.ui_short_ontime = 1;
        }
        ///////////////////////////////////////////////////////////
        if((Dcdcisr.ui_short_permit == 0) || (Dcdcisr.ui_poweron_off == 0)) //
        {
            ucOffFlag = ucOffFlag | 0x22;
            uiMdlStatus = uiMdlStatus | 0x0008;   //
            uiActionReady = 0;
            uiPfcCtrl = uiPfcCtrl & 0xffdf;       //
            //                //
            ubitTestFlag.lData |= 0x00020000; //
        }

        //
        Dcdcisr.ui_Dcdc_Volt_Ref = uiDCVoltref;
        //

        if (ui60STimer > 12000) {
            ui60STimer = 12000;
        }

        if (uiOTjudgeTimer > 24000) {
            uiOTjudgeTimer = 24000;
        }

        if (uiLedTimer >= 100)              //
        {
            ucLightFlag = 1;
            uiLedTimer = 0;
            uiSequeOnTimer = uiSequeOnTimer + 1;
        }
        if(uiCanFailTime > 8000) //        //
        {
            uiMdlStatus = uiMdlStatus | 0x0400;
            uiMdlStatus = uiMdlStatus & 0xBFEF;     //
            uiCanFailTime = 0;

#if CAN_ERR_POWER_OFF==1
            uiMdlStatus |= 0x0008; //
            uiActionReady = 0;
            uiMdlStatus = uiMdlStatus | 0x0010;
            uwOnOffDCDC = 0x01;

            iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
            lreftmp = _IQ10mpy(iq10_VFACTOR, (_IQ10mpy((iq10SetVoltTrue.lData), (iq12VoltConSysa.lData >> 2))) + (iq12VoltConSysb.lData >> 2));
            if (lreftmp > (long)32400 << 10)
            {
                uiDCVoltref = 32400;
            }
            else
            {
                uiDCVoltref = (unsigned int)(lreftmp >> 10);
            }
            Dcdcisr.ui_Dcdc_Volt_Ref = uiDCVoltref;
#endif




            //
            //
            //
        }

        if (uiSciComFail == 0)              //
        {
            uiSciFailTime = uiSciFailTime + 1;
            if(uiSciFailTime > 1000) //2000) //
            {
                uiSciComFail = 1;
                uiSciFailTime = 2000;
            }
        }

        if (uiSciComFail == 1)
        {
            uiSciComTime++;
            if (uiSciComTime > 1000) //3000)//
            {
                uiSciComTime = 0;
                uiSciComFail = 2;           //
            }
        }

        //
        if (uiSciComFail)
        {
            if ((uiPfcStatus & 0x01c0) == 0x0100) //
            {
                uiVoltAca = 0;
                uiVoltAcb = 0;
                uiVoltAcc = 0;
            }
            uiVoltPfcUp = 0;
            uiVoltPfcDn = 0;
            uiTempPfc = Dcdcisr.ui_Dcdc_Temp_Ctrl + 5;
            uiPfcStatus = uiPfcStatus & 0xfde9;     //
            uiMdlStatus = uiMdlStatus & 0xfefb;     //
        }

        uiMaskWarnCnt++;                        //
        if (uiMaskWarnCnt >= 24000) {   //
            uiMaskWarnCnt = 24000;
        }

        //
        if((uiBaseTimer % 240) == 0)  //
        {
            ubAddrCalcTimer = 1;
        }
        //

        if (uiBaseTimer % 607 == 0)
        {
            ucCan5SynFlag = 1;          //
        }
        else if (uiBaseTimer % 101 == 0)
        {
            if (uiSelfChkStart) {
                uiSelfChkSend = 1;    //
            }
        }
        else if (uiBaseTimer % 79 == 0)
        {
            ucCan5DatFlag = 1;          //
        }

        if (uiBaseTimer % 23 == 0)
        {
            ucCircuitCalFlag = 1;           //
            vWriteE2PROMSIG();
        }
        if (uiBaseTimer % 19 == 0)
        {
            ucSCISendFlag = 1;
            ucSciDatFlag = 1;               //
        }

        //
        if((ucIoSamOVFlag > 0) && (ucIoSamOVFlag < 16))
        {
            if(uiBaseTimer % 5 == 0) {
                ucIoSamConFlag = 1;
            }
        } else
        {
            if (uiBaseTimer % 13 == 0)
            {
                ucIoSamConFlag = 1;         //
            }
        }

        if (uiBaseTimer % 4 == 0)
        {
            ucAdcSampFlag = 1;          //
        }

        if (ucOverVoltStatus == 2)
        {
            uiOpenTime++;
            if(uiOpenTime >= 1000)          //
            {
                ucOverVoltStatus = 3;
            }
        }

        if (ucOverVoltStatus >= 1)
        {
            uiCloseTime++;
            if(uiCloseTime >= (unsigned int)(iq10ReonTime.lData >> 10) * 200) //
            {
                ucOverVoltStatus = 0;
                ucOverVoltCount = 0;
                uiCloseTime = 0;
            }
        }

        if (uiWalkinTimer >= (unsigned int)(iq10LoadSstartTime.lData >> 10) / 100) //
        {
            ucWalkinFlag = 1;
            uiWalkinTimer = 0;
        }

        if (uiBox0FailTimer >= 1600)
        {
            uiBox0FailTimer = 1600;
            iq10MdlAvgCurr.lData = iq10MdlCurr.lData;
        }

        if (uiSequeOnTimer >= 2000) {
            uiSequeOnTimer = 2000;
        }

        uiPfcOnOff = (uiPfcOnOff << 1) | GpioDataRegs.GPADAT.bit.GPIO14;

        if (uiActionReady >= 2)
        {
            //
            if (Dcdcisr.ui_Dcdc_debug) {
                ucOffFlag = ucOffFlag & 0xfd;
            }
            else
            {
                //
                //

                //
                if(((uiPfcOnOff & 0x000f) == 0x000f) || (uiMdlStatus & 0x2104) || (uiPfcCtrl & 0x0001)) //
                {
                    ucOffFlag = ucOffFlag | 0x22;
                    uiMdlStatus = uiMdlStatus | 0x0008;     //
                    uiActionReady = 0;
                    uiPfcCtrl = uiPfcCtrl & 0xffdf;         //
                    //              //
                    ubitTestFlag.lData |= 0x00020000; //
                }
                else if(ucOffFlag & 0x02)
                {
                    ucOffFlag = ucOffFlag & 0xfd;
                }
            }
        }

        //2018-10-19 增加根据输出电流补偿输出电压功能
        lreftmp = iq10CurrDisp.lData - (CURR_RATE / 2);
        lTmp = (512000l - iq10SetVolt.lData);
        if(lTmp > 0)
        {
            lTmp = _IQ10div(_IQ10mpy(lTmp, 123), 307200l);
        }
        else                                                                //输出大于500V时不补偿
        {
            lTmp = 0;
        }
        lreftmp = _IQ10mpy(lreftmp, lTmp);                                  //1A补偿0.1V
        if(lreftmp > 512)
        {
            lreftmp = 512;
        }
        if((3 > uiActionReady) || (0 != uiSelfChkEn))                       //开机未完成，或者非单机模式
        {
            lreftmp = 0;
        }    
        iq10VrefGainCurr.lData = lreftmp;
        //2018-10-19 end
    }
}
/**********************************************************/
/*                                */
/**********************************************************/
void vSelfDiagnotics()
{
    unsigned int i;

    if (uiNextDelay > 4)    //
    {
        uiNextDelay = 0;
        uiSelfchkAddr = uiSelfchkAddr + 1;
    }

    if ((uiSelfchkAddr >= MAX_NUM) && uiMasterMdl)
    {
        uiSelfchkAddr = MAX_NUM;
        uiSelfChkStart = 0;
        for (i = 0; i < MAX_NUM; i++)
        {
            if (uiMdlVolt[i] & 0x8000)      //
            {
                //          //
                ulSelfChkTimer = ulTime1 - 60;          //
                uiSelfChkFlag = 0;
                break;
            }
        }
        for (i = 0; i < MAX_NUM; i++)
        {
            uiMdlVolt[i] = 0;
            //
        }
    }

    if (uiSelfChkFlag == 2) {
        uiSelfChkFlag = 3;
    }

    if (uiSelfChkAttr == 0x01)
    {
        if ((uiMdlStatus & 0x010c) || ucIDError || (iq10MdlCurr.lData > 1536))
        {
            uiSelfChkAttr = 0x0;
            uiSelfChkSend = 1;
        }
        else if (!iVoltSelfAdj)
        {
            //
            iVoltSelfAdj = 75; //
            iq10VoltOld.lData = iq10MdlVolt.lData;
            iq10SetVoltOld.lData = iq10SetVoltTrue.lData;
        }
    }

    //      //
    if (uiSelfChkDelay >= 200)      //
    {
        uiSelfChkAttr  = 0x0;
        uiSelfChkSend = 1;      //
        if ((iq10SetVoltOld.lData == iq10SetVoltTrue.lData) && (iq10VoltOld.lData - iq10MdlVolt.lData > 1024))
        {
            uiMdlVolt[uiMdlAddr] |= 0x8000;
            uiFuseBroken++;
            if (uiFuseBroken == 3)
            {
                uiFuseBroken = 4;
                uiMdlStatus = uiMdlStatus | 0x0020;
                uiMdlCtrl = uiMdlCtrl | 0x8000;
                uiMdlVolt[uiMdlAddr] &= 0x7fff;
                uiEepromFlag = uiEepromFlag | 0x0002; //
            }
            if (uiMasterMdl) {
                uiSelfchkAddr = uiSelfChking + 1;
            }
        }
        else
        {
            uiFuseBroken = 0;       //
        }
        iVoltSelfAdj = 0;           //
        uiSelfChkDelay = 0;
    }

}

/****************************************************************/
/*                                                      */
/****************************************************************/
void   vSstartCtrl(void)
{

    signed long lreftmp;   

    if (uiActionReady == 0)
    {
        uwOnOffDCDC = 0x01;

        EPwm1Regs.AQCSFRC.all = 0x05;       //
        EPwm2Regs.AQCSFRC.all = 0x05;       //?
        EPwm3Regs.AQCSFRC.all = 0x05;       //
        EPwm4Regs.AQCSFRC.all = 0x05;       //
        if((uiPfcCtrl & 0x0001) == 1) //2017 20k
        {
            GpioDataRegs.GPASET.bit.GPIO16 = 1;
            //GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;  //2017 20k
        }
        VOLTcal_delay = 1;
        Dcdcisr.PWM_load_flag = 0x00;
        RAMP_MAX = dPERIOD_MAX;
        uiVpfcSet = dPFC_REF;//400;         //2018-11-28
        uiVpfcSet_true = dPFC_REF;//400;    //2018-11-28
        Dcdcisr.ui_Dcdc_Duty_Permit = 166 * 64;
        Dcdcisr.ui_Dcdc_Start_Cnt = 0;
        Dcdcisr.ui_Dcdc_Volt_Adc = 0;       // 801DH
        Dcdcisr.i_Dcdc_Volt_Err0 = 0;       // 801EH
        Dcdcisr.i_Dcdc_Volt_Err1 = 0;       // 801FH
        Dcdcisr.i_Dcdc_Volt_Err2 = 0;       // 8020H
        Dcdcisr.ui_Dcdc_Volt_Out0 = 0;          // 8021H
        Dcdcisr.ui_Dcdc_Volt_Out1 = 0;          // 8022H
        Dcdcisr.ui_Dcdc_Volt_Out2 = 0;          // 8023H
        Dcdcisr.ui_dead_comp_flag2 = 0;
        Dcdcisr.ui_dead_comp_flag = 1;
        Dcdcisr.ui_Dcdc_Curr_Adc = 0;       // 8024H
        Dcdcisr.i_Dcdc_Curr_Err0 = 0;       // 8025H
        Dcdcisr.i_Dcdc_Curr_Err1 = 0;       // 8026H
        Dcdcisr.i_Dcdc_Curr_Err2 = 0;       // 8027H
        Dcdcisr.ui_Dcdc_Curr_Out0 = 0;          // 8028H
        Dcdcisr.ui_Dcdc_Curr_Out1 = 0;          // 8029H
        Dcdcisr.ui_Dcdc_Curr_Out2 = 0;          // 802AH
        Dcdcisr.ui_Dcdc_Curr_Max = 0;
        Dcdcisr.ui_Dcdc_Duty_Ramp = 166 * 64;
        Dcdcisr.ui_Dcdc_Duty_Short = 160 * 64;

        ucOffFlag = 0;
        uiActionReady = 1;
        uiMdlStatus |= dMDL_OFF;
    }

    else if (uiActionReady == 1)
    {
        if (((((uiMdlStatus & 0xa187) == 0) && (uiPfcOnOff == 0) && (uiPfcStatus & 0x01)) && ((uiMdlStatus & dPSM_OFF) == 0)) && (Dcdcisr.ui_short_permit == 1) && (Dcdcisr.ui_poweron_off == 1) && ((uiPfcCtrl & 0x0001) == 0))
        {
            if((ucOpenOrder == 0) || (uiSequeOnTimer >= (20 + (unsigned int)(iq10OpenTime.lData >> 9)*uiMdlAddr)))
            {

                GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;  //2017 20k
                Dcdcisr.ui_Dcdc_Volt_Ref = uiDCVoltref;
                //
                if(uiMdlCtrl & 0x0001)
                {
                    iq10WalkinLimit.lData = 31;
                    iq10SoftLimit.lData = iq10WalkinLimit.lData;
                }
                else
                {
                    iq10SoftLimit.lData = iq10SetLimit.lData;
                }

                lreftmp = _IQ10mpy(lCurrRate, iq10SoftLimit.lData);
                lreftmp = _IQ10mpy(lreftmp, iq10CurrConSysa.lData) + iq10CurrConSysb.lData;
                lreftmp = _IQ10mpy(lreftmp, iq10_IFACTOR);

                if (lreftmp >= (long)31800 << 10) {
                    Dcdcisr.ui_Dcdc_Curr_Ref = 31800;
                }
                else if (lreftmp < (long)100 << 10) {
                    Dcdcisr.ui_Dcdc_Curr_Ref = 100;
                }
                else {
                    Dcdcisr.ui_Dcdc_Curr_Ref = (unsigned int)(lreftmp >> 10);
                }


                if(Dcdcisr.ui_Dcdc_Start_Cnt >= 200) //
                {
                    EPwm1Regs.AQCSFRC.all = 0x05;     //PWM1A,PWM1B force low
                    EPwm2Regs.AQCSFRC.all = 0x05;     //PWM2A,PWM2B force low?
                    EPwm3Regs.AQCSFRC.all = 0x05;     //PWM3A,PWM3B force low
                    EPwm4Regs.AQCSFRC.all = 0x05;     //PWM4A,PWM4B force low?

                    /*EPwm1Regs.TBPRD = 166;
                    EPwm1Regs.CMPA.half.CMPA = 113;      //
                    EPwm1Regs.CMPB = 103;
                    
                    EPwm2Regs.TBPRD = 166;
                    EPwm2Regs.CMPA.half.CMPA = 53;
                    EPwm2Regs.CMPB = 63;

                    EPwm3Regs.TBPRD = 166;
                    EPwm3Regs.CMPA.half.CMPA = 50;
                    EPwm3Regs.CMPB = 40;

                    EPwm4Regs.TBPRD = 166;
                    EPwm4Regs.CMPA.half.CMPA = 116;
                    EPwm4Regs.CMPB = 126;*/

                    Dcdcisr.ui_Dcdc_Start_Cnt = 0;
                    uiActionReady = 2;
                }

                uiPfcCtrl = uiPfcCtrl | 0x0020;
                uiMdlStatus &= ~dMDL_OFF; 

            }
        }
        else
        {
            uiSequeOnTimer = 0;
            uiMdlStatus |= dMDL_OFF;
        }
        uiOTjudgeTimer = 0;
    }
    else if (uiActionReady == 2)
    {
        if(uiMdlCtrl & 0x0001)
        {
            if (ucWalkinFlag)
            {
                ucWalkinFlag = 0;

                lreftmp = iq10WalkinLimit.lData + 5;
                if (lreftmp >= iq10SetLimit.lData)
                {
                    iq10WalkinLimit.lData = iq10SetLimit.lData;

                    if(Dcdcisr.ui_Dcdc_Duty_Ramp >= RAMP_MAX)
                    {
                        uiActionReady = 3;
                    }
                }
                else {
                    iq10WalkinLimit.lData = lreftmp;
                }

                iq10SoftLimit.lData = iq10WalkinLimit.lData;
            }
        }
#ifdef __DEBUG
        Dcdcisr.ui_Dcdc_Duty_Ramp += 5;
#else
        Dcdcisr.ui_Dcdc_Duty_Ramp += 1;  //2017 20k
#endif        
        if(Dcdcisr.ui_Dcdc_Duty_Ramp >= RAMP_MAX)
        {
            Dcdcisr.ui_Dcdc_Duty_Ramp = RAMP_MAX;

            if((uiMdlCtrl & 0x0001) == 0)
            {
#ifdef __DEBUG
                uiActionReady = 0;
#else
                uiActionReady = 3;
#endif                
                uwOnOffDCDC = 0X00;
            }
        }
    }
    if(uiActionReady < 2)
    {
        uiMdlStatus |= dMDL_OFF;
    }
}


/****************************************************************/
/*                                          */
/****************************************************************/
void  vIoPortCtrl(void)
{
    if (ucIoSamConFlag)
    {
        ucIoSamConFlag = 0;

        //SCI通讯异常
        if (uiSciComFail == 1)
        {
            uiMdlStatus = uiMdlStatus | dSCI_FAIL;
        }
        else if ((uiSciComFail == 2) && (uiSciFailTime <= 20))
        {
            uiMdlStatus = uiMdlStatus & ~dSCI_FAIL;
            uiSciComFail = 0;
        }
        //过温
        if ((iq10TempDc.lData >= iq10MdlTempUp.lData) || ((iq10TempCtrl.lData > ((long)70 << 10)) && (uiOTjudgeTimer >= 24000)) || (uiPfcStatus & 0x0200))
        {
            uiMdlStatus = uiMdlStatus | dOVER_TEMP;
        }
        else if ((iq10TempDc.lData <= (iq10MdlTempUp.lData - ((long)20 << 10))) && (iq10TempCtrl.lData <= ((long)61 << 10)) && (!(uiPfcStatus & 0x0200))) //hsr/20120629 过温点回差由10修改为20度
        {
            uiMdlStatus = uiMdlStatus & ~dOVER_TEMP;
        }
        //均流
        if((abs(iCurrDelta) == cCurrDeltaMax) && (iq10MdlAvgCurr.lData >= (long)5 << 10) && (uiActionReady >= 3) && ((uiMdlStatus & 0x0008) == 0) && (!Dcdcisr.ui_Dcdc_Run_Mode)) //mxhmah 20150305  系统平均电流大于5A才开始侦测是否不均流
        {
            if((iq10MdlCurr.lData < 1024) && (iCurrDelta >= cCurrDeltaMax)) //
            {
                ucCurrSerUnBal++;
                ucCurrUnBal = 0;
                if (ucCurrSerUnBal >= 300)
                {
                    ucCurrSerUnBal = 300;
                    uiMdlStatus = uiMdlStatus | dAVGCUR_FAIL;
                    ubitTestFlag.lData |= 0x00200000; //
                }
            }
            else if ((iq10MdlCurr.lData < 1024) && (iCurrDelta >= cCurrDeltaMax) && (ucCurrSerUnBal == 300)) //
            {
                ucCurrSerUnBal = 300;
                uiMdlStatus = uiMdlStatus | dAVGCUR_FAIL;
                ubitTestFlag.lData |= 0x00400000; //
            }
            else if (labs(iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >= (long)4 << 10) //
            {
                ucCurrSerUnBal = 0;
                ucCurrUnBal++;
                if (ucCurrUnBal >= 300)
                {
                    ucCurrUnBal = 300;
                    uiMdlStatus = uiMdlStatus | dAVGCUR_FAIL;
                    ubitTestFlag.lData |= 0x00800000; //
                }
            }
            else if((labs(iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >= (long)6 << 10) && (ucCurrUnBal == 300))
            {
                ucCurrUnBal = 300;
                uiMdlStatus = uiMdlStatus | dAVGCUR_FAIL;
                ubitTestFlag.lData |= 0x01000000; //
            }

            else
            {
                ucCurrSerUnBal = 0;
                ucCurrUnBal = 0;
                uiMdlStatus = uiMdlStatus & ~dAVGCUR_FAIL;
            }
        }
        else
        {
            ucCurrSerUnBal = 0;
            ucCurrUnBal = 0;
            uiMdlStatus = uiMdlStatus & ~dAVGCUR_FAIL;
        }

        //硬件输出过压检测
        if(GpioDataRegs.GPADAT.bit.GPIO15 == 1)
        {
            ucIoSamOVFlag++;
            if(ucIoSamOVFlag >= 16)
            {
                ucIoSamOVFlag = 16;
                uiMdlStatus = uiMdlStatus | dDCDC_HVSD;
            }
        }
        else
        {
            ucIoSamOVFlag = 0;
        }
        //软件输出过压
        if (uiMdlCtrl & 0x0004)
        {
            if (iq10MdlVolt.lData >= iq10MdlVoltUp.lData)
            {
                uiSoftOvpTimer++;
                if( uiSoftOvpTimer >= 10)
                {
                    uiSoftOvpTimer = 10;
                    uiMdlStatus = uiMdlStatus | dDCDC_HVSD;
                    //Dcdcisr.test=4;
                }
            }
            else {
                uiSoftOvpTimer = 0;
            }
        }
        else
        {
            if ((iq10MdlVolt.lData >= iq10MdlVoltUp.lData) && (iq10MdlCurr.lData >= (long)2 << 10))
            {
                uiSoftOvpTimer++;
                if( uiSoftOvpTimer >= 10)
                {
                    uiSoftOvpTimer = 10;
                    uiMdlStatus = uiMdlStatus | dDCDC_OV;
                    //Dcdcisr.test=3;
                }
            }
            else if ( iq10MdlVolt.lData <= iq10MdlVoltUp.lData - 512)
            {
                uiSoftOvpTimer = 0;
                uiMdlStatus = uiMdlStatus & ~dDCDC_OV;
            }
        }

        if ((uiMdlStatus & dDCDC_OV) && ((ucOverVoltStatus == 0) || (ucOverVoltStatus == 4)))
        {
            ucOverVoltStatus = 1;
        }

        if ((uiMdlStatus & dDCDC_OV) && (ucOverVoltCount >= 3)) {
            uiMdlStatus = uiMdlStatus | dDCDC_HVSD;         //连续软件过压3次，也锁死
        }

        if (uiMdlStatus & 0x8093)
        {
            //  EPwm1Regs.AQCSFRC.all = 0x5;        //PWM1A,PWM1B force low
            //  EPwm2Regs.AQCSFRC.all = 0x5;        //PWM2A,PWM2B force low
            //    EPwm3Regs.AQCSFRC.all = 0x5;      //PWM1A,PWM1B force low
            //  EPwm4Regs.AQCSFRC.all = 0x5;        //PWM2A,PWM2B force low
            uiMdlStatus = uiMdlStatus | dMDL_OFF;
            uiActionReady = 0;
            ubitTestFlag.lData |= 0x00040000;

            if (uiMdlStatus & 0x8090) {
                ucOffFlag = ucOffFlag | 0x04;
            }
            else if (uiMdlStatus & 0x0001) {
                ucOffFlag = ucOffFlag | 0x11;
            }

            uiPfcCtrl = uiPfcCtrl & 0xFFDF;
        }
        else
        {
            if (ucOverVoltStatus == 1)
            {   //
                ucOverVoltStatus = 2;
                ucOverVoltCount++;
                uiOpenTime = 0;
            }

            if(ucOffFlag & 0x04)
            {
                ucOffFlag = ucOffFlag & 0xfb;
                uiActionReady = 0;
                ubitTestFlag.lData |= 0x00080000;
            }

            if(ucOffFlag & 0x11)
            {
                if (ucOverVoltStatus == 3)
                {
                    ucOffFlag = ucOffFlag & 0xee;
                    ucOverVoltStatus = 4;
                    uiOpenTime = 0;
                    uiActionReady = 0;
                    ubitTestFlag.lData |= 0x00100000;
                }
            }

            if(ucOffFlag == 0x20)
            {
                ucOffFlag = ucOffFlag & 0xdf;
            }
        }

        if((uiVoltAca < 260 * 32) || (uiVoltAca > 535 * 32) ||
                (uiVoltAcb < 260 * 32) || (uiVoltAcb > 535 * 32) ||
                (uiVoltAcc < 260 * 32) || (uiVoltAcc > 535 * 32))
        {
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
            if(((uiMdlStatus & dSCI_FAIL) == 0) && (leddelay >= 1400))
            {
                GpioDataRegs.GPASET.bit.GPIO19 = 1;
            }
        }
        else
        {
            GpioDataRegs.GPASET.bit.GPIO9 = 1;
            uiMdlStatus = uiMdlStatus & 0xbfff;
        }

        if((uiMdlStatus & 0x218A) || (ucCurrUnBal >= 300))
        {
            if (ui60STimer > 2000) {
                GpioDataRegs.GPASET.bit.GPIO8 = 1;
            }
        }
        else if(uiMdlStatus & 0x0400)
        {
            if(ucLightFlag) {
                GpioDataRegs.GPATOGGLE.bit.GPIO8 = 1;
            }
        }
        else
        {
            GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;
        }

        if(ucCurrSerUnBal)
        {
            ubitTestFlag.lData |= 0x00000001;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000001;
        }

        if(ucIDError)
        {
            ubitTestFlag.lData |= 0x00000002;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000002;
        }

        if(uiMdlStatus & 0x8000)
        {
            ubitTestFlag.lData |= 0x00000004;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000004;
        }

        if(uiMdlStatus & 0x0020)
        {
            ubitTestFlag.lData |= 0x00000008;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000008;
        }

        if(uiMdlStatus & 0x0001)
        {
            ubitTestFlag.lData |= 0x00000010;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000010;
        }

        //
        if(uiPfcStatus & 0x0010)
        {
            ubitTestFlag.lData |= 0x00000020;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000020;
        }

        //
        if(uiMdlStatus & 0x2000)
        {
            ubitTestFlag.lData |= 0x00000040;
            Dcdcisr.test = 300;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000040;
        }

        if(uiMdlStatus & 0x0100)
        {
            ubitTestFlag.lData |= 0x00000080;
            Dcdcisr.test = 400;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000080;
        }

        if(uiMdlStatus & 0x0080)
        {
            ubitTestFlag.lData |= 0x00000100;
            Dcdcisr.test = 500;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000100;
        }

        if(uiMdlStatus & 0x0002)
        {
            ubitTestFlag.lData |= 0x00000200;
            Dcdcisr.test = 600;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000200;
        }

        if(ucCurrUnBal >= 300)
        {
            ubitTestFlag.lData |= 0x00000400;
            Dcdcisr.test = 700;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000400;
        }

        //
        if(uiMdlStatus & 0x0400)
        {
            ubitTestFlag.lData |= 0x00000800;
        }
        else
        {
            ubitTestFlag.lData &= ~0x00000800;
        }
        //
        if(uiPfcStatus & 0x8000)
        {
            ubitTestFlag.lData |= 0x00001000;
        }
        if(uiPfcStatus & 0x4000)
        {
            ubitTestFlag.lData |= 0x00002000;
        }
        if(uiPfcStatus & 0x2000)
        {
            ubitTestFlag.lData |= 0x00004000;
        }
        if(uiPfcStatus & 0x1000)
        {
            ubitTestFlag.lData |= 0x00008000;
        }
        if(uiPfcStatus & 0x0800)
        {
            ubitTestFlag.lData |= 0x00010000;
        }

        if((uiMdlStatus & 0x8021) || (ucCurrSerUnBal >= 300) || ucIDError || (uiMdlStatus & 0x0800))
        {
            GpioDataRegs.GPASET.bit.GPIO19 = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
        }
        else if(uiPfcStatus & 0x0010)
        {
            GpioDataRegs.GPASET.bit.GPIO19 = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
        }
        else
        {
            GpioDataRegs.GPACLEAR.bit.GPIO19 = 1;
        }

        if (uiSciComFail == 0)
        {
            if (ucLightFlag) {
                GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;
            }
        }
        else {
            GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
        }

        if (ucLightFlag) {
            GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
        }

        //
        if ((labs(iq10MdlAvgCurr.lData - iq10MdlCurr.lData) > 3072) || (uiMdlStatus & 0x0228)) {
            uiMdlCurr = (unsigned int)(iq10MdlCurr.lData >> 8);
        }
        else {
            uiMdlCurr = (unsigned int)(iq10MdlAvgCurr.lData >> 8);
        }

        if (uiMdlCurr > uiMdlCurrOld) {
            uiMdlCurrOld = uiMdlCurr;
        }
        else if ((uiMdlCurrOld - uiMdlCurr) >= 4) {
            uiMdlCurrOld = uiMdlCurr;
        }

        ucLightFlag = 0;
    }
}

void  vAdcTreatment(void)
{
    ubitfloat iq10temp, iq12temp;
    if (ucAdcSampFlag)
    {
        ucAdcSampFlag = 0;
        uiAdcData0[40] = Dcdcisr.ui_Dcdc_Curr_Dis0;
        //
        uiAdcData1[40] = (unsigned int)(Dcdcisr.ul_Dcdc_Volt_Ave >> 2);
        uiAdcData2[40] = AdcMirror.ADCRESULT15;
        uiAdcData3[40] = AdcMirror.ADCRESULT14;

        lAdcSum0 += (long)uiAdcData0[40] - (long)uiAdcData0[uiAdcPoint];
        lAdcSum1 += (long)uiAdcData1[40] - (long)uiAdcData1[uiAdcPoint];
        lAdcSum2 += (long)uiAdcData2[40] - (long)uiAdcData2[uiAdcPoint];
        lAdcSum3 += (long)uiAdcData3[40] - (long)uiAdcData3[uiAdcPoint];

        uiAdcData0[uiAdcPoint] = uiAdcData0[40];
        uiAdcData1[uiAdcPoint] = uiAdcData1[40];
        uiAdcData2[uiAdcPoint] = uiAdcData2[40];
        uiAdcData3[uiAdcPoint] = uiAdcData3[40];

        uiAdcPoint++;
        if( uiAdcPoint >= 40 ) {
            uiAdcPoint = 0;
        }

        //
        //
        iq12temp.lData = _IQ12mpy((lAdcSum1 << 1), iq12_VOLTFACTOR);
        iq12temp.lData = _IQ12mpy(iq12temp.lData, iq12VoltSampSysa.lData) + iq12VoltSampSysb.lData;

        //2018-10-26
        iq12temp.lData -= (iq10VrefGainCurr.lData <<2);         //需要将输出补偿值减去
        //2017-10-26 end

        if (labs(iq12temp.lData - (iq10MdlVolt.lData << 2)) < 615) {
            iq12temp.lData = _IQ12mpy((iq10MdlVolt.lData << 2), 3891) + _IQ12mpy(iq12temp.lData, 205);
        }

        if (iq12temp.lData < 400)
        {
            iq10MdlVolt.lData = 400;
        }
        else if (iq12temp.lData > (long)770 << 12)
        {
            iq10MdlVolt.lData = (long)770 << 10;
        }
        else
        {
            iq10MdlVolt.lData = iq12temp.lData >> 2;
        }

        //
        //
        iq10temp.lData = _IQ10mpy(lAdcSum0, iq10_CURRFACTOR);
        iq10temp.lData = _IQ10mpy(iq10temp.lData, iq10CurrSampSysa.lData) + iq10CurrSampSysb.lData;

        if (labs(iq10temp.lData - iq10MdlCurr.lData) < 820) {
            iq10temp.lData = _IQ10mpy(iq10MdlCurr.lData, 972) + _IQ10mpy(iq10temp.lData, 52);
        }

        if (iq10temp.lData < 50)
        {
            iq10MdlCurr.lData = 0;
        }
        else if (iq10temp.lData > (long)50 << 10) //2017 20k
        {
            iq10MdlCurr.lData = (long)50 << 10; //2017 20k
        }
        else
        {
            iq10MdlCurr.lData = iq10temp.lData;
        }

        if(uiMdlStatus & dMDL_OFF)
        {
            iq10MdlCurr.lData = 0;
            iq10CurrDisp.lData = 0;
        }
        else
        {
            if((iq10MdlAvgCurr.lData >= 1536) && (Dcdcisr.ui_Dcdc_Run_Mode == 0) && (uiActionReady >= 3) && (labs(iq10MdlAvgCurr.lData - iq10MdlCurr.lData) < 2048)) {
                iq10CurrDisp.lData = iq10MdlAvgCurr.lData + ((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) >> 2);
            }
            else {
                iq10CurrDisp.lData = iq10MdlCurr.lData;
            }

        }

        iq10temp.lData = _IQ10mpy(lAdcSum2, iq10_TEMPFACTOR) - ((long)50 << 10);

        if (labs(iq10temp.lData - iq10TempCtrl.lData) > 1024)
        {
            if (iq10temp.lData >= (long)125 << 10)
            {
                iq10TempCtrl.lData = (long)125 << 10;
            }
            else if (iq10temp.lData <= (long) -40 << 10)
            {
                iq10TempCtrl.lData = (long) -40 << 10;
            }
            else {
                iq10TempCtrl.lData = iq10temp.lData;
            }
        }


        //
        Dcdcisr.ui_Dcdc_Temp_Ctrl = (unsigned int)((iq10TempCtrl.lData + ((long)50 << 10)) >> 10);
        //
        //
        iq10temp.lData = _IQ10mpy(lAdcSum3, iq10_TEMPFACTOR_Inside); //
        if (labs(iq10temp.lData - iq10TempDc.lData) > 1024)
        {
            if (iq10temp.lData >= (long)150 << 10)
            {
                iq10TempDc.lData = (long)150 << 10;
            }
            else if (iq10temp.lData <= (long)0 << 10)
            {
                iq10TempDc.lData = (long)0 << 10;
            }
            else {
                iq10TempDc.lData = iq10temp.lData;
            }
        }

        if(iq10TempDc.lData >= (long)60 << 10)
        {
            iq10TempDc.lData = iq10TempDc.lData - ((iq10TempDc.lData - ((long)60 << 10)) >> 2);
        }

        iq10AcVab.lData = (unsigned long)uiVoltAca << 5;
        iq10AcVbc.lData = (unsigned long)uiVoltAcb << 5;
        iq10AcVca.lData = (unsigned long)uiVoltAcc << 5;

        iq10Vaca.lData = _IQ10mpy(iq10AcVab.lData, 591);
        iq10Vacb.lData = _IQ10mpy(iq10AcVbc.lData, 591);
        iq10Vacc.lData = _IQ10mpy(iq10AcVca.lData, 591);
    }
}
/*************************************************/
/*                            */
/*************************************************/
void vCircuitCal(void)
{
    signed int iDeltaTmp;
    unsigned int uiRefTmp, uiAcCurr;
    ubitfloat iq10LimitData, iq10LimitData1, iq10LimitTmp, iq10LimitCal, iq10Limit_Voltage;
    ubitfloat iq10TempLimit, iq10TempUse, iq10TempCurrLimit;
    signed long lreftmp;
    static unsigned int uiSetVoltRefRaw = 0;                //原始输出电压参考，不带补偿 2018-10-31

    if(uiActionReady >= 3)                                  //WJP20150616
    {
        if(iq10SetVoltTrue.lData < (iq10SetVolt.lData - 1))
        {
            if(iq10SetVoltTrue.lData < (iq10SetVolt.lData - 5))
            {
                iq10SetVoltTrue.lData = iq10SetVoltTrue.lData + 5;
            }
            else
            {
                iq10SetVoltTrue.lData = iq10SetVoltTrue.lData + 1;
            }
        }
        else if(iq10SetVoltTrue.lData > (iq10SetVolt.lData + 5))
        {
            iq10SetVoltTrue.lData = iq10SetVoltTrue.lData - 5;
        }
        else
        {
            iq10SetVoltTrue.lData = iq10SetVolt.lData;
        }

        //PFC电压计算
//2019-01-24 pfc电压计算
        if(0x01 == Dcdcisr.currlimit_flag)                  //在限流态
        {
            lreftmp = iq10MdlVolt.lData;
        }
        else                                                //在恒压态
        {
            lreftmp = iq10SetVolt.lData;
        }
        if(lreftmp <= (1024l * 520))
        {
            uiVpfcSet = dPFC_SET_MIN;
        }
        else if(lreftmp >= (1024l * 732))
        {
            uiVpfcSet = dPFC_SET_MAX;
        }
        else
        {
            lreftmp -= (1024l * 520);
            uiVpfcSet = dPFC_SET_MIN + (_IQ10mpy(lreftmp, (dPFC_SET_MAX - dPFC_SET_MIN)) / (732 - 520));
        }
        if(uiVpfcSet > dPFC_SET_MAX)
        {
            uiVpfcSet = dPFC_SET_MAX;
        }
        else if(uiVpfcSet < dPFC_SET_MIN)
        {
            uiVpfcSet = dPFC_SET_MIN;
        }
//2019-01-24 end
        /*if(Dcdcisr.currlimit_flag == 0x01)                  //在限流时，根据输出电压实际计算PFC电压
        {
            if(iq10MdlVolt.lData <= ((long)520 << 10))
            {
                uiVpfcSet = 330;
            }
            else if(iq10MdlVolt.lData >= (long)732 << 10)
            {
                uiVpfcSet = 420;
            }
            else
            {
                lreftmp = iq10MdlVolt.lData - ((long)520 << 10);
                temp = (int)(lreftmp * 27 >> 16);
                uiVpfcSet = 330 + temp;
            }
        }
        else                                                //非限流时，根据输出电压设置值计算PFC电压
        {
            if(uiSetVoltRefRaw >= il732ref) //2017 20k 28994
            {
                uiVpfcSet = 420;
            }
            else if(uiSetVoltRefRaw <= il520ref) //20597
            {
                uiVpfcSet = 330;
            }
            else
            {
                temp = (int)uiSetVoltRefRaw - (int)il520ref;
                temp1 = _IQ12mpy(VASAMPLE_INV, 681);
                lreftmp = (long)temp * temp1;
                temp = (unsigned int)(lreftmp >> 16);
                uiVpfcSet = 330 + temp;
            }
        }
        if(uiVpfcSet >= 420)
        {
            uiVpfcSet = 420;
        }
        else if(uiVpfcSet <= 330)
        {
            uiVpfcSet = 330;
        }*/
    }
    
    //电压参考计算 2018-10-31
    lreftmp = _IQ12mpy((iq10SetVoltTrue.lData << 2), iq12VoltConSysa.lData) + iq12VoltConSysb.lData;
    uiSetVoltRefRaw = _IQ12mpy(lreftmp, iq12_VFACTOR) >> 12;
        //补偿值
    iDeltaTmp = (int)(iCurrDelta + (int)ucVoltAdjDelta - 128);
    lreftmp = _IQ12mpy((iq10VrefGainCurr.lData << 2), iq12VoltConSysa.lData);
    lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
    lreftmp = (lreftmp  + ((long)iDeltaTmp * 1628)) >> 12;
        //实际参考
    lreftmp += uiSetVoltRefRaw;             
    if(lreftmp > 32400)
    {
        uiDCVoltref = 32400;
    }
    else
    {
        uiDCVoltref = lreftmp;
    }

    //定时调整
    if (ucCircuitCalFlag)                       //间隔120ms调整PFC电压
    {
        ucCircuitCalFlag = 0;
        if(uiVpfcSet_true <= uiVpfcSet)         //电压升高立刻执行
        {
            uiVpfcSet_true = uiVpfcSet;
        }
        else if((uiVpfcSet_true - 5) >= uiVpfcSet)
        {
            uiVpfcSet_true = uiVpfcSet_true - 5;
        }
        else
        {
            uiVpfcSet_true = uiVpfcSet;
        }

        if (labs(iq10SetLimitTrue.lData - iq10SetLimit.lData ) >= 410) //2017
        {
            iq10SetLimitTrue.lData = iq10SetLimitTrue.lData + ((iq10SetLimit.lData - iq10SetLimitTrue.lData ) >> 3);
        }
        else if(labs(iq10SetLimitTrue.lData - iq10SetLimit.lData ) > 100)
        {
            iq10SetLimitTrue.lData = iq10SetLimitTrue.lData + ((iq10SetLimit.lData - iq10SetLimitTrue.lData ) >> 2);
        }
        else
        {
            iq10SetLimitTrue.lData = iq10SetLimit.lData;
        }
        if(labs(iq10SetPowerTrue.lData - iq10SetPower.lData) >= 410)
        {
            iq10SetPowerTrue.lData = iq10SetPowerTrue.lData + ((iq10SetPower.lData - iq10SetPowerTrue.lData ) >> 3);
        }

        else if (labs(iq10SetPowerTrue.lData - iq10SetPower.lData) > 100)
        {
            iq10SetPowerTrue.lData = iq10SetPowerTrue.lData + ((iq10SetPower.lData - iq10SetPowerTrue.lData ) >> 2);
        }
        else
        {
            iq10SetPowerTrue.lData = iq10SetPower.lData;
        }


        if ((ucPermitAdjust == 0) || (uiActionReady < 3))
        {
            lreftmp = _IQ12mpy((long)2998272, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //2017 20k
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il732ref = (unsigned int)(lreftmp >> 12); //2017 20k

            lreftmp = _IQ12mpy((long)2486272, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il607ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)2252800, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il550ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)2129920, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //594v
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il520ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1241088, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il303ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1269760, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il310ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1433600, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il350ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1474560, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il360ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1720320, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il420ref = (unsigned int)(lreftmp >> 12);

            lreftmp = _IQ12mpy((long)1761280, iq12VoltConSysa.lData) + iq12VoltConSysb.lData; //
            lreftmp = _IQ12mpy(lreftmp, iq12_VFACTOR);
            il430ref = (unsigned int)(lreftmp >> 12);
        }

        //限功率计算
        ucLimitStatus = ucLimitStatus | 0x06;
        iq10LimitData.lData = iq10SetPowerTrue.lData;

        //根据环温限功率
        iq10TempUse.lData = iq10TempCtrl.lData;
        if (iq10TempUse.lData >= dQ10_AMB_LIMIT_MAX)                    //根据面板温度降额
        {
            iq10TempCurrLimit.lData = dQ10_CURR_LIMIT_MIN;
            iq10TempLimit.lData = dQ10_POWER_LIMIT_MIN;
        }
        else if(iq10TempUse.lData > dQ10_AMB_LIMIT_MIN)
        {
            //fLimitTmp = 1.05-0.05666*(fCtrlTempOld-50.0);
            iq10TempCurrLimit.lData = _IQ10mpy(1109, (iq10TempUse.lData - dQ10_AMB_LIMIT_MIN)) >> 3;
            iq10TempCurrLimit.lData = dQ10_CURR_LIMIT_MAX - iq10TempCurrLimit.lData;

            iq10TempLimit.lData  = _IQ10mpy(741, (iq10TempUse.lData - dQ10_AMB_LIMIT_MIN)) >> 4;
            iq10TempLimit.lData  = dQ10_POWER_LIMIT_MAX - iq10TempLimit.lData;
        }
        else
        {
            iq10TempCurrLimit.lData = dQ10_CURR_LIMIT_MAX;
            iq10TempLimit.lData = dQ10_POWER_LIMIT_MAX;
            ucLimitStatus = ucLimitStatus & 0xFD;
        }
        //
        //
        if(uiVoltAca <= uiAcaVoltOld) {
            uiAcaVoltOld = uiVoltAca;
        }
        else if(uiVoltAca > (uiAcaVoltOld + 320)) {
            uiAcaVoltOld = uiVoltAca;
        }

        if(uiVoltAcb <= uiAcbVoltOld) {
            uiAcbVoltOld = uiVoltAcb;
        }
        else if(uiVoltAcb > (uiAcbVoltOld + 320)) {
            uiAcbVoltOld = uiVoltAcb;
        }

        if(uiVoltAcc <= uiAccVoltOld) {
            uiAccVoltOld = uiVoltAcc;
        }
        else if(uiVoltAcc > (uiAccVoltOld + 320)) {
            uiAccVoltOld = uiVoltAcc;
        }

        //根据输入状态限功率
        if ((uiPfcStatus & 0x0180) || (uiMdlStatus & 0x2000))
        {
            ucLimitStatus = ucLimitStatus & 0xFB;
        }

        if ((uiAcaVoltOld > POWER_ALL) && (uiAcbVoltOld > POWER_ALL) && (uiAccVoltOld > POWER_ALL))
        {
            iq10LimitTmp.lData = 1024;
            ucLimitStatus = ucLimitStatus & 0xFB;
        }
        else
        {
            if (Dcdcisr.ui_Dcdc_debug)
            {
                iq10LimitTmp.lData = 1024;            //
            }
            else
            {
                iq10LimitTmp.lData = 512;
            }
        }

        iq10LimitTmp.lData = _IQ10mpy(iq10TempLimit.lData, iq10LimitTmp.lData); //

        if(iq10LimitData.lData > iq10LimitTmp.lData) { //setpowerlim>PlimTmp*AClim
            iq10LimitData.lData = iq10LimitTmp.lData;
        }

        //输入电压限功率
        if (uiAcaVoltOld < uiAcbVoltOld) {
            uiRefTmp = uiAcaVoltOld;
        }
        else {
            uiRefTmp = uiAcbVoltOld;
        }
        if (uiRefTmp > uiAccVoltOld) {
            uiRefTmp = uiAccVoltOld;
        }
        if (uiSciComFail) {
            uiRefTmp = 380 << 5;
        }
        uiAcCurr = (unsigned int)(iq10AcCurrFt.lData >> 10);
        uiAcCurr = 13;
        iq10LimitTmp.lData = ((long)(uiRefTmp >> 5) * uiEfficfactor[uiAcCurr]) >> 10; //
        iq10LimitTmp.lData = 1024;                      //给定最大功率
        if(iq10LimitData.lData > iq10LimitTmp.lData)
        {
            iq10LimitData.lData = iq10LimitTmp.lData;
        }

        //功率限流点计算(额定功率/输出电压)
        iq10LimitData1.lData = _IQ10div(_IQ10mpy(iq10LimitData.lData, dQ10_CURR_LIMIT_VREF), iq10MdlVolt.lData);
        if(iq10LimitData1.lData > dQ10_CURR_LIMIT_MAX)
        {
            iq10LimitData1.lData = dQ10_CURR_LIMIT_MAX;
        }
        /*if(iq10MdlVolt.lData >= (long)250 << 10)
        {
            iq10LimitData1.lData = _IQ10div(_IQ10mpy(iq10LimitData.lData, dQ10_CURR_LIMIT_VREF), iq10MdlVolt.lData);
            if(iq10LimitData1.lData >= dQ10_CURR_LIMIT_MAX)
            {
                iq10LimitData1.lData = dQ10_CURR_LIMIT_MAX;
            }
        }
        else
        {
            iq10LimitData1.lData = dQ10_CURR_LIMIT_MAX;
        }*/
        if(iq10LimitData1.lData >= iq10TempCurrLimit.lData)
        {
            iq10LimitData1.lData = iq10TempCurrLimit.lData;
        }

        iq10SetPowerWant.lData = iq10LimitData.lData;
        //2018-11-16
        lreftmp = iq10MdlCurr.lData;
        if(lreftmp > dQ10_POWER_GAIN_CURR_MIN)
        {
            if(lreftmp > dQ10_CURR_MAX)
            {
                lreftmp = dQ10_CURR_MAX;
            }
            lreftmp = (dQ10_CURR_MAX + 2048) - lreftmp;
            lreftmp = _IQ10div(_IQ10mpy(lreftmp, dQ10_POWER_GAIN), (dQ10_CURR_MAX - dQ10_POWER_GAIN_CURR_MIN));
        }
        else
        {
            lreftmp = dQ10_POWER_GAIN;
        }
        /*if(lreftmp > (35l * 1024))
        {
            if(lreftmp > (45l * 1024))
            {
                lreftmp = (45l * 1024);
            }
            lreftmp = (47l * 1024) - lreftmp;
            lreftmp = _IQ10div(_IQ10mpy(lreftmp, dQ10_POWER_GAIN), 10240);
        }
        else
        {
            lreftmp = dQ10_POWER_GAIN;
        }*/
      
        lreftmp = _IQ10mpy(lPowerMax, iq10SetPowerWant.lData) + (lreftmp * ucPermitAdjust);
        //2018-11-16 end
        //lreftmp = _IQ10mpy(lPowerMax, iq10SetPowerWant.lData) + (dQ10_POWER_GAIN * ucPermitAdjust);   //2018-11-16
        lreftmp = _IQ10mpy(lreftmp, iq10PowerConSysa.lData) + iq10PowerConSysb.lData;
        lreftmp = _IQ10mpy(lreftmp, (long)656);
        lreftmp = _IQ10mpy(lreftmp, (long)iq10_VFACTOR);
        if (lreftmp >= 0x3C87AE2A)
        {
            Dcdcisr.ul_Dcdc_Power_Lim = 0x3C87AE2A;
        }
        else if (lreftmp < 0x1F71B43)
        {
            Dcdcisr.ul_Dcdc_Power_Lim = 0x1F71B43;
        }
        else
        {
            Dcdcisr.ul_Dcdc_Power_Lim = (unsigned long)lreftmp;
        }
        if (uiActionReady >= 3)
        {
            iq10LimitCal.lData = iq10SetLimitTrue.lData;

            if (iq10LimitData1.lData < iq10SetLimitTrue.lData)
            {
                iq10MdlLimit.lData = iq10LimitData1.lData;
            }
            else
            {
                iq10MdlLimit.lData = iq10SetLimitTrue.lData;
            }
        }
        else
        {
            iq10LimitCal.lData = iq10SoftLimit.lData;
            iq10MdlLimit.lData = iq10SoftLimit.lData;
            if (uiSciComFail)
            {
                iq10MdlLimit.lData = 0;
            }
        }

        //330V以下的限流点
        iq10Limit_Voltage.lData = dQ10_CURR_LIMIT_MAX;              //默认最大
        if((uiActionReady >= 3) && (VOLTcal_delay == 1000))
        {
            if(iq10MdlVolt.lData > dQ10_LOW_VOLT_TOP)               //超过低压区间则按照最大限流点
            {
                iq10Limit_Voltage.lData = dQ10_CURR_LIMIT_MAX;
            }
            else if(iq10MdlVolt.lData < dQ10_LOW_VOLT_BOTTOM)       //低于低压区间则按照最小限流点
            {
                iq10Limit_Voltage.lData = dQ10_CURR_LIMIT_VOLT_MIN;
            }
            else                                                    //否则在区间中调整
            {
                lreftmp = iq10MdlVolt.lData - dQ10_LOW_VOLT_BOTTOM;
                iq10Limit_Voltage.lData = dQ10_CURR_LIMIT_VOLT_MIN;
                iq10Limit_Voltage.lData += _IQ10mpy(lreftmp, dQ10_CURR_LIMIT_VOLT_STEP) >> 10;
            }
        }
        if(iq10LimitCal.lData >= iq10Limit_Voltage.lData)
        {
            iq10LimitCal.lData = iq10Limit_Voltage.lData;
        }

        if(iq10LimitCal.lData >= iq10LimitData1.lData)
        {
            iq10LimitCal.lData = iq10LimitData1.lData;
        }
        //更新限流点
        if (uiActionReady >= 3)
        {
            iq10MdlLimit.lData = iq10LimitCal.lData;
        }
        
        //计算电流参考
        lreftmp = _IQ10mpy(lCurrRate, iq10LimitCal.lData);
        lreftmp = _IQ10mpy(lreftmp, iq10CurrConSysa.lData) + iq10CurrConSysb.lData;
        lreftmp = _IQ10mpy(lreftmp, iq10_IFACTOR);

        if (lreftmp >= (long)31800 << 10) {
            Dcdcisr.ui_Dcdc_Curr_Ref = 31800;
        }
        else if (lreftmp < (long)100 << 10) {
            Dcdcisr.ui_Dcdc_Curr_Ref = 100;
        }
        else {
            Dcdcisr.ui_Dcdc_Curr_Ref = (unsigned int)(lreftmp >> 10);
        }


        if((ucLimitStatus & 0x06) && ((uiMdlStatus & 0x0008) == 0))
        {
            iq10TempLimit.lData = _IQ10mpy(iq10MdlCurr.lData, iq10MdlVolt.lData);
            if (((iq10TempLimit.lData + ((long)80 << 10)) >= _IQ10mpy(iq10SetPowerWant.lData, lPowerMax))
                    && Dcdcisr.ui_Dcdc_Run_Mode)
            {
                ucLimitStatus = ucLimitStatus | 0x01;
            }
            else if(((iq10TempLimit.lData + ((long)100 << 10)) <= _IQ10mpy(iq10SetPowerWant.lData, lPowerMax))
                    && (Dcdcisr.ui_Dcdc_Run_Mode == 0))
            {
                ucLimitStatus = ucLimitStatus & 0xfe;
            }
        }
        else {
            ucLimitStatus = ucLimitStatus & 0xfe;
        }


    }
}

/************************************************/
/*                                       */
/************************************************/
void    vAvgCurrCal(void)
{
    long lreftmp;
    lreftmp = _IQ10div((iq10MdlAvgCurr.lData - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    AvgCurr = (signed int)(lreftmp >> 10);

    if ((uiCurrCtrlFlag) && ((uiMdlStatus & dMDL_OFF) == 0) && (ucCurrSerUnBal == 0) && (uiActionReady >= 3))
    {
        if (uiCurrCtrlFlag == 0x0300)
        {
            iCurrDelta = iCurrDelta - 4;
        }
        else if (uiCurrCtrlFlag == 0x0200)
        {
            iCurrDelta = iCurrDelta + 4;
        }
        else if((uiCurrCtrlFlag == 0x0100) && ((Dcdcisr.ui_Dcdc_Run_Mode == 0) || (Dcdcisr.ui_Dcdc_Run_Mode && ((ucLimitStatus & 0x01) == 0))))
        {
            if (labs(iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >= 768)
            {
                ucLoadShareFlag = 0;

                if((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) > cOpCurr3Amp)
                {
                    iCurrDelta = iCurrDelta + ((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >> 9);
                }
                else if((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) > cOpCurr3Amp)
                {
                    iCurrDelta = iCurrDelta - ((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) >> 9);
                }
                else if((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) > cOpCurr1Amp)
                {
                    iCurrDelta = iCurrDelta + ((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >> 10);
                }
                else if((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) > cOpCurr1Amp)
                {
                    iCurrDelta = iCurrDelta - ((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) >> 10);
                }
                else if((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >= 768) //
                {
                    iCurrDelta += 1;
                }
                else if((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) >= 768) //
                {
                    iCurrDelta -= 1;
                }
            }
            else
            {
                if (ucLoadShareFlag == 0)
                {
                    if((iq10MdlAvgCurr.lData - iq10MdlCurr.lData) >= 512) {
                        iCurrDelta = iCurrDelta + 1;
                    }
                    else if((iq10MdlCurr.lData - iq10MdlAvgCurr.lData) >= 512) {
                        iCurrDelta = iCurrDelta - 1;
                    }
                    else {
                        ucLoadShareFlag = 1;
                    }
                }
            }
///////////////////////////////////////////////////////////////////////////
            if ((iq10MdlAvgCurr.lData <= 512) && ((iCurrDelta >= cCurrDeltaMax) || (iCurrDelta <= (-cCurrDeltaMax))))
            {
                iCurrDelta = 0;
            }
        }

        //
        if(iCurrDelta > cCurrDeltaMax) {
            iCurrDelta = cCurrDeltaMax;
        }
        else if(iCurrDelta < (-cCurrDeltaMax)) {
            iCurrDelta = -cCurrDeltaMax;
        }

        uiCurrCtrlFlag = 0;
    }
}

/************************************************/
/*                        */
/************************************************/

void  vAddressCal(void)
{
    adresstemp2 = adresstemp1;
    adresstemp1 = 0;
    if(AdcMirror.ADCRESULT0 < 682)
    {
        adresstemp1 = adresstemp1 | 0x01;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0FE;
    }
    if(AdcMirror.ADCRESULT3 < 682)
    {
        adresstemp1 = adresstemp1 | 0x02;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0FD;
    }
    if(AdcMirror.ADCRESULT13 < 682)
    {
        adresstemp1 = adresstemp1 | 0x04;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0FB;
    }
    if(AdcMirror.ADCRESULT4 < 682)
    {
        adresstemp1 = adresstemp1 | 0x08;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0F7;
    }

    if(AdcMirror.ADCRESULT7 < 682)
    {
        adresstemp1 = adresstemp1 | 0x010;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0EF;
    }

    if(GpioDataRegs.GPADAT.bit.GPIO27 == 0)
    {
        adresstemp1 = adresstemp1 | 0x020;
    }
    else
    {
        adresstemp1 = adresstemp1 & 0x0DF;
    }

    if((adresstemp1 == adresstemp2) && (adresstemp1 != uiMdlAddr))
    {
        uiMdlAddr = adresstemp1 & 0x00ff;
        ECana32Temp.CANME.all = 0x00000033;
        ECana32Temp.CANME.bit.ME4 = 0;
        ECana32Temp.CANME.bit.ME5 = 0;
        ECanaRegs.CANME.all = ECana32Temp.CANME.all;

        ECanaMboxes.MBOX4.MSGID.bit.SRCADDR = uiMdlAddr;
        ECanaMboxes.MBOX5.MSGID.bit.SRCADDR = uiMdlAddr;

        ECana32Temp.CANME.all = 0x00000003;
        ECana32Temp.CANME.bit.ME4 = 1;
        ECana32Temp.CANME.bit.ME5 = 1;
        ECanaRegs.CANME.all = ECana32Temp.CANME.all;
    }
}

/************************************************/
/**/
/**/
/**/
/**/
/**/
/**/
/**/
/************************************************/
void uSecond(unsigned int uS)
{
    unsigned int i, j;

    for (i = 0; i < uS; i++)
    {
        for (j = 0; j < 20; j++);   //
        {
            asm (" NOP ");
        }
        ServiceDogPointer();
    }
}

/************************************************/
/************************************************/
Uint16 uiI2caWriteData(unsigned int uAddress, ubitfloat fTemp)
{
    uiCount1 = 0;
    while (I2caRegs.I2CMDR.bit.STP == 1)
    {
        uiCount1 ++ ;
        if(uiCount1 > 6000)
        {
            uiI2CErrorType = I2C_ERROR;
            return I2C_STP_NOT_READY_ERROR;
        }
    }

    //
    I2caRegs.I2CSAR = 0x50;

    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    I2caRegs.I2CCNT = 6;

    //
    I2caRegs.I2CDXR = (unsigned char)(uAddress >> 8);
    I2caRegs.I2CDXR = (unsigned char)(uAddress & 0x00ff);

    I2caRegs.I2CDXR = fTemp.intdata[0].bitdata.highchar;
    I2caRegs.I2CDXR = fTemp.intdata[0].bitdata.lowchar;
    I2caRegs.I2CDXR = fTemp.intdata[1].bitdata.highchar;
    I2caRegs.I2CDXR = fTemp.intdata[1].bitdata.lowchar;

    //
    I2caRegs.I2CMDR.all = 0x6E20;

    return I2C_SUCCESS;
}

/************************************************/
/************************************************/
Uint16 uiI2caReadData(unsigned int uAddress)
{
    Uint16  i;

    uiCount2 = 0;
    while (I2caRegs.I2CMDR.bit.STP == 1)
    {
        uiCount2 ++ ;
        if(uiCount2 > 6000)
        {
            uiI2CErrorType = I2C_ERROR;
            return I2C_STP_NOT_READY_ERROR;
        }
    }

    //
    I2caRegs.I2CSAR = 0x50;

    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    I2caRegs.I2CCNT = 2;
    I2caRegs.I2CDXR = (unsigned char)(uAddress >> 8);
    I2caRegs.I2CDXR = (unsigned char)(uAddress & 0x00ff);
    I2caRegs.I2CMDR.all = 0x6620;
    I2caRegs.I2CSTR.bit.SCD = 1;

    //
    uiCount3 = 0;
    uiCount4 = 0;
    while (I2caRegs.I2CSTR.bit.ARDY != 1)
    {
        uiCount3 ++ ;
        if(uiCount3 > 4000)
        {
            uiI2CErrorType = I2C_ERROR;
            return I2C_ERROR;
        }
    }

    I2caRegs.I2CCNT = 4;
    I2caRegs.I2CMDR.all = 0x6C20;
    while (I2caRegs.I2CSTR.bit.SCD != 1)
    {
        uiCount4 ++ ;
        if(uiCount4 > 6000)//
        {
            uiI2CErrorType = I2C_ERROR;
            return I2C_ERROR;
        }
    }

    for (i = 0; i < 4; i++)
    {
        uiRdMsgBuffer[i] = I2caRegs.I2CDRR;
    }

    fRdTemp.intdata[0].bitdata.highchar = uiRdMsgBuffer[0] & 0x00ff;
    fRdTemp.intdata[0].bitdata.lowchar = uiRdMsgBuffer[1] & 0x00ff;
    fRdTemp.intdata[1].bitdata.highchar = uiRdMsgBuffer[2] & 0x00ff;
    fRdTemp.intdata[1].bitdata.lowchar = uiRdMsgBuffer[3] & 0x00ff;

    return I2C_SUCCESS;
}


/*************************************************/
/*  */
/*************************************************/
unsigned char ucWriteFloatDataThree(unsigned int uAddress, ubitfloat fTemp)
{
    unsigned int uiCodeType, i;
    ubitfloat  ubitfData;

    ubitfData.lData = fTemp.lData;

    GpioDataRegs.GPACLEAR.bit.GPIO26 = 1;

    ServiceDogPointer();
    uiCodeType = I2C_SUCCESS;

    for (i = 0; i < 3; i++)
    {
        uiCodeType = uiCodeType | uiI2caWriteData(uAddress + (i << 8), ubitfData);
        uSecond(1500);
        if (uiI2caReadData(uAddress + (i << 8)) == I2C_SUCCESS)
        {
            if (ubitfData.lData != fRdTemp.lData) {
                uiCodeType = I2C_ERROR;
            }
        }
        else {
            uiCodeType = I2C_ERROR;
        }
    }

    GpioDataRegs.GPASET.bit.GPIO26 = 1;

    if (uiCodeType == I2C_SUCCESS) {
        return    RECOK;
    }
    else {
        return    NOVCOM;
    }

}


/*************************************************/
/**/
/*************************************************/
ubitfloat fReadFloatDataThree(unsigned int uAddress)
{
    ubitfloat ubitfTmp, ftemp1, ftemp2, ftemp3;

    ucReadEepromOk = 0;
    ftemp1.lData = 0x1111;
    ftemp2.lData = 0x2222;
    ftemp3.lData = 0x3333;

    uSecond(1500);
    if (uiI2caReadData(uAddress) == I2C_SUCCESS) {
        ftemp1.lData = fRdTemp.lData;
    }

    if (uiI2caReadData(uAddress + 256) == I2C_SUCCESS) {
        ftemp2.lData = fRdTemp.lData;
    }

    if (uiI2caReadData(uAddress + 512) == I2C_SUCCESS) {
        ftemp3.lData = fRdTemp.lData;
    }

    if ((ftemp1.lData == ftemp2.lData) && (ftemp1.lData == ftemp3.lData))
    {
        return ftemp1;
    }
    else if ((ftemp1.lData == ftemp2.lData) && (ftemp1.lData != ftemp3.lData))
    {
        ubitfTmp.lData = ftemp1.lData;
        if(uiI2caWriteData(uAddress + 512, ubitfTmp) != I2C_SUCCESS) {
            ucReadEepromOk = 1;
        }
        uSecond(1500);
        return ftemp1;
    }
    else if ((ftemp1.lData != ftemp2.lData) && (ftemp2.lData == ftemp3.lData))
    {
        ubitfTmp.lData = ftemp2.lData;
        if(uiI2caWriteData(uAddress, ubitfTmp) != I2C_SUCCESS) {
            ucReadEepromOk = 1;
        }
        uSecond(1500);
        return ftemp2;
    }
    else if ((ftemp1.lData == ftemp3.lData) && (ftemp1.lData != ftemp2.lData))
    {
        ubitfTmp.lData = ftemp1.lData;
        if(uiI2caWriteData(uAddress + 256, ubitfTmp) != I2C_SUCCESS) {
            ucReadEepromOk = 1;
        }
        uSecond(1500);
        return ftemp1;
    }
    else
    {
        ucReadEepromOk = 1;
        return ftemp1;
    }
}

/************************************************/
/* */
/************************************************/
void vWriteE2PROMSIG(void)
{
    ubitfloat   iq10wrtmp;
    if(uiEepromFlag)
    {
        if(uiEepromFlag & 0x0001)
        {
            vWriteFloatDataSig (116, iq10RunTime);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfffe;
            }
        }
        else if(uiEepromFlag & 0x0002)
        {
            iq10wrtmp.intdata[0].id = uiMdlCtrl & 0x801f;
            iq10wrtmp.intdata[1].id = 0;
            vWriteFloatDataSig (100, iq10wrtmp);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfffd;
            }
        }
        else if(uiEepromFlag & 0x0004)
        {
            InitECan();
            uiEepromFlag = uiEepromFlag & 0xfffb;
        }
        else if(uiEepromFlag & 0x0008)
        {
            vWriteFloatDataSig(88, iq10MdlVoltUp);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfff7;
            }
        }
        else if(uiEepromFlag & 0x0010)
        {
            vWriteFloatDataSig(84, iq10MdlVoltFt);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xffef;
            }
        }
        else if(uiEepromFlag & 0x0020)
        {
            uiEepromFlag &= ~0x0020;
        }
        else if(uiEepromFlag & 0x0040)
        {
            vWriteFloatDataSig(104, iq10ReonTime);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xffbf;
            }
        }
        else if(uiEepromFlag & 0x0080)
        {
            vWriteFloatDataSig(108, iq10LoadSstartTime);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xff7f;
            }
        }
        else if(uiEepromFlag & 0x0100)
        {
            vWriteFloatDataSig(112, iq10OpenTime);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfeff;
            }
        }
        else if(uiEepromFlag & 0x0200)
        {
            vWriteFloatDataSig(72, iq10MdlPowerFt);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfdff;
            }
        }
        else if(uiEepromFlag & 0x0400)
        {
            vWriteFloatDataSig(76, iq10MdlCurrFt);
            if(ucWriteNumber == 0) {
                uiEepromFlag = uiEepromFlag & 0xfbff;
            }
        }
        else if(uiEepromFlag & 0x0800)
        {
            vWriteFloatDataSig(80, iq10AcCurrFt);
            if(ucWriteNumber == 0)
            {
                uiEepromFlag = uiEepromFlag & 0xf7ff;
            }
        }
        //2018-11-02 增加鲁能协议相关
        else if(uiEepromFlag & dEEPROM_ADDR_FLAG)
        {
            iq10wrtmp.lData = uiAddrConst;
            vWriteFloatDataSig(dEEPROM_ADDR_ADDR, iq10wrtmp);
            if(0 == ucWriteNumber)
            {
                uiEepromFlag &= ~dEEPROM_ADDR_FLAG;
            }
        }
        else if(uiEepromFlag & dEEPROM_GROUP_FLAG)
        {
            iq10wrtmp.lData = uiGroupConst;
            vWriteFloatDataSig(dEEPROM_GROUP_ADDR, iq10wrtmp);
            if(0 == ucWriteNumber)
            {
                uiEepromFlag &= ~dEEPROM_GROUP_FLAG;
            }
            
        }
        else if(uiEepromFlag & dEEPROM_BAUD_FLAG)
        {
            iq10wrtmp.lData = uiBaudConst;
            vWriteFloatDataSig(dEEPROM_BAUD_ADDR, iq10wrtmp);
            if(0 == ucWriteNumber)
            {
                uiEepromFlag &= ~dEEPROM_BAUD_FLAG;
            }
        }
        else if(uiEepromFlag & dEEPROM_SHARE_FLAG)
        {
            if(!Flag.bit.share_on)      //禁止均流
            {
                iq10wrtmp.lData = 0;
            }
            else
            {
                iq10wrtmp.lData = 1;
            }
            vWriteFloatDataSig(dEEPROM_SHARE_ADDR, iq10wrtmp);
            if(0 == ucWriteNumber)
            {
                uiEepromFlag &= ~dEEPROM_SHARE_FLAG;
            }
        }
        //2018-11-02 end
    }
}

/*************************************************/
/*  */
/*************************************************/
void    vWriteFloatDataSig(unsigned int uAddress, ubitfloat fTemp)
{
    ubitfloat  ubitfData;

    ubitfData.lData = fTemp.lData;
    ucWriteNumber++;
    GpioDataRegs.GPACLEAR.bit.GPIO26 = 1;

    if ((ucWriteNumber == 1))
    {
        uiI2caWriteData(uAddress, ubitfData);
    }
    else if ((ucWriteNumber == 2))
    {
        uiI2caWriteData(uAddress + 256, ubitfData);
    }
    else if ((ucWriteNumber == 3))
    {
        uiI2caWriteData(uAddress + 512, ubitfData);
    }
    else
    {
        ucWriteNumber = 0;
        GpioDataRegs.GPASET.bit.GPIO26 = 1;
    }
}

/************************************************/
/**/
/************************************************/
void vSendSciData(void)
{
    Uint16 k;
    unsigned int ucIdTemp, temp;
    ubitint  com_int ;
    ubitfloat iq10temp;

    if (ucSciDatFlag)
    {
        if (ucSCISendFlag)
        {
            ucSCISendFlag = 0;

            if (uiPfcSendId) {
                ucIdTemp = uiPfcSendId;
            }
            else {
                ucIdTemp = 1;
            }

            if (ucIdTemp == 1)
            {
                ucTxBuffer[1] = 1;
                ucTxBuffer[2] = 10;
                uiTxLength = ucTxBuffer[2] + 2;

                ucTxBuffer[3] = uiVpfcSet_true / 256;
                ucTxBuffer[4] = uiVpfcSet_true % 256;

                uiPfcCtrl = (uiPfcCtrl & 0x0021) | (uiMdlCtrl & 0x001A);
                uiPfcCtrl = uiPfcCtrl | 0x0040;

                ucTxBuffer[5] = uiPfcCtrl / 256;
                ucTxBuffer[6] = uiPfcCtrl % 256;

                ucTxBuffer[7] = (unsigned char)((iq10TempCtrl.lData + ((long)50 << 10)) >> 10);
                ucTxBuffer[8] = (unsigned char)((iq10TempDc.lData + ((long)50 << 10)) >> 10);

                temp = (unsigned int)(iq10MdlVolt.lData >> 10);
                ucTxBuffer[9] = (unsigned int)(temp / 256);
                ucTxBuffer[10] = (unsigned int)(temp % 256);

                ucTxBuffer[11] = (unsigned char)(iq10MdlCurr.lData >> 10);

                iq10temp.lData = _IQ10mpy(Dcdcisr.ui_Dcdc_Curr_Max, 1061);
                iq10CurrMax.lData = _IQ10mpy(iq10temp.lData, iq10CurrSampSysa.lData) + iq10CurrSampSysb.lData;
                if (iq10CurrMax.lData < 50)
                {
                    iq10CurrMax.lData = 0;
                }
                else if (iq10CurrMax.lData > (long)30 << 10)
                {
                    iq10CurrMax.lData = (long)30 << 10;
                }

                ucTxBuffer[12] = (unsigned char)(iq10CurrMax.lData >> 10);
                Dcdcisr.ui_Dcdc_Curr_Max = 0;

                uiPfcSendId = 0;
            }

            else if (ucIdTemp == 5)
            {
                ucTxBuffer[1] = 5;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                uiVacaSampSysa = (Uint16)iq10AcaSampSysa.lData;
                iVacaSampSysb = (signed int)(iq10AcaSampSysb.lData >> 3);

                ucTxBuffer[3] = uiVacaSampSysa / 256;
                ucTxBuffer[4] = uiVacaSampSysa % 256;

                ucTxBuffer[5] = (iVacaSampSysb >> 8) & 0x00FF;
                ucTxBuffer[6] = iVacaSampSysb & 0x00FF;

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 6)
            {
                ucTxBuffer[1] = 6;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                uiVacbSampSysa = (Uint16)iq10AcbSampSysa.lData;
                iVacbSampSysb = (signed int)(iq10AcbSampSysb.lData >> 3);

                ucTxBuffer[3] = uiVacbSampSysa / 256;
                ucTxBuffer[4] = uiVacbSampSysa % 256;

                ucTxBuffer[5] = (iVacbSampSysb >> 8) & 0x00FF;
                ucTxBuffer[6] = iVacbSampSysb & 0x00FF;

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 7)
            {
                ucTxBuffer[1] = 7;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                uiVaccSampSysa = (Uint16)iq10AccSampSysa.lData;
                iVaccSampSysb = (signed int)(iq10AccSampSysb.lData >> 3);

                ucTxBuffer[3] = uiVaccSampSysa / 256;
                ucTxBuffer[4] = uiVaccSampSysa % 256;

                ucTxBuffer[5] = (iVaccSampSysb >> 8) & 0x00FF;
                ucTxBuffer[6] = iVaccSampSysb & 0x00FF;

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 10)
            {
                ucTxBuffer[1] = 10;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                ucTxBuffer[3] = uiMemWrAddr / 256;
                ucTxBuffer[4] = uiMemWrAddr % 256;

                ucTxBuffer[5] = uiMemWrValue / 256;
                ucTxBuffer[6] = uiMemWrValue % 256;

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 11)
            {
                ucTxBuffer[1] = 11;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                ucTxBuffer[3] = uiMemRdAddr0 / 256;
                ucTxBuffer[4] = uiMemRdAddr0 % 256;

                ucTxBuffer[5] = uiMemRdAddr1 / 256;
                ucTxBuffer[6] = uiMemRdAddr1 % 256;

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 12)
            {
                ucTxBuffer[1] = 12;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                ucTxBuffer[3] = (unsigned char)((iq10PfcFanRatio_Temp >> 8) & 0x00FF);
                ucTxBuffer[4] = (unsigned char)(iq10PfcFanRatio_Temp & 0x00FF);

                ucTxBuffer[5] = (unsigned char)((iq10PfcFanRatio_Curr >> 8) & 0x00FF);
                ucTxBuffer[6] = (unsigned char)(iq10PfcFanRatio_Curr & 0x00FF);

                uiPfcSendId = 0;
            }
            else if (ucIdTemp == 20)
            {
                ucTxBuffer[1] = 20;
                ucTxBuffer[2] = 4;
                uiTxLength = ucTxBuffer[2] + 2;

                ucTxBuffer[3] = uiEpromWrAddr / 256;
                ucTxBuffer[4] = uiEpromWrAddr % 256;

                ucTxBuffer[5] = uiEpromWrValue / 256;
                ucTxBuffer[6] = uiEpromWrValue % 256;

                uiPfcSendId = 0;
            }

            vTxdAsc(uiTxLength);
            uiTxLength = uiTxLength * 2;

            //
            uiSendChar = ucTxBuffer[1];
            for (k = 2; k < uiTxLength + 1; k++) {
                uiSendChar = uiSendChar ^ ucTxBuffer[k];
            }

            com_int.id = uiHexToAsc(uiSendChar) ;
            ucTxBuffer[uiTxLength + 1] = com_int.bitdata.highchar ;
            ucTxBuffer[uiTxLength + 2] = com_int.bitdata.lowchar ;

            ucTxBuffer[uiTxLength + 3] = 0x0d;

            ucTxBuffer[0] = 0x7e;
            SciaRegs.SCITXBUF = ucTxBuffer[0];
            uiTxPoint = 1;

        }

        if (SciaRegs.SCICTL2.all & 0x80)
        {
            if ( (uiTxPoint >= uiTxLength + 4) || (uiTxLength > 200) )
            {
                uiTxPoint = 0;
                ucSciDatFlag = 0;
                GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1;
            }
            else
            {
                SciaRegs.SCITXBUF = ucTxBuffer[uiTxPoint++];
            }
        }
    }
}

/************************************************/
/*                    */
/************************************************/
void vGetSciData(void)
{
    unsigned char data;
    Uint16 i, j;

    asm(" NOP");

    if(SciaRegs.SCIFFRX.bit.RXFFST >= 1)
    {
        data = SciaRegs.SCIRXBUF.all & 0x00FF;

        if ( uiRxPoint > 30 )
        {
            uiRxPoint = 0;
            return;
        }

        ucRxBuffer[uiRxPoint] = data;
        uiRxPoint++;

        if ( (uiRxPoint == 0x01) || (data == 0x7e) )
        {
            if ( uiRxPoint != 0x01 )
            {
                uiRxPoint = 1;
            }
            if ( data != 0x7e )
            {
                uiRxPoint = 0;
            }
        }
        //
        else if (data == 0x0d)
        {
            ucReceiveOk = 1;
        }
    }

    if (SciaRegs.SCIRXST.bit.RXERROR)
    {
        SciaRegs.SCICCR.all = 0x0027;
        SciaRegs.SCICTL1.all = 0x0003;
        SciaRegs.SCICTL2.all = 0x0000;

        SciaRegs.SCIHBAUD = 0x0000;
        SciaRegs.SCILBAUD = 0x00A2;

        SciaRegs.SCIRXBUF.all = 0x00;
        SciaRegs.SCITXBUF    = 0x00;
        SciaRegs.SCICTL1.all = 0x0023;

        uiRxPoint = 0;
        ucReceiveOk = 0;
    }

    if (ucReceiveOk)
    {
        ucReceiveOk = 0;

        uiCheckChar = ucRxBuffer[1];

        for (j = 2; j < uiRxPoint - 3; j++) {
            uiCheckChar = uiCheckChar ^ ucRxBuffer[j];
        }

        uiSciErrType = RECOK;
        if ( uiCheckChar == ucAscToHex( ucRxBuffer[uiRxPoint - 3], ucRxBuffer[uiRxPoint - 2]) )
        {
            for ( i = 2; i < uiRxPoint; i++)
            {
                ucRxBuffer[i >> 1] = ucAscToHex(ucRxBuffer[i - 1], ucRxBuffer[i]);
                i++ ;
            }

            switch ( ucRxBuffer[1] )
            {
                case 1:
                    if (ucRxBuffer[2] == 11) //13)//11) //
                    {
                        uiSciFailTime = 0;

                        uiVoltAca = (((unsigned int)ucRxBuffer[3] & 0x00FF) << 8) | ((unsigned int)ucRxBuffer[4] & 0x00FF);
                        uiVoltAcb = (((unsigned int)ucRxBuffer[5] & 0x00FF) << 8) | ((unsigned int)ucRxBuffer[6] & 0x00FF);
                        uiVoltAcc = (((unsigned int)ucRxBuffer[7] & 0x00FF) << 8) | ((unsigned int)ucRxBuffer[8] & 0x00FF);
                        uiPfcStatus = (((unsigned int)ucRxBuffer[9] & 0x00FF) << 8) | ((unsigned int)ucRxBuffer[10] & 0x00FF);

                        uiMdlStatus = uiMdlStatus & 0xfefb;

                        if (uiPfcStatus & 0x0010) {
                            uiMdlStatus = uiMdlStatus | dFAN_FAIL;
                        }

                        if (uiPfcStatus & 0x0382) {
                            uiMdlStatus = uiMdlStatus | 0x0100;
                        }

                        uiTempPfc = ucRxBuffer[11] & 0x00FF;
#ifdef HV
                        uiVoltPfcUp = ((unsigned int)ucRxBuffer[12] & 0x00FF) << 1;
                        uiVoltPfcDn = ((unsigned int)ucRxBuffer[13] & 0x00FF) << 1;
#endif

#ifdef LV
                        uiVoltPfcUp = (unsigned int)ucRxBuffer[12] & 0x00FF;
                        uiVoltPfcDn = (unsigned int)ucRxBuffer[13] & 0x00FF;
#endif
                    }

                    break;

                case 11:
                    if (ucRxBuffer[2] == 4)
                    {
                        uiSciFailTime = 0;
                        uiMemAddr0Value = ((ucRxBuffer[3] & 0x00FF) << 8) | (ucRxBuffer[4] & 0x00FF);
                        uiMemAddr1Value = ((ucRxBuffer[5] & 0x00FF) << 8) | (ucRxBuffer[6] & 0x00FF);
                        ECanaMboxes.MBOX4.MDL.word.HI_WORD = 0x01F0;
                        ECanaMboxes.MBOX4.MDL.word.LOW_WORD = 0x00e4;
                        ECanaMboxes.MBOX4.MDH.word.LOW_WORD = uiMemAddr0Value;
                        ECanaMboxes.MBOX4.MDH.word.HI_WORD = uiMemAddr1Value;

                        ECana32Temp.CANTRS.all = 0;
                        ECana32Temp.CANTRS.bit.TRS4 = 1;
                        ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;
                    }

                    break;

                case 12:
                    if(ucRxBuffer[2] == 4)
                    {
                        iq10PfcFanRatio_Temp = (((unsigned int)ucRxBuffer[3]) << 8) | (ucRxBuffer[4]);
                        iq10PfcFanRatio_Curr = (((unsigned int)ucRxBuffer[5]) << 8) | (ucRxBuffer[6]);
                    }
                    break;

                default:
                    uiSciErrType = NOVCOM;
                    break ;
            }
        }
        else {
            uiSciErrType = XORCOM;
        }

    }
}

/******************************************************************

******************************************************************/
unsigned char ucAscToHex(unsigned char ascl, unsigned char  asch)
{
    if ( asch > 0x3a )
    {
        asch = asch - 0x37; /**/
    }
    else
    {
        asch = asch - 0x30;
    }
    asch = asch << 4 ;

    if ( ascl > 0x3a )
    {
        ascl = ascl - 0x37; /**/
    }
    else
    {
        ascl = ascl - 0x30;
    }
    asch = asch | ascl;

    return asch;
}

/******************************************************************

******************************************************************/
unsigned int uiHexToAsc( unsigned char hex  )
{
    ubitint ascreg ;

    ascreg.bitdata.highchar = hex & 0x0f ;
    if ( ascreg.bitdata.highchar >= 10 )
    {
        ascreg.bitdata.highchar = ascreg.bitdata.highchar + 0x37 ;  /**/
    }
    else
    {
        ascreg.bitdata.highchar = ascreg.bitdata.highchar + 0x30 ;  /**/
    }

    ascreg.bitdata.lowchar = hex >> 4   ;
    if ( ascreg.bitdata.lowchar >= 10 )
    {
        ascreg.bitdata.lowchar = ascreg.bitdata.lowchar + 0x37 ;    /**/
    }
    else
    {
        ascreg.bitdata.lowchar = ascreg.bitdata.lowchar + 0x30 ;    /**/
    }
    return ascreg.id;
}

/******************************************************************

******************************************************************/
void vTxdAsc(unsigned int len)//
{
    unsigned int i ;

    ubitint  com_int ;

    for ( i = len; i > 0; i--)
    {
        com_int.id = uiHexToAsc(ucTxBuffer[i]) ;
        ucTxBuffer[i << 1] = com_int.bitdata.lowchar ;
        ucTxBuffer[(i << 1) - 1] = com_int.bitdata.highchar ;
    }
}

/******************************************************************

******************************************************************/
float fStatusTrans_New(void) //
{
    ////////////////////////////
    unsigned int    uiTmpH, uiTmpL;

    uiTmpH = 0;
    uiTmpL = 0;

    if(uiMdlStatus & 0x8001) {
        uiTmpH = uiTmpH | 0x8000;    //
    }
    if(uiMdlStatus & 0x0002) {
        uiTmpH = uiTmpH | 0x4000;    //
    }
    if((uiMdlStatus & 0x8021) || (ucCurrSerUnBal >= 300) || ucIDError) {
        uiTmpH = uiTmpH | 0x2000;    //
    }
    if((uiMdlStatus & 0x2182) || (ucCurrUnBal >= 300) || (Dcdcisr.ui_short_permit == 0) || (Dcdcisr.ui_poweron_off == 0)) ////////////////////////////////////
    {
        uiTmpH = uiTmpH | 0x1000;   //
        if (((uiPfcStatus & 0x01c0) == 0x0100) || (uiPfcStatus & 0x0080)) {
            uiTmpH = uiTmpH & 0xefff;    //
        }
        if((uiMdlStatus & 0x2000) && ((uiMaskWarnCnt < 24000))) {
            uiTmpH = uiTmpH & 0xefff;    //
        }
    }

    if(uiMdlStatus & dFAN_FAIL) {
        uiTmpH = uiTmpH | 0x0800;    //
    }
    if((uiMdlStatus & 0x1000) || (uiPfcStatus & 0x0020)) {
        uiTmpH = uiTmpH | 0x0400;    //
    }
    if(ucLimitStatus & 0x0004) {
        uiTmpH = uiTmpH | 0x0200;    //
    }
    if(ucLimitStatus & 0x0002) {
        uiTmpH = uiTmpH | 0x0100;    //
    }

    if(ucLimitStatus & 0x0001) {
        uiTmpH = uiTmpH | 0x0080;    //
    }
    if(uiMdlStatus & 0x0008) {
        uiTmpH = uiTmpH | 0x0040;    //
    }
    if(uiMdlCtrl & 0x0008) {
        uiTmpH = uiTmpH | 0x0020;    //
    }
    if(uiMdlCtrl & 0x0001) {
        uiTmpH = uiTmpH | 0x0010;    //
    }

    //
    //
    if((!(uiPfcStatus & 0x0001)) && (uiPfcCtrl & 0x0001)) {
        uiTmpH = uiTmpH | 0x0008;    //
    }
    if (uiMdlStatus & 0x4000) {
        uiTmpH = uiTmpH | 0x0004;    //
    }
    if(uiMdlStatus & 0x0200) {
        uiTmpH = uiTmpH | 0x0002;    //
    }
    if((ECanaRegs.CANREC.bit.REC > 10) || (ECanaRegs.CANTEC.bit.TEC > 10)) {
        uiTmpH = uiTmpH | 0x0001;    //
    }

    if(iq10OpenTime.lData > 0) {     //////////////////////////////////////
        uiTmpL = uiTmpL | 0x8000;    //
    }
    //
    //
    if(((uiPfcStatus & 0x01c0) == 0x00c0) || ((uiPfcStatus & 0x01c0) == 0x0100)) {              //
        uiTmpL = uiTmpL | 0x4000;    //
    }
    //
    //
    if((uiPfcStatus & 0x01c0) == 0x0080) {                      //
        uiTmpL = uiTmpL | 0x1000;    //
    }

    if(ucCurrSerUnBal >= 300) {     //
        uiTmpL = uiTmpL | 0x0800;
    }
    if(ucIDError) {                     //
        uiTmpL = uiTmpL | 0x0400;
    }
    if((uiPfcStatus & 0x01c0) == 0x0140) { //
        uiTmpL = uiTmpL | 0x0200;
    }
    if(uiPfcStatus & 0x0002) {          //
        uiTmpL = uiTmpL | 0x0100;
    }

    if(ucCurrUnBal >= 300) {            //
        uiTmpL = uiTmpL | 0x0080;
    }
    //              //
    //

    if ((uiMdlStatus & 0x2000) && (uiMaskWarnCnt >= 24000)) //
    {
        uiTmpL = uiTmpL | 0x0008;   //
        if (((uiPfcStatus & 0x01c0) == 0x0100) || (uiPfcStatus & 0x0080)) {
            uiTmpL = uiTmpL & 0xfff7;    //
        }
    }
    if (uiMdlStatus & 0x0020) {
        uiTmpL = uiTmpL | 0x0004;    //
    }

    tagStatusSd.intdata[1].id = uiTmpH;
    tagStatusSd.intdata[0].id = uiTmpL;

    return tagStatusSd.fd;
}


/************************************************/
/*                                */
/************************************************/
void vGetCanData(void)
{
    Uint32   m, n, trans, recen;
    unsigned char ucAddress, i;
    unsigned int  uiCanCidTmp;
    ubitfloat ubitfData;
    int iDelta;

    asm(" NOP");

    trans = ECanaRegs.CANTA.all;
    recen = ECanaRegs.CANRMP.all;

    if (trans & 0x0020)   //
    {
        ECana32Temp.CANTA.all = 0;
        ECana32Temp.CANTA.bit.TA5 = 1;      //
        ECanaRegs.CANTA.all = ECana32Temp.CANTA.all;

    }
    if (trans & 0x0010)   //
    {
        ECana32Temp.CANTA.all = 0;
        ECana32Temp.CANTA.bit.TA4 = 1;      //
        ECanaRegs.CANTA.all = ECana32Temp.CANTA.all;

        if(ucCANSend)
        {
            ucCANSend++;
            ucMsgType = 0x41;

            if(ucCANSend == 0x02)
            {
                uiCanCidTmp = 0x02;
            }
            else if(ucCANSend == 0x03)
            {
                uiCanCidTmp = 0x03;
            }
            else if(ucCANSend == 0x04)
            {
                uiCanCidTmp = 0x04;
            }
            else if(ucCANSend == 0x05)
            {
                uiCanCidTmp = 0x05;
            }
            else if(ucCANSend == 0x06)
            {
                uiCanCidTmp = 0x07;
            }
            else if(ucCANSend == 0x07)
            {
                uiCanCidTmp = 0x40;
                ucMsgType = 0x42;
            }
            else if(ucCANSend == 0x08)
            {
                uiCanCidTmp = 0x54;
            }
            else if(ucCANSend == 0x09)
            {
                ucCANSend = 0;
                uiCanCidTmp = 0x58;
            }
            else if(ucCANSend == 0x11)
            {
                uiCanCidTmp = 0x0d;
            }
            else if(ucCANSend == 0x12)
            {
                uiCanCidTmp = 0x0e;
            }
            else if(ucCANSend == 0x13)
            {
                ucCANSend = 0;
                uiCanCidTmp = 0x40;
                ucMsgType = 0x42;
            }

            else if(ucCANSend == 0x21)
            {
                uiCanCidTmp = 0x11;
            }
            else if(ucCANSend == 0x22)
            {
                uiCanCidTmp = 0x12;
            }
            else if(ucCANSend == 0x23)
            {
                uiCanCidTmp = 0x13;
            }
            else if(ucCANSend == 0x24)
            {
                uiCanCidTmp = 0x51;
            }
            else if(ucCANSend == 0x25)
            {
                uiCanCidTmp = 0x55;
            }
            else if(ucCANSend == 0x26)
            {
                uiCanCidTmp = 0x56;
            }
            else if(ucCANSend == 0x27)
            {
                uiCanCidTmp = 0x5A;
            }
            else if(ucCANSend == 0x28)
            {
                uiCanCidTmp = 0x5B;
            }
            else if(ucCANSend == 0x29)
            {
                uiCanCidTmp = 0x5C;
            }
            else if(ucCANSend == 0x2A)
            {
                ucCANSend = 0;
                uiCanCidTmp = 0x5D;
            }

            if (ucCANSend == 0)
            {
                ECana32Temp.CANME.all = 0x00000033;
                ECana32Temp.CANME.bit.ME4 = 0;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;

                ECanaMboxes.MBOX4.MSGID.bit.CNT = 0;

                ECana32Temp.CANME.all = 0x00000023;
                ECana32Temp.CANME.bit.ME4 = 1;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;
            }

            uiCanDataId = uiCanCidTmp;
            ubitfData.lData = 0;
            vRmpProtocol(ubitfData);

        }
    }

    if (recen & 0x0002)     //
    {
        ECana32Temp.CANRMP.all = 0;
        ECana32Temp.CANRMP.bit.RMP1 = 1;
        ECanaRegs.CANRMP.all = ECana32Temp.CANRMP.all;
        m = ECanaMboxes.MBOX1.MDL.word.HI_WORD;
        n = ECanaMboxes.MBOX1.MDL.word.LOW_WORD;
        ubitfData.intdata[0].id = ECanaMboxes.MBOX1.MDH.word.LOW_WORD;
        ubitfData.intdata[1].id = ECanaMboxes.MBOX1.MDH.word.HI_WORD;
        ucAddress = ECanaMboxes.MBOX1.MSGID.bit.DSTADDRH << 5 | ECanaMboxes.MBOX1.MSGID.bit.DSTADDRL;

        iq10Tmp.lData = _IQ10(ubitfData.fd);
        iq12Tmp.lData = _IQ12(ubitfData.fd);

        if((ucAddress == 0xff) || (ucAddress == uiMdlAddr))
        {
            if(ucAddress == 0xff) {
                ucBroadCastCom = 1;
            }
            else {
                ucBroadCastCom = 0;
            }
            uiCanFailTime = 0;
            uiMdlStatus = uiMdlStatus & 0xfbff;     //
            ucMsgType = (unsigned char)((m & 0xff00) >> 8);
            if ((ucMsgType == 0x00) || (ucMsgType == 0x10) || (ucMsgType == 0x20))
            {
                if(ucMsgType == 0x00)
                {
                    uiRdMemNum = 0;
                    uiCodeType = 0xf0;
                    //
                    if((iq10Tmp.lData >= iq10RunTime.lData + 1000) && (iq10Tmp.lData <= iq10RunTime.lData + 1050))
                    {
                        uiEepromFlag = uiEepromFlag | 0x0001;
                        //
                        iq10RunTime.lData = iq10Tmp.lData;
                    }
                    ucVoltAdjDelta = n & 0x00ff;

                    if (Dcdcisr.ui_Dcdc_debug)
                    {
                        if (n & 0x8000)         //
                        {
                            uiMdlStatus = uiMdlStatus | 0x0010;
                        }
                        else if ((uiMdlStatus & 0x8083) == 0)   //
                        {
                            uiMdlStatus = uiMdlStatus & 0xFFEF;
                        }
                        else
                        {
                            uiCodeType = 0xf2;
                        }
                    }
                    else
                    {
                        if (n & 0x8000)         //
                        {
                            uiMdlStatus = uiMdlStatus | 0x0010;
                        }
                        //      //
                        //      //
                        //
                        else if (((uiMdlStatus & 0xa187) == 0) && (uiPfcOnOff == 0x0) && (uiPfcStatus & 0x01)) //
                        {
                            uiMdlStatus = uiMdlStatus & 0xFFEF;
                        }
                        else
                        {
                            uiCodeType = 0xf2;
                        }
                    }

                    if ((n & 0x4000) && (uiMdlStatus & 0x8000))     //
                    {
                        uiCloseTime = 0;
                        ucOverVoltStatus = 0;
                        ucOverVoltCount = 0;
                        uiMdlStatus = uiMdlStatus & 0x7FFE;
                    }
                    /* */
                    if ((n & 0x2000) != ((uiMdlCtrl & 0x0001) << 13))
                    {
                        uiEepromFlag = uiEepromFlag | 0x0002;

                        if (n & 0x2000)             //
                        {
                            uiMdlCtrl = uiMdlCtrl | 0x0001;
                        }
                        else //     //
                        {
                            uiMdlCtrl = uiMdlCtrl & 0xFFFE;
                        }
                    }

                    /*  */
                    if ((n & 0x1000) != ((uiMdlCtrl & 0x0008) << 9))
                    {
                        uiEepromFlag = uiEepromFlag | 0x0002;
                        if (n & 0x1000)             //
                        {
                            uiMdlCtrl = uiMdlCtrl | 0x0008;
                        }
                        else //         //
                        {
                            uiMdlCtrl = uiMdlCtrl & 0xFFF7;
                        }
                    }

                    /**/
                    if (n & 0x0800)                             //
                    {
                        uiMdlStatus = uiMdlStatus | 0x4000;
                    }
                    else
                    {
                        uiMdlStatus = uiMdlStatus & 0xBFFF;
                    }

                    /*  */
                    if (n & 0x0400)                             //
                    {
                        //
                        uiPfcCtrl = uiPfcCtrl | 0x0001;         //
                    }
                    else
                    {
                        uiPfcCtrl = uiPfcCtrl & 0xfffe;         //
                    }

                    /* */
                    if ((n & 0x0200) == 0x0200)
                    {
                        uiEepromFlag = uiEepromFlag | 0x0004;
                    }
                    /* */
                    if ((n & 0x0100) != ((uiMdlCtrl & 0x0002) << 7))
                    {
                        uiEepromFlag = uiEepromFlag | 0x0002;
                        //
                        if (n & 0x0100)             //
                        {
                            uiMdlCtrl = uiMdlCtrl | 0x0002;
                        }
                        else// if
                        {
                            uiMdlCtrl = uiMdlCtrl & 0xFFFD;
                        }
                    }

                    ucCANSend = 1;
                    uiCanCidTmp = 0x01;
                }
                else if(ucMsgType == 0x10)
                {


                    ucCANSend = 0x10;
                    uiCanCidTmp = 0x0C;
                }
                else                                //
                {
                    ucCANSend = 0x20;
                    uiCanCidTmp = 0x0F;
                }

                ucMsgType = 0x41;

                ECana32Temp.CANME.all = 0x00000033;
                ECana32Temp.CANME.bit.ME4 = 0;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;

                ECanaMboxes.MBOX4.MSGID.bit.CNT = 1;

                ECana32Temp.CANME.all = 0x00000023;
                ECana32Temp.CANME.bit.ME4 = 1;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;

                uiCanDataId = uiCanCidTmp;
                ubitfData.lData = 0;
                vRmpProtocol(ubitfData);

            }
            else
            {
                if((ucMsgType == 0x01) || (ucMsgType == 0x02 ) || (ucMsgType == 0x03 ))
                {
                    ucMsgType |= 0x40;
                    ucErrType = 0xf0;
                }
                else
                {
                    ucErrType = 0xf2;
                }
                uiCanDataId = n;
                vRmpProtocol(ubitfData);
            }
        }
    }
    if (recen & 0x0001)     //
    {   //
        ECana32Temp.CANRMP.all = 0;
        ECana32Temp.CANRMP.bit.RMP0 = 1;
        ECanaRegs.CANRMP.all = ECana32Temp.CANRMP.all;
        m = ECanaMboxes.MBOX0.MDL.word.HI_WORD;
        n = ECanaMboxes.MBOX0.MDL.word.LOW_WORD;
        ubitfData.intdata[0].id = ECanaMboxes.MBOX0.MDH.word.LOW_WORD;
        ubitfData.intdata[1].id = ECanaMboxes.MBOX0.MDH.word.HI_WORD;
        ucAddress = ECanaMboxes.MBOX0.MSGID.bit.SRCADDR;
        ucMsgType = (unsigned char)((m & 0xff00) >> 8);

        iq10Tmp.lData = _IQ10(ubitfData.fd);

        if ((uiSequeOnTimer > 4) && (ucIDError == 0)) //       //
        {
            if (ucAddress == uiMdlAddr)
            {
                ucAddIdentify = 1;
                ucAddIdentifyNum = 0;
                ucIDError = 1;
            }
        }

        if ( ucMsgType == 0x01)
        {
            m = n;
            m = m & 0x0fff;
            if (m >= 0x0800)
            {
                m = m | 0xf000;
            }
            iDelta = m;
            if(ucAddress <= MAX_NUM)
            {
                iMdlCurrDelta[ucAddress] = iDelta;//
                ucMdlOnoff[ucAddress] = (n & 0xF000) >> 12;
                iq10MdlCurrent[ucAddress].lData = iq10Tmp.lData;
                if (ucAddress <= ucAddressSmall)
                {
                    ucAddressSmall = ucAddress;
                }
                uiBox0FailTimer = 0;
            }
        }
        else if (ucMsgType == 0x02)
        {
            uiCurrCtrlFlag = n;
            iq10MdlAvgCurr.lData = iq10Tmp.lData;
        }
        else if (ucMsgType == 0x04)
        {
            if (n == 0x015a)
            {
                uwAveCurrCalIndex = 0; //

                /*if(ucAddIdentify!=1)
                {
                    ucAddIdentify=2;
                    ucAddIdentifyNum=1;
                }*/

                uiMdlAddr = 0;
                uiSequeOnTimer = 0;

                ECana32Temp.CANME.all = 0x00000033;
                ECana32Temp.CANME.bit.ME4 = 0;
                ECana32Temp.CANME.bit.ME5 = 0;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;

                ECanaMboxes.MBOX4.MSGID.bit.SRCADDR = uiMdlAddr;
                ECanaMboxes.MBOX5.MSGID.bit.SRCADDR = uiMdlAddr;

                ECana32Temp.CANME.all = 0x00000003;
                ECana32Temp.CANME.bit.ME4 = 1;
                ECana32Temp.CANME.bit.ME5 = 1;
                ECanaRegs.CANME.all = ECana32Temp.CANME.all;

                for (i = 0; i < MAX_NUM; i++)
                {
                    fMdlAddID[i].msgaddid[1] = 0;
                    fMdlAddID[i].msgaddid[0] = 0;
                }

                ulSelfChkTimer = 0;         //
                uiSelfChkFlag = 0;
                uiSelfchkAddr = uiMdlAddr;
                uiSelfChkStart = 0;
            }
        }
        else if (ucMsgType == 0x03)
        {
            if ((n & 0x00ff) == 0x005a)
            {
                ucAddressCalFlag = 1;
                fMdlAddID[MAX_NUM + 1].msgaddid[0] = ubitfData.intdata[1].id;
                fMdlAddID[MAX_NUM + 1].msgaddid[1] = ubitfData.intdata[0].id;
                fMdlAddID[MAX_NUM + 1].msgaddid[2] = n;
                uiSequeOnTimer = 0;
            }
        }

    }

    //lnint_stack();              //2018-11-01 LNINT协议栈
}

/************************************************/
/*                             */
/************************************************/
void vRmpProtocol(ubitfloat tempfloat)
{
    unsigned int  uiCanCidTmp;
    ubitfloat      ubitfData;
    unsigned char ucCodeType;
    unsigned int  *  setadd;
    //
    signed long lreftmp;
    //
    ubitfData.lData = tempfloat.lData;
    ucCodeType = RECOK;

    uiCanCidTmp = uiCanDataId;
    switch (uiCanDataId)
    {
        //
        case cInqNewRec:
            ubitfData.lData = 0x12345678;
            break;
        //
        case 0x01:              //
            //
            if (iq10MdlVolt.lData > 153600) {
                ubitfData.fd =  _IQ10toF(iq10MdlVolt.lData + (signed long)100 - (iq10MdlCurr.lData >> 9));
            }
            else {
                ubitfData.fd =  _IQ10toF(iq10MdlVolt.lData);
            }

            if (ucCANSend == 0x01) {
                ucCodeType = uiCodeType & 0x00ff;
            }
            break;

        case 0x02:              //
            ubitfData.fd =  _IQ10toF(iq10MdlCurr.lData);
            break;

        case 0x03:              //
            if (iq10MdlLimit.lData >= dQ10_CURR_LIMIT_MAX) {
                ubitfData.fd = _IQ10toF(dQ10_CURR_LIMIT_MAX);
            }
            else {
                ubitfData.fd = _IQ10toF(iq10MdlLimit.lData);
            }
            break;

        case 0x04:              //
            //
            ubitfData.fd = _IQ10toF(iq10TempDc.lData); //
            break;

        case 0x05:              //
            if (uiMdlAddr % 3 == 0) {
                ubitfData.fd = _IQ10toF(iq10Vaca.lData);
            }
            else if (uiMdlAddr % 3 == 1) {
                ubitfData.fd = _IQ10toF(iq10Vacb.lData);
            }
            else {
                ubitfData.fd = _IQ10toF(iq10Vacc.lData);
            }
            break;

        case 0x06:              //
            ubitfData.fd =  _IQ10toF(iq10MdlVoltUp.lData);
            break;

        case 0x07:              //
            ubitfData.fd =  _IQ10toF(iq10CurrDisp.lData); //
            break;

        case 0x08:              //
            ubitfData.fd = uiVoltPfcUp;
            break;

        case 0x09:              //
            ubitfData.fd = _IQ10toF(iq10MdlTempUp.lData);
            break;

        case 0x0a:              //
            ubitfData.fd = uiVoltPfcDn;
            break;

        case 0x0b:              //
            ubitfData.fd =  _IQ10toF(iq10TempCtrl.lData);
            break;

        case 0x0c:              //
            //  iq10AcVab.lData = 204800;
            ubitfData.fd = _IQ10toF(iq10AcVab.lData);
            break;

        case 0x0d:              //
            ubitfData.fd = _IQ10toF(iq10AcVbc.lData);
            break;

        case 0x0e:              //
            ubitfData.fd = _IQ10toF( iq10AcVca.lData);
            break;

        case 0x0f:              //
            ubitfData.fd =  _IQ10toF(iq10MdlVoltFt.lData);
            break;

        case 0x10:              //
            ubitfData.fd = (signed int)uiTempPfc - 50;
            break;

        case 0x11:              //
            ubitfData.fd =  15000;//
            break;

        case 0x12:              //
            ubitfData.fd =  30;
            break;

        case 0x13:              //
            ubitfData.fd =  380;
            break;

        case 0x14:              //
            ubitfData.fd =  _IQ10toF(iq10SetPowerWant.lData);
            break;

        case 0x18:              //
            //if ( (ubitfData.fd <= 1.22) && (ubitfData.fd >= 0.001) )
            if ( (iq10Tmp.lData <= dQ10_POWER_LIMIT_MAX) && (iq10Tmp.lData >= 10) ) //
            {
                if (labs(iq10Tmp.lData - iq10MdlPowerFt.lData) >= 2)
                {
                    iq10MdlPowerFt.lData = iq10Tmp.lData + 1;
                    uiEepromFlag = uiEepromFlag | 0x0200;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x19:              //
            //if ( (ubitfData.fd <= 1.22) && (ubitfData.fd >= 0.001) )
            if ( (iq10Tmp.lData <= dQ10_CURR_LIMIT_MAX) && (iq10Tmp.lData >= 10) )//
            {
                if (labs(iq10Tmp.lData - iq10MdlCurrFt.lData) >= 2)
                {
                    iq10MdlCurrFt.lData = iq10Tmp.lData + 1;
                    uiEepromFlag = uiEepromFlag | 0x0400;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x1a:
            if ( (iq10Tmp.lData < (long)44 << 10) && (iq10Tmp.lData >= 1000) )
            {
                if (labs(iq10Tmp.lData - iq10AcCurrFt.lData) >= 10)
                {
                    iq10AcCurrFt.lData = iq10Tmp.lData + 1;
                    uiEepromFlag = uiEepromFlag | 0x0800;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x20:
            if ( (iq10Tmp.lData <= dQ10_POWER_LIMIT_MAX) && (iq10Tmp.lData >= 10) ) //
            {
                iq10SetPower.lData = iq10Tmp.lData + 1;
            }
            else if ( (iq10Tmp.lData > dQ10_POWER_LIMIT_MAX) && (iq10Tmp.lData <= 2048) )
            {
                iq10SetPower.lData = 1126;      //???2018-10-25 原始程序写法意义不明
            }
            else if ( (iq10Tmp.lData >= 0) && (iq10Tmp.lData < 10))
            {
                iq10SetPower.lData = 10;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x21:
            if (uiForbidConnect)
            {
                iq10SetVolt.lData = iq10Tmp.lData;
            }
            else
            {
                if ((iq10Tmp.lData <= (iq10MdlVoltUp.lData - 5120)) && (iq10Tmp.lData >= dQ10_VOUT_MIN))
                {
                    iq10SetVolt.lData = iq10Tmp.lData + 1;
                }
                else
                {
                    ucCodeType = 0xf2;
                }
            }

            break;

        case 0x22:
            if ( (iq10Tmp.lData <= dQ10_CURR_LIMIT_MAX) && (iq10Tmp.lData >= 10) )
            {
                iq10SetLimit.lData = iq10Tmp.lData + 1;
            }
            else if ( (iq10Tmp.lData > dQ10_CURR_LIMIT_MAX) && (iq10Tmp.lData <= 3000) )
            {

                iq10SetLimit.lData = dQ10_CURR_LIMIT_MAX;
            }
            else if ( (iq10Tmp.lData >= 0) && (iq10Tmp.lData < 10))
            {
                iq10SetLimit.lData = 10;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x23:
            if (uiForbidConnect)
            {
                iq10MdlVoltUp.lData = iq10Tmp.lData;
            }
            else
            {
                if((iq10Tmp.lData <= dQ10_OVP_MAX) && (iq10Tmp.lData >= dQ10_OVP_MIN)
                        && (iq10Tmp.lData >= iq10SetVolt.lData + 1024) && (iq10Tmp.lData >= iq10MdlVoltFt.lData + 1024))
                {
                    if (labs(iq10Tmp.lData - iq10MdlVoltUp.lData) >= 10)
                    {
                        iq10MdlVoltUp.lData = iq10Tmp.lData + 1;
                        uiEepromFlag = uiEepromFlag | 0x0008;
                    }
                }
                else
                {
                    ucCodeType = 0xf2;
                }
            }
            break;

        case 0x24:              //
            //if ( (ubitfData.fd <= (tagMdlVoltUp.fd-0.5)) && (ubitfData.fd >= 48.0) )
            if ((iq10Tmp.lData <= iq10MdlVoltUp.lData - 512) && (iq10Tmp.lData >= dQ10_VOUT_MIN))
            {
                if (labs(iq10Tmp.lData - iq10MdlVoltFt.lData) >= 10)
                {
                    iq10MdlVoltFt.lData = iq10Tmp.lData + 1;
                    uiEepromFlag = uiEepromFlag | 0x0010;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x25:              //
            if ((iq10Tmp.lData <= (long)150 << 10) && (iq10Tmp.lData >= (long)70 << 10))
            {
                if (iq10Tmp.lData != iq10MdlTempUp.lData)
                {
                    ucCodeType = ucWriteFloatDataThree(92, iq10Tmp);
                    if (ucCodeType == RECOK)
                    {
                        iq10MdlTempUp.lData = iq10Tmp.lData;
                    }
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x26:              //
            if ((iq10Tmp.lData > 0) && (iq10Tmp.lData < (long)255 << 10))
            {
                ucVoltAdjDelta = (unsigned char)(iq10Tmp.lData >> 10);
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x27:              //
            if ((iq10Tmp.lData <= (long)900 << 10) && (iq10Tmp.lData >= (long)50 << 10))
            {
                uiVpfcSet = (unsigned int)(iq10Tmp.lData >> 10);
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x28:              //
            //if ( (ubitfData.fd <= 300.0) && (ubitfData.fd >= 50.0) )
            if ((iq10Tmp.lData <= (long)300 << 10) && (iq10Tmp.lData >= (long)50 << 10))
            {
                if (iq10Tmp.lData * 100 >= 3 * (iq10LoadSstartTime.lData + ((long)750 << 10)))
                {
                    if (iq10Tmp.lData != iq10ReonTime.lData)
                    {
                        iq10ReonTime.lData = iq10Tmp.lData;
                        uiEepromFlag = uiEepromFlag | 0x0040;
                    }
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x29:              //
            //
            //
            if ((iq10Tmp.lData <= (long)12800 << 10) && (iq10Tmp.lData >= (long)800 << 10))
            {
                if (iq10Tmp.lData >= (long)9000 << 10) {    //
                    iq10Tmp.lData = (long)9000 << 10;
                }

                if (iq10Tmp.lData <= (iq10ReonTime.lData * 100 / 3 - ((long)750 << 10)))
                {
                    if (iq10Tmp.lData != iq10LoadSstartTime.lData)
                    {
                        iq10LoadSstartTime.lData = iq10Tmp.lData;
                        uiEepromFlag = uiEepromFlag | 0x0080;
                    }
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x2A:              //
            //if ( (ubitfData.fd <= 10.0) && (ubitfData.fd >= 0.0) )
            if ((iq10Tmp.lData <= (long)10 << 10) && (iq10Tmp.lData >= 0))
            {
                //
                if (iq10Tmp.lData != iq10OpenTime.lData)
                {
                    //
                    iq10OpenTime.lData = iq10Tmp.lData;
                    if(iq10OpenTime.lData <= 1) {
                        ucOpenOrder = 0;
                    }
                    else {
                        ucOpenOrder = 1;
                    }
                    uiEepromFlag = uiEepromFlag | 0x0100;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;
        //
        case 0x2C: //
            ubitfData.fd =  _IQ10toF(iq10PfcFanRatio_Temp);
            break;
        case 0x2D: //
            ubitfData.fd =  _IQ10toF(iq10PfcFanRatio_Curr);
            break;
        case 0x2E: //
            if(iq10Tmp.lData > (long)10 << 10)
            {
                iq10Tmp.lData = (long)10 << 10;
            }
            else if(iq10Tmp.lData < 0)
            {
                iq10Tmp.lData = 0;
            }
            iq10PfcFanRatio_Temp = (int)iq10Tmp.lData;
            uiPfcSendId = 12;
            break;
        case 0x2F: //
            if(iq10Tmp.lData > (long)1 << 10)
            {
                iq10Tmp.lData = (long)1 << 10;
            }
            else if(iq10Tmp.lData < 0)
            {
                iq10Tmp.lData = 0;
            }
            iq10PfcFanRatio_Curr = (int)iq10Tmp.lData;
            uiPfcSendId = 12;
            break;
        //
        case 0x30:  //
            if (ubitfData.intdata[1].id == 1)
            {
                //
                uiMdlStatus |= 0x0008; //
                uiActionReady = 0;
                uiMdlStatus = uiMdlStatus | 0x0010;
                uwOnOffDCDC = 0x01;

                //
                iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
                lreftmp = _IQ10mpy(iq10_VFACTOR, (_IQ10mpy((iq10SetVoltTrue.lData), (iq12VoltConSysa.lData >> 2))) + (iq12VoltConSysb.lData >> 2));
                if (lreftmp > (long)32400 << 10)
                {
                    uiDCVoltref = 32400;
                }
                else
                {
                    uiDCVoltref = (unsigned int)(lreftmp >> 10);
                }
                Dcdcisr.ui_Dcdc_Volt_Ref = uiDCVoltref;

            }
            else if ((uiMdlStatus & 0xa187) == 0) //
            {
                uiMdlStatus = uiMdlStatus & 0xFFEF; //
                uwOnOffDCDC = 0x0;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x31:              //
            if ((ubitfData.intdata[1].id == 1) && (uiMdlStatus & 0x8000))       //
            {
                uiCloseTime = 0;
                ucOverVoltStatus = 0;
                ucOverVoltCount = 0;
                uiMdlStatus = uiMdlStatus & 0x7FFE;
            }
            else if  (ubitfData.intdata[1].id == 0)     //
            {
                ;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x32:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                if ((uiMdlCtrl & 0x0001) == 0)
                {
                    uiMdlCtrl = uiMdlCtrl | 0x0001;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else if (ubitfData.intdata[1].id == 0)        //
            {
                if (uiMdlCtrl & 0x0001)
                {
                    uiMdlCtrl = uiMdlCtrl & 0xfffe;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x33:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                if ((uiMdlCtrl & 0x0008) == 0)
                {
                    uiMdlCtrl = uiMdlCtrl | 0x0008;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                if (uiMdlCtrl & 0x0008)
                {
                    uiMdlCtrl = uiMdlCtrl & 0xFFF7;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x34:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                uiMdlStatus = uiMdlStatus | 0x4000;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                uiMdlStatus = uiMdlStatus & 0xBFFF;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x35:              //
            //if (ubitfData.intdata[1].id )
            //{
            //  ucCodeType = 0xf2;
            //}
            if (ubitfData.intdata[1].id == 1)
            {
                uiPfcCtrl = uiPfcCtrl | 0x0001;         //
                //
                uiMdlStatus |= 0x0008; //
                uiActionReady = 0;
                uiMdlStatus = uiMdlStatus | 0x0010;
                uwOnOffDCDC = 0x01;

                //
                iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
                lreftmp = _IQ10mpy(iq10_VFACTOR, (_IQ10mpy((iq10SetVoltTrue.lData), (iq12VoltConSysa.lData >> 2))) + (iq12VoltConSysb.lData >> 2));
                if (lreftmp > (long)32400 << 10)
                {
                    uiDCVoltref = 32400;
                }
                else
                {
                    uiDCVoltref = (unsigned int)(lreftmp >> 10);
                }
                Dcdcisr.ui_Dcdc_Volt_Ref = uiDCVoltref;

            }
            else if (ubitfData.intdata[1].id == 0)
            {
                uiPfcCtrl = uiPfcCtrl & 0xfffe;         //
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x36:          //
            if (ubitfData.intdata[1].id == 1)           //
            {
                InitECana();
            }
            else if  (ubitfData.intdata[1].id == 0)     //
            {
                ;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x37:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                ucPermitAdjust = 1;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                ucPermitAdjust = 0;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x39:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                if (uiMdlCtrl & 0x0004)
                {
                    uiMdlCtrl = uiMdlCtrl & 0xFFFB;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                if ((uiMdlCtrl & 0x0004) == 0)
                {
                    uiMdlCtrl = uiMdlCtrl | 0x0004;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;


        case 0x3A:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                if((uiMdlCtrl & 0x0002) == 0)
                {
                    uiMdlCtrl = uiMdlCtrl | 0x0002;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {

                if (uiMdlCtrl & 0x0002)
                {
                    uiMdlCtrl = uiMdlCtrl & 0xFFFD;
                    uiEepromFlag = uiEepromFlag | 0x0002;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        /*case 0x3B:                //
            if (ubitfData.intdata[1].id == 1)           //
            {
                ucMemoryScope=1;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                ucMemoryScope=0;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;  */

        case 0x3C:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                ucAddIdentifyNum = 0;               //
                ucAddIdentify = 1;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                ;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        /*case 0x3D:                //
            if (ubitfData.intdata[1].id == 1)           //
            {
                if ((uiMdlCtrl&0x0010)==0)
                {
                    uiEepromFlag = uiEepromFlag|0x0002;
                    uiMdlCtrl = uiMdlCtrl|0x0010;
                }

            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                if (uiMdlCtrl&0x0010)
                {
                    uiEepromFlag = uiEepromFlag|0x0002;
                    uiMdlCtrl = uiMdlCtrl&0xFFEF;
                }
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;*/

        case 0x3E:              //
            if (ubitfData.intdata[1].id == 1)           //
            {
                uiForbidConnect = 1;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                uiForbidConnect = 0;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        /*case 0x3F:                //
            if (ubitfData.intdata[1].id == 1)           //
            {
                uiSelfDiagnosis = 1;
            }
            else if (ubitfData.intdata[1].id == 0)      //
            {
                uiSelfDiagnosis = 0;
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;*/

        case 0x40:              //
            ubitfData.fd = fStatusTrans_New();//
            break;

        //
        case 0x45:
            ubitfData.lData = ubitfCodeChkRd0.lData;
            break;
        case 0x46:
            ubitfData.lData = ubitfCodeChkRd1.lData;
            break;
        case 0x47:
            ubitfData.lData = ubitfCodeChkRd2.lData;
            break;

        case 0x49:
            ubitfData.intdata[0].id = uwPfcFirmwareVer;
            ubitfData.intdata[1].id = cDcdcFirmwareVer;
            break;
        case 0x4A:
            ubitfData.fd = ubitTestFlag.fd;
            break;

        case 0x50:              //
            ubitfData.intdata[1].id = iCurrDelta;
            ubitfData.intdata[0].id = 0;
            break;

        case 0x51:              //
            ubitfData.intdata[1].id = uiCharactData0H;
            ubitfData.intdata[0].id = uiCharactData0L;
            break;

        case 0x52:              //
            ubitfData.intdata[1].id = uiMaintainData0H;
            ubitfData.intdata[0].id = uiMaintainData0L;
            break;

        case 0x53:              //
            ubitfData.intdata[1].id = uiMaintainData1H;
            ubitfData.intdata[0].id = uiMaintainData1L;
            break;

        case 0x54:              //
            ubitfData.intdata[1].id = uiNodeId0H;
            ubitfData.intdata[0].id = uiNodeId0L;
            break;

        case 0x55:              //
            ubitfData.intdata[1].id = uiNodeId1H;
            ubitfData.intdata[0].id = uiNodeId1L;
            break;

        case 0x56:              //
            ubitfData.intdata[1].id = uiVersionNoHw;
            ubitfData.intdata[0].id = uiVersionNoSw;
            break;

        case 0x58:              //
            ubitfData.fd = _IQ10toF(iq10RunTime.lData);

            //ubitfData.fd = _IQ10toF(iq10TempCtrl.lData);
            break;




        case 0x59:              //
            ubitfData.fd = _IQ10toF(iq10MaintainTimes.lData);
            break;

        case 0x5a:              //
            ubitfData.intdata[1].id = uiBarCode0H;
            ubitfData.intdata[0].id = uiBarCode0L;
            break;

        case 0x5b:              //
            ubitfData.intdata[1].id = uiBarCode1H;
            ubitfData.intdata[0].id = uiBarCode1L;
            break;

        case 0x5c:              //
            ubitfData.intdata[1].id = uiBarCode2H;
            ubitfData.intdata[0].id = uiBarCode2L;
            break;

        case 0x5d:              //
            ubitfData.intdata[1].id = uiBarCode3H;
            ubitfData.intdata[0].id = uiBarCode3L;
            break;

        case 0x5f:              //
            ubitfData.fd = _IQ10toF((uiMdlCtrl & 0x0004) >> 2);
            break;

        //
        case 0x60:              //
            uiMdlCalibra = 0;
            iq10SetSysTmp.lData = iq12Tmp.lData;
            iq12Tmp.lData = _IQ12mpy(iq12VoltSampSysa.lData, iq12Tmp.lData);
            if((iq12Tmp.lData >= 3200 ) && (iq12Tmp.lData <= 5000))
            {
                iq10AdjTmp.lData = iq12Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x61:              //
            iq12Tmp.lData = _IQ12mpy(iq12VoltSampSysb.lData, iq10SetSysTmp.lData) + iq12Tmp.lData;
            if((iq12Tmp.lData >=  (long) -50 << 12 ) && (iq12Tmp.lData <= (long)50 << 12) && uiMdlCalibra)
            {
                ucCodeType = ucWriteFloatDataThree(4, iq10AdjTmp);
                ucCodeType = ucCodeType | ucWriteFloatDataThree(8, iq12Tmp);
                if (ucCodeType == RECOK)
                {
                    iq12VoltSampSysa.lData = iq10AdjTmp.lData;
                    iq12VoltSampSysb.lData = iq12Tmp.lData;
                }
                else {
                    ucIDError = 1;    //
                }
            }
            else
            {
                ucCodeType = 0xf2;
                //  ucIDError = 1;                  //
            }
            break;

        case 0x62:              //
            uiMdlCalibra = 0;
            iq10SetSysTmp.lData = iq10Tmp.lData;
            iq10Tmp.lData = _IQ10mpy(iq10CurrSampSysa.lData, iq10Tmp.lData);
            if((iq10Tmp.lData >= 512 ) && (iq10Tmp.lData <= 1536))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x63:              //
            iq10Tmp.lData = _IQ10mpy(iq10CurrSampSysb.lData, iq10SetSysTmp.lData) + iq10Tmp.lData;
            if((iq10Tmp.lData >=  (long) -5 << 10 ) && (iq10Tmp.lData <= (long)5 << 10) && uiMdlCalibra)
            {
                ucCodeType = ucWriteFloatDataThree(20, iq10AdjTmp);
                ucCodeType = ucCodeType | ucWriteFloatDataThree(24, iq10Tmp);
                if (ucCodeType == RECOK)
                {
                    iq10CurrSampSysa.lData = iq10AdjTmp.lData;
                    iq10CurrSampSysb.lData = iq10Tmp.lData;
                }
                else {
                    ucIDError = 1;    //
                }
            }
            else
            {
                ucCodeType = 0xf2;
                //      ucIDError = 1;
            }
            break;

        case 0x64:              //
            uiMdlCalibra = 0;
            iq10SetSysTmp.lData = iq12Tmp.lData;
            iq12Tmp.lData = _IQ12mpy(iq12VoltConSysa.lData, iq12Tmp.lData);
            if((iq12Tmp.lData >= 3200 ) && (iq12Tmp.lData <= 5000))
            {
                iq10AdjTmp.lData = iq12Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x65:              //
            iq12Tmp.lData = _IQ12mpy(iq12VoltConSysb.lData, iq10SetSysTmp.lData) + iq12Tmp.lData;
            if((iq12Tmp.lData >=  (long) -50 << 12 ) && (iq12Tmp.lData <= (long)50 << 12) && uiMdlCalibra)
            {
                ucCodeType = ucWriteFloatDataThree(12, iq10AdjTmp);
                ucCodeType = ucCodeType | ucWriteFloatDataThree(16, iq12Tmp);
                if (ucCodeType == RECOK)
                {
                    iq12VoltConSysa.lData = iq10AdjTmp.lData;
                    iq12VoltConSysb.lData = iq12Tmp.lData;
                }
                else {
                    ucIDError = 1;    //
                }
            }
            else
            {
                ucCodeType = 0xf2;
                //  ucIDError = 1;                  //
            }
            break;

        case 0x66:              //
            uiMdlCalibra = 0;
            iq10SetSysTmp.lData = iq10Tmp.lData;
            iq10Tmp.lData = _IQ10mpy(iq10CurrConSysa.lData, iq10Tmp.lData);
            if((iq10Tmp.lData >= 512 ) && (iq10Tmp.lData <= 1536))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x67:              //
            iq10Tmp.lData = _IQ10mpy(iq10CurrConSysb.lData, iq10SetSysTmp.lData) + iq10Tmp.lData;
            if((iq10Tmp.lData >=  (long) -5 << 10 ) && (iq10Tmp.lData <= (long)5 << 10) && uiMdlCalibra)
                // //
            {
                ucCodeType = ucWriteFloatDataThree(28, iq10AdjTmp);
                ucCodeType = ucCodeType | ucWriteFloatDataThree(32, iq10Tmp);
                if (ucCodeType == RECOK)
                {
                    iq10CurrConSysa.lData = iq10AdjTmp.lData;
                    iq10CurrConSysb.lData = iq10Tmp.lData;
                }
                else {
                    ucIDError = 1;    //
                }
            }
            else
            {
                ucCodeType = 0xf2;
                //  ucIDError = 1;                  //
            }
            break;


        case 0x68:              //
            uiMdlCalibra = 0;
            //
            //
            //
            if((iq10Tmp.lData >= 800 ) && (iq10Tmp.lData <= 1250))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x69:              //
            //
            if((iq10Tmp.lData >=  (long) -80 << 10 ) && (iq10Tmp.lData <= (long)80 << 10) && uiMdlCalibra)
            {
                //
                //if (ucCodeType == RECOK)
                //{
                iq10AcaSampSysa.lData = iq10AdjTmp.lData;
                //}
                //
                //if (ucCodeType == RECOK)
                //{
                iq10AcaSampSysb.lData = iq10Tmp.lData;
                uiPfcSendId = 5;
                //}
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x6a:              //
            uiMdlCalibra = 0;
            //
            //
            //
            if((iq10Tmp.lData >= 800 ) && (iq10Tmp.lData <= 1250))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x6b:              //
            //
            if((iq10Tmp.lData >=  (long) -80 << 10 ) && (iq10Tmp.lData <= (long)80 << 10) && uiMdlCalibra)
            {
                //
                //if (ucCodeType == RECOK)
                //{
                iq10AcbSampSysa.lData = iq10AdjTmp.lData;
                //}
                //
                //if (ucCodeType == RECOK)
                //{
                iq10AcbSampSysb.lData = iq10Tmp.lData;
                uiPfcSendId = 6;
                //}
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        case 0x72:              //
            uiMdlCalibra = 0;
            //
            //
            //
            if((iq10Tmp.lData >= 800 ) && (iq10Tmp.lData <= 1250))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x73:              //
            //
            if((iq10Tmp.lData >=  (long) -80 << 10 ) && (iq10Tmp.lData <= (long)80 << 10) && uiMdlCalibra)
            {
                //
                //if (ucCodeType == RECOK)
                //{
                iq10AccSampSysa.lData = iq10AdjTmp.lData;
                //}

                //
                //if (ucCodeType == RECOK)
                //{
                iq10AccSampSysb.lData = iq10Tmp.lData;
                uiPfcSendId = 7;
                //}
            }
            else
            {
                ucCodeType = 0xf2;
            }
            break;

        /*case 0x6c:                //
            if((iq10Tmp.lData >= 800 )&&(iq10Tmp.lData <= 1250))
            {
                uiVpfcAllSampSys = (unsigned int)(iq10Tmp.lData);
                uiPfcSendId = 8;
            }
            break;

        case 0x6d:              //
            if((iq10Tmp.lData >= 800 )&&(iq10Tmp.lData <= 1250))
            {
                uiVpfcAllCtrlSys = (unsigned int)(iq10Tmp.lData);
                uiPfcSendId = 9;
            }
            break;

        case 0x6e:              //
            ubitfData.fd = fTempSampSys+ubitfData.fd;
            if ( (ubitfData.fd >= (-20.0)) && (ubitfData.fd <= 20.0) )
            {
                ucCodeType = ucWriteFloatDataThree (36,ubitfData.fd);
                if (ucCodeType == RECOK)
                {
                    fTempSampSys = ubitfData.fd;
                }
            }
            break;*/

        case 0x70:              //
            uiMdlCalibra = 0;
            iq10SetSysTmp.lData = iq10Tmp.lData;
            iq10Tmp.lData = _IQ10mpy(iq10PowerConSysa.lData, iq10Tmp.lData);
            if((iq10Tmp.lData >= 512 ) && (iq10Tmp.lData <= 1536))
            {
                iq10AdjTmp.lData = iq10Tmp.lData;
                uiMdlCalibra = 1;
            }
            break;

        case 0x71:              //

            iq10Tmp.lData = _IQ10mpy(iq10PowerConSysb.lData, iq10SetSysTmp.lData) + iq10Tmp.lData;
            if((iq10Tmp.lData >=  (long) -3000 << 10 ) && (iq10Tmp.lData <= (long)3000 << 10) && uiMdlCalibra)
            {
                ucCodeType = ucWriteFloatDataThree(64, iq10AdjTmp);
                ucCodeType = ucCodeType | ucWriteFloatDataThree(68, iq10Tmp);
                if (ucCodeType == RECOK)
                {
                    iq10PowerConSysa.lData = iq10AdjTmp.lData;
                    iq10PowerConSysb.lData = iq10Tmp.lData;
                }
                else {
                    ucIDError = 1;    //
                }
            }
            else
            {
                ucCodeType = 0xf2;
                //  ucIDError = 1;                  //
            }
            break;

        /*case 0x74:                //
            //if ( (ubitfData.fd >= 0.8) && (ubitfData.fd <= 1.2) )
            if((iq10Tmp.lData >= 800 )&&(iq10Tmp.lData <= 1250))
            {
                uiVpfcUpSampSys = (unsigned int)(iq10Tmp.lData);
                uiPfcSendId = 8;
            }
            break;

        case 0x75:              //
            //if ( (ubitfData.fd >= 0.8) && (ubitfData.fd <= 1.2) )
            if((iq10Tmp.lData >= 800 )&&(iq10Tmp.lData <= 1250))
            {
                uiVpfcUpCtrlSys = (unsigned int)(iq10Tmp.lData);
                uiPfcSendId = 9;
            }
            break;*/


        case 0xa0:              //
            //
            ucCodeType = ucWriteFloatDataThree(120, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiNodeId0H = ubitfData.intdata[1].id;
                uiNodeId0L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa1:              //
            //
            ucCodeType = ucWriteFloatDataThree(124, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiNodeId1H = ubitfData.intdata[1].id;
                uiNodeId1L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa2:              //
            //
            ucCodeType = ucWriteFloatDataThree(128, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiVersionNoHw = ubitfData.intdata[1].id;
                //
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa4:              //
            //
            if((iq10Tmp.lData > iq10RunTime.lData + 1000) && (iq10Tmp.lData < iq10RunTime.lData + 1100))
            {
                uiEepromFlag = uiEepromFlag | 0x0001;
                iq10RunTime.lData = iq10Tmp.lData;
            }
            break;

        case 0xa5:              //
            if (( iq10Tmp.lData >= 0) && (iq10Tmp.lData <= (long)10 << 10))
            {
                //
                ucCodeType = ucWriteFloatDataThree(152, iq10Tmp);
                if (ucCodeType == RECOK)
                {
                    //
                    iq10MaintainTimes.lData = iq10Tmp.lData;
                }
            }
            break;

        case 0xa6:              //
            //
            ucCodeType = ucWriteFloatDataThree(136, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiBarCode0H = ubitfData.intdata[1].id;
                uiBarCode0L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa7:              //
            //
            ucCodeType = ucWriteFloatDataThree(140, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiBarCode1H = ubitfData.intdata[1].id;
                uiBarCode1L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa8:              //
            //
            ucCodeType = ucWriteFloatDataThree(144, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiBarCode2H = ubitfData.intdata[1].id;
                uiBarCode2L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xa9:              //
            //
            ucCodeType = ucWriteFloatDataThree(148, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiBarCode3H = ubitfData.intdata[1].id;
                uiBarCode3L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xaa:
            //
            ucCodeType = ucWriteFloatDataThree(132, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiCharactData0H = ubitfData.intdata[1].id;
                uiCharactData0L = ubitfData.intdata[0].id;
            }
            else {
                ucIDError = 1;    //
            }
            break;

        case 0xab:              //
            //
            ucCodeType = ucWriteFloatDataThree(156, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiMaintainData0H = ubitfData.intdata[1].id;
                uiMaintainData0L = ubitfData.intdata[0].id;
            }
            break;

        case 0xac:              //
            //
            ucCodeType = ucWriteFloatDataThree(160, ubitfData);
            if (ucCodeType == RECOK)
            {
                uiMaintainData1H = ubitfData.intdata[1].id;
                uiMaintainData1L = ubitfData.intdata[0].id;
            }
            break;

        case 0xe0:              //
            uiMemScopeAdd0 = (unsigned int *)ubitfData.intdata[0].id;
            ubitfData.intdata[0].id= (unsigned int)*uiMemScopeAdd0;
            uiMemScopeAdd1 = (unsigned int *)ubitfData.intdata[1].id;
            ubitfData.intdata[1].id=(unsigned int)*uiMemScopeAdd1;
            break;

        case 0xe1:              //
            setadd = (unsigned int *)ubitfData.intdata[0].id;
            *setadd = ubitfData.intdata[1].id;
            break;

        case 0xe2:              // write EEPROM of DCDC
            uiSetEepromAdd = (unsigned int )ubitfData.fd;
            break;

        case 0xe3:              //
            //
            if((uiSetEepromAdd == 4) || (uiSetEepromAdd == 8) || (uiSetEepromAdd == 12) || (uiSetEepromAdd == 16)) {
                ucWriteFloatDataThree (uiSetEepromAdd, iq12Tmp);
            }
            else {
                ucWriteFloatDataThree (uiSetEepromAdd, iq10Tmp);
            }
            break;

        case 0xe4:              //
            uiMemRdAddr0 = ubitfData.intdata[0].id;
            uiMemRdAddr1 = ubitfData.intdata[1].id;
            uiPfcSendId = 11;
            break;

        case 0xe5:              //
            uiMemWrAddr = ubitfData.intdata[0].id;
            uiMemWrValue = ubitfData.intdata[1].id;
            uiPfcSendId = 10;
            break;

        case 0xe6:              //
            uiEpromWrAddr = (unsigned int )ubitfData.fd;
            break;

        case 0xe7:              //
            if((uiEpromWrAddr == 4) || (uiEpromWrAddr == 12) || (uiEpromWrAddr == 20)
                    || (uiEpromWrAddr == 28) || (uiEpromWrAddr == 36) || (uiEpromWrAddr == 44))
            {
                uiEpromWrValue = (unsigned int )(iq10Tmp.lData);
                uiPfcSendId = 20;
            }
            else if((uiEpromWrAddr == 8) || (uiEpromWrAddr == 16) || (uiEpromWrAddr == 24)
                    || (uiEpromWrAddr == 32) || (uiEpromWrAddr == 40) || (uiEpromWrAddr == 48))
            {
                uiEpromWrValue = (unsigned int )(iq10Tmp.lData >> 3);
                uiPfcSendId = 20;
            }
            break;

        case 0xe8:              //
            uiMemScopeAdd0 = (unsigned int *)ubitfData.intdata[0].id;
            ubitfData.intdata[0].id = (unsigned int) * (uiMemScopeAdd0 + uiRdMemNum);
            uiMemScopeAdd1 = (unsigned int *)ubitfData.intdata[1].id;
            ubitfData.intdata[1].id = (unsigned int) * (uiMemScopeAdd1 + uiRdMemNum);
            uiRdMemNum = uiRdMemNum + 1;
            if (uiRdMemNum >= uiRdNumSet) {
                uiRdMemNum = 0;
            }
            break;

        case 0xe9:              //
            uiMemRdAddr0 = ubitfData.intdata[0].id + uiRdMemNum;
            uiMemRdAddr1 = ubitfData.intdata[1].id + uiRdMemNum;
            uiPfcSendId = 11;
            uiRdMemNum = uiRdMemNum + 1;
            if (uiRdMemNum >= uiRdNumSet) {
                uiRdMemNum = 0;
            }
            break;

        case 0xfe:          /* */
            if (ubitfData.intdata[1].id == 1)
            {
                DINT;
                IER = 0x0000;
                IFR = 0x0000;

                uiMdlStatus = uiMdlStatus | 0x8;
                //  EPwm1Regs.AQCSFRC.all = 0x5;        //PWM1A,PWM1B force low
                //  EPwm2Regs.AQCSFRC.all = 0x5;        //PWM2A,PWM2B force low
                //    EPwm3Regs.AQCSFRC.all = 0x5;      //PWM1A,PWM1B force low
                //  EPwm4Regs.AQCSFRC.all = 0x5;        //PWM2A,PWM2B force low    //
                /*  ECanaMboxes.MBOX4.MDL.word.HI_WORD = (((unsigned int)ucMsgType)<<8)|ucErrType;
                    if (ucErrType != 0xf0)
                    {
                        ECanaMboxes.MBOX4.MDL.word.HI_WORD |= 0x8000;
                    }
                    ECanaMboxes.MBOX4.MDL.word.LOW_WORD = uiCanCidTmp;
                    ECanaMboxes.MBOX4.MDH.word.LOW_WORD = ubitfData.intdata[0].id;
                    ECanaMboxes.MBOX4.MDH.word.HI_WORD = ubitfData.intdata[1].id;
                    ECana32Temp.CANTRS.all = 0;
                    ECana32Temp.CANTRS.bit.TRS4 = 1;        //
                    ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;*/

                //
                uSecond(1500);
                B1AppEntryPointer();
            }
            break;

        default:
            ucCodeType = NOVCOM;
            break ;
    }

    ucErrType = ucCodeType;

    if(ucAddIdentify)
    {
        ucErrType = 0xf4;
        uiSequeOnTimer = 0;
    }

    if((ucBroadCastCom == 0) && (uiCanDataId != 0xe4) && (uiCanDataId != 0xe9))
    {
        if(uiCanCidTmp != cInqNewRec)
        {
            ECanaMboxes.MBOX4.MDL.word.HI_WORD = (((unsigned int)ucMsgType) << 8) | ucErrType;
            if (ucErrType != 0xf0)
            {
                ECanaMboxes.MBOX4.MDL.word.HI_WORD |= 0x8000;
            }
            ECanaMboxes.MBOX4.MDL.word.LOW_WORD = uiCanCidTmp;
            ECanaMboxes.MBOX4.MDH.word.LOW_WORD = ubitfData.intdata[0].id;
            ECanaMboxes.MBOX4.MDH.word.HI_WORD = ubitfData.intdata[1].id;
            ECana32Temp.CANTRS.all = 0;
            ECana32Temp.CANTRS.bit.TRS4 = 1;
            ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;
        }
        //
        else if(uwTurnOnFirstRpl == 0)
        {
            uwTurnOnFirstRpl = 1;

            ECanaMboxes.MBOX4.MDL.word.HI_WORD = (((unsigned int)ucMsgType) << 8) | ucErrType;
            if (ucErrType != 0xf0)
            {
                ECanaMboxes.MBOX4.MDL.word.HI_WORD |= 0x8000;
            }
            ECanaMboxes.MBOX4.MDL.word.LOW_WORD = uiCanCidTmp;
            ECanaMboxes.MBOX4.MDH.word.LOW_WORD = ubitfData.intdata[0].id;
            ECanaMboxes.MBOX4.MDH.word.HI_WORD = ubitfData.intdata[1].id;
            ECana32Temp.CANTRS.all = 0;
            ECana32Temp.CANTRS.bit.TRS4 = 1;
            ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;
        }
    }
}


/************************************************/
/**/
/************************************************/
void vRrpProtocol(void)
{
    unsigned int i, k;
    ubitfloat iq10max, iq10min, iq10sum, tagCurrTmp;

    ucAddIdentify = 0;
    if(ubAddrCalcTimer)
    {
        ubAddrCalcTimer = 0;

        if(++ucAddIdentifyNum >= 3)
        {
            ucAddIdentifyNum = 3;
            ucAddIdentify = 0;
        }
    }
    if (ucCan5SynFlag)
    {
        ucCan5SynFlag = 0;
        uiMasterMdl = 0;
        uiMdlNumber = 0;
        for (i = 0; i < MAX_NUM; i++)
        {
            if (ucMdlOnoff[i] == 0)
            {
                uiMdlNumber++;
            }
        }
        if (uiMdlNumber > 1) {
            uiSelfChkEn = 1;
        }
        else {
            uiSelfChkEn = 0;
        }

        if (uiMdlAddr <= ucAddressSmall)
        {
            uiMasterMdl = 1;
            iq10max.lData = 0;
            iq10min.lData = (long)250 << 10;
            iq10sum.lData = 0;

            k = 0;
            for (i = 0; i < MAX_NUM; i++)
            {
                if (ucMdlOnoff[i] == 0)
                {
                    if (iq10MdlCurrent[i].lData > iq10max.lData)
                    {
                        iq10max.lData = iq10MdlCurrent[i].lData;
                    }
                    if (iq10MdlCurrent[i].lData < iq10min.lData)
                    {
                        iq10min.lData = iq10MdlCurrent[i].lData;
                    }
                    iq10sum.lData = iq10sum.lData + iq10MdlCurrent[i].lData;
                    if (iMdlCurrDelta[i] > 1)
                    {
                        k = k | 0x01;
                    }
                    else if (iMdlCurrDelta[i] < -1)
                    {
                        k = k | 0x02;
                    }
                    else
                    {
                        k = k | 0x04;
                    }
                }
            }

            if(uiMdlNumber)
            {
                if(!Flag.bit.share_on)              //2018-11-06 增加禁止均流控制(兼容LNINT协议)
                {
                    iq10MdlAvgCurr.lData = iq10MdlCurr.lData;
                }
                else
                {
                    iq10MdlAvgCurr.lData = iq10sum.lData / uiMdlNumber;
                }
                //+++iq10MdlAvgCurr.lData = iq10sum.lData / uiMdlNumber;
            }
            else
            {
                iq10MdlAvgCurr.lData = 0;
            }

            tagCurrTmp.fd = _IQ10toF(iq10MdlAvgCurr.lData);

            uiCurrCtrlFlag = 0;

            if (k == 0x01) {
                uiCurrCtrlFlag = 0x0300;
            }
            else if (k == 0x02) {
                uiCurrCtrlFlag = 0x0200;
            }
            else
            {
                if ((iq10max.lData - iq10min.lData) > 1024) {
                    uiCurrCtrlFlag = 0x0100;
                }
                else {
                    uiCurrCtrlFlag = 0x0000;
                }
            }
            ECanaMboxes.MBOX5.MDL.word.HI_WORD = 0x02F0;
            ECanaMboxes.MBOX5.MDL.word.LOW_WORD = uiCurrCtrlFlag;
            ECanaMboxes.MBOX5.MDH.word.LOW_WORD = tagCurrTmp.intdata[0].id;
            ECanaMboxes.MBOX5.MDH.word.HI_WORD = tagCurrTmp.intdata[1].id;
            ECana32Temp.CANTRS.all = 0;
            ECana32Temp.CANTRS.bit.TRS5 = 1;
            ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;
        }

        ucAddressSmall = MAX_NUM;
        for(i=0; i<MAX_NUM; i++)
        {
            iq10MdlCurrent[i].lData = 0;
            iMdlCurrDelta[i] = 0;
            ucMdlOnoff[i] = 0x03;
        }
    }
    else if(ucCan5DatFlag)
    {
        ucCan5DatFlag = 0;

        if(ucAddIdentify == 0)
        {
            i = 0;
            if(uwOnOffDCDC == 0x01)
            {
                i = i | 0x01;
            }

            if (uiMdlAddr >= MAX_NUM)
            {
                return;
            }
            ucMdlOnoff[uiMdlAddr] = i;

            iq10MdlCurrent[uiMdlAddr].lData = iq10MdlCurr.lData;
            tagCurrTmp.fd = _IQ10toF(iq10MdlCurr.lData);
            iMdlCurrDelta[uiMdlAddr] = iCurrDelta;
            ECanaMboxes.MBOX5.MDL.word.HI_WORD = 0x01F0;
            ECanaMboxes.MBOX5.MDL.word.LOW_WORD = (iCurrDelta & 0x0fff) | (i << 12);
            //
            ECanaMboxes.MBOX5.MDH.word.LOW_WORD = tagCurrTmp.intdata[0].id;
            ECanaMboxes.MBOX5.MDH.word.HI_WORD = tagCurrTmp.intdata[1].id;
            ECana32Temp.CANTRS.all = 0;
            ECana32Temp.CANTRS.bit.TRS5 = 1;
            ECanaRegs.CANTRS.all = ECana32Temp.CANTRS.all;
        }
    }
}


/****************************************************************/
/*                                              */
/****************************************************************/
void vDataInit(void)
{
    Uint16  i;
    Uint16 *addr;

    ServiceDogPointer();


    for(i = 0x9000; i < 0xA000; i++)
    {
        addr = (unsigned int *)i;
        *addr = 0;
    }

    ServiceDogPointer();
    il303ref = 12002;
    il310ref = 12279;
    il350ref = 13863;
    il360ref = 14259;
    il420ref = 16636;
    il430ref = 17032;
    il520ref = 20597;
    il550ref = 21785;
    il607ref = 24043;
    il732ref = 28994; //2017 20k
    RAMP_MAX = dPERIOD_MAX;

    Dcdcisr. PWM_load_flag = 0x00;
    Dcdcisr.ui_Dcdc_Short_Flag = 0;
    Dcdcisr.currlimit_flag = 0x00;
    Dcdcisr.ui_short_ontime = 1;
    Dcdcisr.ui_short_permit = 1;
    iq10PfcFanRatio_Temp = 3072;
    iq10PfcFanRatio_Curr = 205;
    Dcdcisr.ui_poweron_off = 1;
    uwOnOffDCDC = 0;  //
    iq10SetVolt.lData = dQ10_VOUT_FT_DEF;
    //
    iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
    iq10MdlLimit.lData = dQ10_CURR_LIMIT_MAX;
    iq10SetLimit.lData = dQ10_CURR_LIMIT_MAX;
    iq10SetLimitTrue.lData = dQ10_CURR_LIMIT_MAX;
    iq10SoftLimit.lData = dQ10_CURR_LIMIT_MAX;
    iq10SetPower.lData = dQ10_POWER_LIMIT_MAX;
    iq10SetPowerTrue.lData = dQ10_POWER_LIMIT_MAX;
    lPowerMax = POWER_MAX;
    lCurrRate = CURR_RATE;

    uiRdNumSet = 0x0400;
    ucPermitAdjust = 1;
    ucAddressSmall = MAX_NUM;
    //
    ucAddIdentify = 1;
    uiVpfcSet = 400;
    uiVpfcSet_true = 400;
    ucVoltAdjDelta = 128;
    uiTempPfc = 75;
    uiPfcOnOff = 0xffff;
    Dcdcisr.unbalance_cont = 0;

    Dcdcisr.test = 50;
    iq10AcaSampSysa.lData = 1024;
    iq10AcbSampSysa.lData = 1024;
    iq10AccSampSysa.lData = 1024;

    ulTime1 = 600;
    ulTime2 = 166200;
    //

    uiEfficfactor[0] = 12;
    uiEfficfactor[1] = 12;
    uiEfficfactor[2] = 275;
    uiEfficfactor[3] = 577;
    uiEfficfactor[4] = 855;
    uiEfficfactor[5] = 1203;
    uiEfficfactor[6] = 1540;
    uiEfficfactor[7] = 1826;
    uiEfficfactor[8] = 2143;
    uiEfficfactor[9] = 2463;
    uiEfficfactor[10] = 2750;
    uiEfficfactor[11] = 3070;
    uiEfficfactor[12] = 3315;
    uiEfficfactor[13] = 3549;

    for (i = 0; i < MAX_NUM; i++)
    {
        ucMdlOnoff[i] = 0x03;
    }

    Dcdcisr.ui_Dcdc_Volt_K1 = 4209;
    Dcdcisr.ui_Dcdc_Volt_K2 = 113;
    Dcdcisr.ui_Dcdc_Volt_K3 = 5374;
    Dcdcisr.ui_Dcdc_Volt_K4 = 8319;
    Dcdcisr.ui_Dcdc_Volt_K5 = 3144;//2017 20k

    Dcdcisr.ui_Dcdc_Curr_K1 = 11925;
    Dcdcisr.ui_Dcdc_Curr_K2 = 3733;
    Dcdcisr.ui_Dcdc_Curr_K3 = 1722;
    Dcdcisr.ui_Dcdc_Curr_K4 = 2978;
    Dcdcisr.ui_Dcdc_Curr_K5 = 1262;

    Dcdcisr.ui_Dcdc_Curr_Filt_K1 = 30850;
    Dcdcisr.ui_Dcdc_Curr_Filt_K2 = 959;

    Dcdcisr.ui_Dcdc_Temp_Ctrl = 50;
    Dcdcisr.ui_Dcdc_Temp_Set1 = 35;
    Dcdcisr.ui_Dcdc_Temp_Set2 = 40;
    Dcdcisr.ui_Dcdc_K_flag = 0;

    Dcdcisr.ui_Dcdc_debug = 0;

    Dcdcisr.ui_Dcdc_Volt_Ref = 11880;

    uiVersionNoSw = dSOFT_VER_DEC;

    Dcdcisr.ui_Dcdc_Duty_Ramp = 166 * 64;
    Dcdcisr.ui_Dcdc_Duty_Short = 160 * 64;
    Dcdcisr.ui_Dcdc_Duty_Permit = 166 * 64;
    Dcdcisr.ul_Dcdc_Power_Lim = 0x23BA82F5;
    Dcdcisr.ui_Dcdc_Power_Lim = 2808 * 8;

    Dcdcisr.ui_Dcdc_Curr_delta = 848;

#ifdef LV
    Dcdcisr.ui_Dcdc_Curr_delta = 0x1000;
#endif

    Dcdcisr.ui_dead_adj_min = dDEAD_ADJ_MIN_NF; //2019-02-15
}

void vLimitInit()
{
    ubitfloat    ftemp1;
    unsigned char ucCodeType;
    ubitfloat ubitfTmp, iq10temp1, iq10temp2;
    signed long lreftmp;

    GpioDataRegs.GPACLEAR.bit.GPIO26 = 1;
    ftemp1.lData = 0;
    if(uiI2caReadData(0) == I2C_SUCCESS)
    {
        ftemp1.lData = fRdTemp.lData;
    }

    if (ftemp1.lData == 0x55AA55AA)
    {
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(72);
        if ((iq10temp1.lData >= 10)
                && (iq10temp1.lData <= dQ10_POWER_LIMIT_MAX)
                && (ucReadEepromOk == 0) ) { //
            iq10MdlPowerFt.lData = iq10temp1.lData;
        }
        else {
            iq10MdlPowerFt.lData = dQ10_POWER_LIMIT_MAX;
        }
        iq10SetPower.lData = iq10MdlPowerFt.lData;

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(76);
        if ((iq10temp1.lData >= 10)
                && (iq10temp1.lData <= dQ10_CURR_LIMIT_MAX)
                && (ucReadEepromOk == 0) ) { //
            iq10MdlCurrFt.lData = iq10temp1.lData;
        }
        else {
            iq10MdlCurrFt.lData = dQ10_CURR_LIMIT_MAX;    //
        }
        iq10SetLimit.lData = iq10MdlCurrFt.lData;



        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(80);
        if ((iq10temp1.lData > 1000)
                && (iq10temp1.lData < (long)31 << 10)
                && (ucReadEepromOk == 0) ) {
            iq10AcCurrFt.lData = iq10temp1.lData;
        }
        else {
            iq10AcCurrFt.lData = (long)30 << 10;
        }

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(84);
        if ((iq10temp1.lData >= dQ10_VOUT_MIN)
                && (iq10temp1.lData <= (dQ10_VOUT_MAX + 1)) //2018-11-23 保存时会+1,读取时也需要判断
                && (ucReadEepromOk == 0) )
        {
            iq10MdlVoltFt.lData = iq10temp1.lData;
        }
        else {
            iq10MdlVoltFt.lData = dQ10_VOUT_FT_DEF;
        }

        iq10SetVolt.lData = iq10MdlVoltFt.lData;

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(88);
        if ((iq10temp1.lData >= dQ10_OVP_MIN)
                && (iq10temp1.lData <= (dQ10_OVP_MAX + 1))  //2018-11-23 保存时会+1,读取时也需要判断
                && (ucReadEepromOk == 0) ) {
            iq10MdlVoltUp.lData = iq10temp1.lData;
        }
        else {
            iq10MdlVoltUp.lData = dQ10_OVP_DEF;
        }

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(92);
        if ((iq10temp1.lData >= (long)40 << 10)
                && (iq10temp1.lData <= (long)150 << 10)
                && (ucReadEepromOk == 0) ) { //
            iq10MdlTempUp.lData = iq10temp1.lData;
        }
        else {
            iq10MdlTempUp.lData = iq10_TEMPUP;    //
        }

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(4);
        ucCodeType = ucReadEepromOk;
        iq10temp2 = fReadFloatDataThree(8);
        if (((iq10temp1.lData >= 3200) && (iq10temp1.lData <= 5000) && (iq10temp2.lData >= (long) -20 << 12)
                && (iq10temp2.lData <= (long)20 << 12)) && (ucCodeType == 0) && (ucReadEepromOk == 0))
        {
            iq12VoltSampSysa.lData = iq10temp1.lData;
            iq12VoltSampSysb.lData = iq10temp2.lData;
        }
        else
        {
            iq12VoltSampSysa.lData = 4130;
            iq12VoltSampSysb.lData = -820;
            uiMdlStatus = uiMdlStatus | 0x1000;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(12);
        ucCodeType = ucReadEepromOk;
        iq10temp2 = fReadFloatDataThree(16);
        if (((iq10temp1.lData >= 3200) && (iq10temp1.lData <= 5000) && (iq10temp2.lData >= (long) -20 << 12)
                && (iq10temp2.lData <= (long)20 << 12)) && (ucCodeType == 0) && (ucReadEepromOk == 0))
        {
            iq12VoltConSysa.lData = iq10temp1.lData;
            iq12VoltConSysb.lData = iq10temp2.lData;
            iq12VoltConSysa_inv.lData = (long)((Uint32)4096 * (Uint32)4096 / iq12VoltConSysa.lData);
            VASAMPLE_INV = (unsigned int)iq12VoltConSysa_inv.lData;
        }
        else
        {
            iq12VoltConSysa.lData = 4065;
            iq12VoltConSysa_inv.lData = 4127;
            VASAMPLE_INV = 4127;
            iq12VoltConSysb.lData = -2100;
            uiMdlStatus = uiMdlStatus | 0x1000;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(20);
        ucCodeType = ucReadEepromOk;
        iq10temp2 = fReadFloatDataThree(24);
        if (((iq10temp1.lData >= 512) && (iq10temp1.lData <= 1536) && (iq10temp2.lData >= (long) -20 << 10)
                && (iq10temp2.lData <= (long)20 << 10)) && (ucCodeType == 0) && (ucReadEepromOk == 0))
        {
            iq10CurrSampSysa.lData = iq10temp1.lData;
            iq10CurrSampSysb.lData = iq10temp2.lData;
        }
        else
        {
            iq10CurrSampSysa.lData = 1040;
            iq10CurrSampSysb.lData = (long) -1 << 10;
            uiMdlStatus = uiMdlStatus | 0x1000;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(28);
        ucCodeType = ucReadEepromOk;
        iq10temp2 = fReadFloatDataThree(32);
        if (((iq10temp1.lData >= 512) && (iq10temp1.lData <= 1536) && (iq10temp2.lData >= (long) -30 << 10)
                && (iq10temp2.lData <= (long)30 << 10)) && (ucCodeType == 0) && (ucReadEepromOk == 0))
            /*if (((iq10temp1.lData>=512)&&(iq10temp1.lData<=1536)&&(iq10temp2.lData>=(long)5<<10)
                &&(iq10temp2.lData<=(long)40<<10))&&(ucCodeType==0)&&(ucReadEepromOk==0))*/ //
        {
            iq10CurrConSysa.lData = iq10temp1.lData;
            iq10CurrConSysb.lData = iq10temp2.lData;
        }
        else
        {
            iq10CurrConSysa.lData = 1024;
            iq10CurrConSysb.lData = (long)3 << 9;
            uiMdlStatus = uiMdlStatus | 0x1000;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(64);
        ucCodeType = ucReadEepromOk;
        iq10temp2 = fReadFloatDataThree(68);
        if ((iq10temp1.lData >= 512) && (iq10temp1.lData <= 1536) && (iq10temp2.lData >= (long) -3000 << 10)
                && (iq10temp2.lData <= (long)3000 << 10) && (ucCodeType == 0) && (ucReadEepromOk == 0))
        {
            iq10PowerConSysa.lData = iq10temp1.lData;
            iq10PowerConSysb.lData = iq10temp2.lData;
        }
        else
        {
            iq10PowerConSysa.lData = 1024;
            iq10PowerConSysb.lData = (long)200 << 10;
            uiMdlStatus = uiMdlStatus | 0x1000;
        }
        //---------------------------------------------
        /*  ftemp1=fReadFloatDataThree(36);
            if ( (ftemp1 >= (-10.0)) && (ftemp1 <= 10.0) && (ucReadEepromOk==0) )
                fTempSampSys = ftemp1;
            else
            {
                fTempSampSys = 1;
                uiMdlStatus=uiMdlStatus|0x1000;
            }       */
        //---------------------------------------------
        /*
                iq10temp1=fReadFloatDataThree(40);
                ucCodeType=ucReadEepromOk;
                iq10temp2=fReadFloatDataThree(44);
                if ((iq10temp1.lData>=800)&&(iq10temp1.lData<=1250)&&(iq10temp2.lData>=(long)-40<<10)
                    &&(iq10temp2.lData<=(long)40<<10)&&(ucCodeType==0)&&(ucReadEepromOk==0))
                {
                    iq10AcaSampSysa.lData = iq10temp1.lData;
                    iq10AcaSampSysb.lData = iq10temp2.lData;
                }
                else
                {
                    iq10AcaSampSysa.lData = 1024;
                    iq10AcaSampSysb.lData = 0;
        //          uiMdlStatus=uiMdlStatus|0x1000;    //
                }
                //---------------------------------------------
                iq10temp1=fReadFloatDataThree(48);
                ucCodeType=ucReadEepromOk;
                iq10temp2=fReadFloatDataThree(52);
                if ((iq10temp1.lData>=800)&&(iq10temp1.lData<=1250)&&(iq10temp2.lData>=(long)-40<<10)
                    &&(iq10temp2.lData<=(long)40<<10)&&(ucCodeType==0)&&(ucReadEepromOk==0))
                {
                    iq10AcbSampSysa.lData = iq10temp1.lData;
                    iq10AcbSampSysb.lData = iq10temp2.lData;
                }
                else
                {
                    iq10AcbSampSysa.lData = 1024;
                    iq10AcbSampSysb.lData = 0;
        //          uiMdlStatus=uiMdlStatus|0x1000;    //
                }
                //---------------------------------------------
                iq10temp1=fReadFloatDataThree(56);
                ucCodeType=ucReadEepromOk;
                iq10temp2=fReadFloatDataThree(60);
                if ((iq10temp1.lData>=800)&&(iq10temp1.lData<=1250)&&(iq10temp2.lData>=(long)-40<<10)
                    &&(iq10temp2.lData<=(long)40<<10)&&(ucCodeType==0)&&(ucReadEepromOk==0))
                {
                    iq10AccSampSysa.lData = iq10temp1.lData;
                    iq10AccSampSysb.lData = iq10temp2.lData;
                }
                else
                {
                    iq10AccSampSysa.lData = 1024;
                    iq10AccSampSysb.lData = 0;
        //          uiMdlStatus=uiMdlStatus|0x1000;   //
                }
          */
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(100);
        if (((iq10temp1.intdata[0].id & 0x7fe0) == 0) && (ucReadEepromOk == 0)) {
            uiMdlCtrl = iq10temp1.intdata[0].id;
        }
        else {
            uiMdlCtrl = 0;
        }

        uiPfcCtrl = uiMdlCtrl & 0x001A;
        if (uiMdlCtrl & 0x8000) {
            uiMdlStatus = uiMdlStatus | 0x0020;
        }

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(104);
        if ((iq10temp1.lData >= (long)50 << 10) && (iq10temp1.lData <= (long)300 << 10) && (ucReadEepromOk == 0)) {
            iq10ReonTime.lData = iq10temp1.lData;
        }
        else {
            iq10ReonTime.lData = (long)300 << 10;
        }

        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(108);
        if ((iq10temp1.lData >= (long)800 << 10) && (iq10temp1.lData <= (long)9000 << 10) && (ucReadEepromOk == 0)) {
            iq10LoadSstartTime.lData = iq10temp1.lData;
        }
        else {
            iq10LoadSstartTime.lData = (long)1000 << 10;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(112);
        if ((iq10temp1.lData >= 0) && (iq10temp1.lData <= (long)10 << 10) && (ucReadEepromOk == 0)) {
            iq10OpenTime.lData = iq10temp1.lData;
        }
        else {
            iq10OpenTime.lData = 0;
        }
        if(iq10OpenTime.lData <= 1) {
            ucOpenOrder = 0;
        }
        else {
            ucOpenOrder = 1;
        }
        //---------------------------------------------
        iq10temp1 = fReadFloatDataThree(116);
        if (( iq10temp1.lData >= 0)
                && (iq10temp1.lData <= (long)200000 << 10) && (ucReadEepromOk == 0))
        {
            iq10RunTime.lData = iq10temp1.lData;
        }
        else {
            iq10RunTime.lData = 0;    //
        }
        //---------------------------------------------
        ubitfTmp = fReadFloatDataThree (120);
        uiNodeId0H = ubitfTmp.intdata[1].id;
        uiNodeId0L = ubitfTmp.intdata[0].id;
        if ((uiNodeId0H == 0) && (uiNodeId0L == 0)) {
            uiNodeId0L = 1;
        }

        ubitfTmp = fReadFloatDataThree (124);
        uiNodeId1H = ubitfTmp.intdata[1].id;
        uiNodeId1L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (128);
        uiVersionNoHw = ubitfTmp.intdata[1].id;
        //

        ubitfTmp = fReadFloatDataThree (132);
        uiCharactData0H = ubitfTmp.intdata[1].id;
        uiCharactData0L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (136);
        uiBarCode0H = ubitfTmp.intdata[1].id;
        uiBarCode0L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (140);
        uiBarCode1H = ubitfTmp.intdata[1].id;
        uiBarCode1L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (144);
        uiBarCode2H = ubitfTmp.intdata[1].id;
        uiBarCode2L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (148);
        uiBarCode3H = ubitfTmp.intdata[1].id;
        uiBarCode3L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (156);
        uiMaintainData0H = ubitfTmp.intdata[1].id;
        uiMaintainData0L = ubitfTmp.intdata[0].id;

        ubitfTmp = fReadFloatDataThree (160);
        uiMaintainData1H = ubitfTmp.intdata[1].id;
        uiMaintainData1L = ubitfTmp.intdata[0].id;

        iq10temp1 = fReadFloatDataThree(152);
        if (( iq10temp1.lData >= 0)
                && (iq10temp1.lData <= (long)10 << 10) && (ucReadEepromOk == 0))
        {
            iq10MaintainTimes.lData = iq10temp1.lData;
        }
        else {
            iq10MaintainTimes.lData = 0;
        }
        //---------------------------------------------

        //2018-11-01 增加鲁能协议相关
            //模块地址
        ubitfTmp = fReadFloatDataThree(dEEPROM_ADDR_ADDR);
        if((0 == ucReadEepromOk) &&\
            (dLNINT_ADDR_MIN <= ubitfTmp.lData) &&\
            (dLNINT_ADDR_MAX >= ubitfTmp.lData))
        {
            uiAddrConst = ubitfTmp.lData;
        }
        else
        {
            uiAddrConst = dLNINT_ADDR_MIN;
        }    
            //分组号
        ubitfTmp = fReadFloatDataThree(dEEPROM_GROUP_ADDR);
        if((0 == ucReadEepromOk) &&\
            (dLNINT_GROUP_MIN <= ubitfTmp.lData) &&\
            (dLNINT_GROUP_MAX >= ubitfTmp.lData))
        {
            uiGroupConst = ubitfTmp.lData;
        }
        else
        {
            uiAddrConst = dLNINT_GROUP_MIN;
        }
            //通讯速率
        ubitfTmp = fReadFloatDataThree(dEEPROM_BAUD_ADDR);    
        if((0 == ucReadEepromOk) &&\
            (dLNINT_BAUD_MIN <= ubitfTmp.lData) &&\
            (dLNINT_BAUD_MAX >= ubitfTmp.lData))
        {
            uiBaudConst = ubitfTmp.lData;
        }
        else
        {
            uiBaudConst = dLNINT_BAUD_DEF;
        }
            //均流开关
        ubitfTmp = fReadFloatDataThree(dEEPROM_SHARE_ADDR);
        if((0 == ucReadEepromOk) && (0 != ubitfTmp.lData))
        {
            Flag.bit.share_on = 1;                  //使能均流
        }
        else
        {
            Flag.bit.share_on = 0;                  //禁止均流
        }
        //2018-11-01
    }
    else
    {
        //
        ubitfTmp.intdata[0].id = 0x55AA;
        ubitfTmp.intdata[1].id = 0x55AA;
        ucCodeType = ucWriteFloatDataThree(0, ubitfTmp);
        if (ucCodeType == NOVCOM)
        {
            ucCodeType = ucWriteFloatDataThree(0, ubitfTmp);
        }

        ServiceDogPointer();
        uiMdlStatus = uiMdlStatus | dEPROM_FAIL;
        uiMdlCtrl = 0;
        uiPfcCtrl = 0;
        iq10MdlVoltFt.lData = dQ10_VOUT_FT_DEF;
        iq10SetVolt.lData = iq10MdlVoltFt.lData;
        iq10MdlVoltUp.lData = dQ10_OVP_DEF;
        iq10MdlTempUp.lData = iq10_TEMPUP;
        iq10MdlPowerFt.lData = dQ10_POWER_LIMIT_MAX;
        iq10MdlCurrFt.lData = dQ10_CURR_LIMIT_MAX;

        iq10AcCurrFt.lData = (long)30 << 10;

        iq12VoltSampSysa.lData = 4100;
        iq12VoltSampSysb.lData = -614;
        iq12VoltConSysa.lData = 3974;
        iq12VoltConSysa_inv.lData = 4222;
        VASAMPLE_INV = 4222;
        //
        iq12VoltConSysb.lData = 4096;
        iq10CurrSampSysa.lData = 1036;
        iq10CurrSampSysb.lData = (long) -4 << 10;
        iq10CurrConSysa.lData = 1024;        //
        iq10CurrConSysb.lData = (long)1 << 10; //
        //
        //
        iq10PowerConSysa.lData = 1024;   //
        iq10PowerConSysb.lData = (long)200 << 10; ///


        iq10LoadSstartTime.lData = (long)1000 << 10;
        iq10ReonTime.lData = (long)300 << 10;
        iq10RunTime.lData = 0;
        uiOpenTime = 0;

        uiNodeId0H = 0;
        uiNodeId0L = 1;

    //2018-11-02 鲁能协议相关
        uiAddrConst = dLNINT_ADDR_MIN;
        uiGroupConst = dLNINT_GROUP_MIN;
        uiBaudConst = dLNINT_BAUD_DEF;
        Flag.bit.share_on = 0;                  //鲁能默认关闭均流
    }
    #ifdef USE_LNINT    
        uiGroupTemp = uiGroupConst;             //更新临时组地址为固化组地址
        uiMdlAddr = uiAddrConst;
    #else
        Flag.bit.share_on = 1;                  //其它非鲁能机型，允许均流
    #endif
    //2018-11-02 end

    //
    ubitfCodeChkRd0 = fReadFloatDataThree(168);
    ubitfCodeChkRd1 = fReadFloatDataThree(172);
    ubitfCodeChkRd2 = fReadFloatDataThree(176);

    //
    if((ubitfCodeChkRd0.lData == 0xFFFFFFFF) && (ubitfCodeChkRd1.lData == 0xFFFFFFFF) && (ubitfCodeChkRd2.lData == 0xFFFFFFFF))
    {
        ubitfCodeChkRd0.lData = 0x4E41494A;
        ubitfCodeChkRd1.lData = 0x474E4157;
        ubitfCodeChkRd2.lData = 0x494A454B;

        ucWriteFloatDataThree(168, ubitfCodeChkRd0);
        ucWriteFloatDataThree(172, ubitfCodeChkRd1);
        ucWriteFloatDataThree(176, ubitfCodeChkRd2);
    }
    //

    //
    iq10VoltAdd.lData = _IQ10div((3072000 - iq12VoltSampSysb.lData), iq12VoltSampSysa.lData) >> 2;
    //
    lreftmp = _IQ10div((((long)48 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    Dcdcisr.ui_Dcdc_Curr_35A = (unsigned int)(lreftmp >> 10);

    lreftmp = _IQ10div((((long)51 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    Dcdcisr.ui_Dcdc_Curr_42A = (unsigned int)(lreftmp >> 10);

    if (Dcdcisr.ui_Dcdc_Curr_42A > 0x8f9f) {
        Dcdcisr.ui_Dcdc_Curr_42A = 0x8f9f;
    }
    lreftmp = _IQ10div((((long)1 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    Dcdcisr.ui_Dcdc_Curr_1A = (unsigned int)(lreftmp >> 10);

    lreftmp = _IQ10div((((long)2 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    Dcdcisr.ui_Dcdc_Curr_2A = (unsigned int)(lreftmp >> 10);
    //

    lreftmp = _IQ10div((((long)10 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    lCurr10A = (signed int)(lreftmp >> 10);

    lreftmp = _IQ10div((((long)8 << 10) - iq10CurrSampSysb.lData), iq10CurrSampSysa.lData) * 655;
    lCurr8A = (signed int)(lreftmp >> 10);

    GpioDataRegs.GPASET.bit.GPIO26 = 1;
}

void sCodeChk(void)
{
    ubitfloat ubitfTmp;
    ubitfloat ubitfCodeChk;

    unsigned char ubCode0; //
    unsigned char ubCode1; //
    unsigned char ubCode2; //
    unsigned char ubCode3; //
    unsigned char ubCode4; //
    unsigned char ubCode5; //
    unsigned char ubCode6; //
    unsigned char ubCode7; //
    unsigned char ubCode8; //
    unsigned char ubCode9; //
    unsigned char ubCode10;//
    unsigned char ubCode11;//

    ubitfCodeChk = fReadFloatDataThree (168);
    ubCode0 = ubitfCodeChk.intdata[0].bitdata.highchar; //
    ubCode1 = ubitfCodeChk.intdata[0].bitdata.lowchar;  //
    ubCode2 = ubitfCodeChk.intdata[1].bitdata.highchar; //
    ubCode3 = ubitfCodeChk.intdata[1].bitdata.lowchar;  //

    ubitfCodeChk = fReadFloatDataThree (172);
    ubCode4 = ubitfCodeChk.intdata[0].bitdata.highchar; //
    ubCode5 = ubitfCodeChk.intdata[0].bitdata.lowchar;  //
    ubCode6 = ubitfCodeChk.intdata[1].bitdata.highchar; //
    ubCode7 = ubitfCodeChk.intdata[1].bitdata.lowchar;  //

    ubitfCodeChk = fReadFloatDataThree (176);
    ubCode8 = ubitfCodeChk.intdata[0].bitdata.highchar; //
    ubCode9 = ubitfCodeChk.intdata[0].bitdata.lowchar;  //
    ubCode10 = ubitfCodeChk.intdata[1].bitdata.highchar;//
    ubCode11 = ubitfCodeChk.intdata[1].bitdata.lowchar; //

    if((ubCode0 != 0x4A) || (ubCode1 != 0x49) || (ubCode2 != 0x41) || (ubCode3 != 0x4E))
    {
        ubitfTmp.intdata[0].bitdata.highchar = ubCode0;
        ubitfTmp.intdata[0].bitdata.lowchar = ubCode1;
        ubitfTmp.intdata[1].bitdata.highchar = ubCode2;
        ubitfTmp.intdata[1].bitdata.lowchar = ubCode3;
        ucWriteFloatDataThree(168, ubitfTmp);

        /*while(1)
        {
            GpioDataRegs.GPASET.bit.GPIO8 = 1;
            GpioDataRegs.GPASET.bit.GPIO19 = 1;
            GpioDataRegs.GPASET.bit.GPIO9 = 1;
        }*/
    }
    else if((ubCode4 != 0x57) || (ubCode5 != 0x41) || (ubCode6 != 0x4E) || (ubCode7 != 0x47))
    {
        ubitfTmp.intdata[0].bitdata.highchar = ubCode4;
        ubitfTmp.intdata[0].bitdata.lowchar = ubCode5;
        ubitfTmp.intdata[1].bitdata.highchar = ubCode6;
        ubitfTmp.intdata[1].bitdata.lowchar = ubCode7;
        ucWriteFloatDataThree(172, ubitfTmp);

        /*while(1)
        {
            GpioDataRegs.GPASET.bit.GPIO8 = 1;
            GpioDataRegs.GPASET.bit.GPIO19 = 1;
            GpioDataRegs.GPASET.bit.GPIO9 = 1;
        }*/
    }
    else if((ubCode8 != 0x4B) || (ubCode9 != 0x45) || (ubCode10 != 0x4A) || (ubCode11 != 0x49))
    {
        ubitfTmp.intdata[0].bitdata.highchar = ubCode8;
        ubitfTmp.intdata[0].bitdata.lowchar = ubCode9;
        ubitfTmp.intdata[1].bitdata.highchar = ubCode10;
        ubitfTmp.intdata[1].bitdata.lowchar = ubCode11;
        ucWriteFloatDataThree(176, ubitfTmp);

        /*while(1)
        {
            GpioDataRegs.GPASET.bit.GPIO19 = 1;
            GpioDataRegs.GPASET.bit.GPIO8 = 1;
            GpioDataRegs.GPASET.bit.GPIO9 = 1;
        }*/

    }

}
//
//===========================================================================
//
//==========================================================================
